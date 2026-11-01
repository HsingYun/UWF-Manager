// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "composition/console/ConsoleApplication.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <exception>
#include <memory>

#include "application/configuration/commands/UwfmgrCommandParser.h"
#include "application/console/ConsoleCommandExecutor.h"
#include "application/uwf/CompositeAllVolumeProtection.h"
#include "composition/uwf/SystemUwf.h"
#include "infrastructure/windows/console/ConsoleSession.h"
#include "infrastructure/windows/console/WindowsConsoleEnvironment.h"
#include "infrastructure/windows/storage/AllVolumeProtectionScope.h"
#include "infrastructure/windows/text/UnicodeConversion.h"
#include "presentation/appearance/ThemeManager.h"
#include "presentation/console/ConsoleDiagnostics.h"
#include "presentation/console/ConsoleRenderer.h"
#include "presentation/views/commands/CommandOutputDialog.h"

namespace uwf::composition {
namespace {
std::unique_ptr<QCoreApplication> createCommandApplication(int& count, char* arguments[], const bool graphical) {
  std::unique_ptr<QCoreApplication> application;
  if (graphical) {
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    auto desktop = std::make_unique<QApplication>(count, arguments);
    desktop->setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));
    ui::ThemeManager::instance().applyStartupTheme(std::nullopt);
    application = std::move(desktop);
  } else {
    application = std::make_unique<QCoreApplication>(count, arguments);
  }
  application->setApplicationName(QStringLiteral("UWF Manager"));
  application->setOrganizationName(QStringLiteral("UWF"));
  return application;
}

// Select the delivery mechanism before attachment can replace standard handles.
// Redirection preserves the original machine-readable stream/exit-code contract.
class CommandOutput final {
 public:
  CommandOutput() : m_graphical(!windows::hasRedirectedConsoleOutput()) {}
  [[nodiscard]] bool graphical() const { return m_graphical; }

  int help(const std::string& text) const {
    if (m_graphical) return show(QStringLiteral("UWF Manager - Help"), text, 0);
    m_console.writeOutput(text);
    return 0;
  }
  int publish(const app::ConsoleCommandResult& result) const {
    console::logConsoleResult(result);
    if (!m_graphical)
      return console::publishConsoleResult(
          result, [&](const std::string_view text) { m_console.writeOutput(text); }, [&](const std::string_view text) { m_console.writeError(text); });
    const auto text = console::renderConsoleResult(result);
    auto message = text.output;
    if (!text.error.empty()) {
      if (!message.empty()) message += '\n';
      message += text.error;
    }
    return show(QStringLiteral("UWF Manager - Command result"), message, result.exitCode);
  }
  int failure(const compat::Failure& failure, const std::string& publicReason = {}) const {
    app::ConsoleCommandResult result;
    result.failure = failure;
    result.effect = failure.effect;
    result.exitCode = app::consoleFailureExitCode(failure.kind);
    result.publicReason = publicReason;
    return publish(result);
  }

 private:
  static int show(const QString& title, const std::string& text, const int exitCode) {
    ui::CommandOutputDialog dialog(title, QString::fromStdString(text));
    static_cast<void>(dialog.exec());
    return exitCode;
  }

  const bool m_graphical;
  const windows::ConsoleSession m_console;
};
}  // namespace

int runConsoleApplication(int argc, char* argv[], const QStringList& arguments) {
  static_cast<void>(argc);
  const CommandOutput output;
  // Qt receives only the executable name, never the UWF command operands.
  int applicationCount = 1;
  char* applicationArguments[]{argv[0], nullptr};
  const auto application = createCommandApplication(applicationCount, applicationArguments, output.graphical());
  compat::Effect executedEffect = compat::Effect::None;
  try {
    std::vector<std::string> operands;
    for (qsizetype index = 1; index < arguments.size(); ++index) operands.push_back(windows::wideToUtf8(arguments[index].toStdWString()));
    const auto command = app::parseUwfmgrArguments(operands);
    if (command.parseError != app::ParseError::None)
      return output.failure({compat::ErrorKind::InvalidArgument, compat::Effect::None, app::uwfmgrParseErrorText(command), {}},
                            app::uwfmgrParseErrorText(command));
    if (command.kind == app::UwfmgrKind::Help) {
      return output.help(app::uwfmgrHelpText(command.args.empty() ? std::string_view{} : command.args.front()));
    }
    // Help and parse failures do not initialize a provider, request elevation
    // or acquire the main UI's single-instance channel.
    const windows::ConsoleCancellation cancellation;
    const compat::Context context{cancellation.token()};
    const auto system = systemUwf();
    const windows::WindowsConsoleEnvironment environment;
    const auto observations = windows::windowsConfigurationEnvironment();
    const auto allVolumes =
        app::composeAllVolumeProtection(system, {.isAdministrator = [check = observations.isAdministrator](const compat::Context&) { return check(); },
                                                 .protectScope = &windows::readAllVolumeProtectionScope});
    const auto result = app::executeConsoleCommand(command, *system, environment, context, allVolumes.get());
    executedEffect = result.effect;
    return output.publish(result);
  } catch (const compat::Error& error) {
    return output.failure(error.failure());
  } catch (const std::exception& error) {
    return output.failure({compat::ErrorKind::Failed, executedEffect, error.what(), {}});
  }
}
int reportConsoleStartupFailure(const QString& message) noexcept {
  try {
    const CommandOutput output;
    int applicationCount = 1;
    char name[] = "UWF.exe";
    char* applicationArguments[]{name, nullptr};
    const auto application = createCommandApplication(applicationCount, applicationArguments, output.graphical());
    return output.failure({compat::ErrorKind::Failed, compat::Effect::None, message.toUtf8().toStdString(), {}});
  } catch (...) {
    return 1;
  }
}
}  // namespace uwf::composition
