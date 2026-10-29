// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QTest>
#include <future>

#include "windows/filesystem/CreateOnlyTextFileWriter.h"

namespace {
using Outcome = uwf::app::TextFileCreationResult::Outcome;

QStringList entries(const QString& directory) { return QDir(directory).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System); }

class WindowsTextFileCreationTests final : public QObject {
  Q_OBJECT

 private slots:
  void createsCompleteUtf8TextWithoutTemporaryArtifacts() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto parent = directory.filePath(QStringLiteral("配置 目录"));
    QVERIFY(QDir().mkdir(parent));
    const auto path = QDir(parent).filePath(QStringLiteral("UWF.ini"));
    const QString text = QStringLiteral("[UI]\nTheme=dark\nLanguage=zh_CN\nHubVisible=false\n; Unicode 内容\n");
    const auto result = uwf::windows::createUtf8TextFile(path, text);
    QVERIFY2(result.outcome == Outcome::Created, qUtf8Printable(result.error));
    QVERIFY(result.error.isEmpty());
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), text.toUtf8());
    QCOMPARE(entries(parent), QStringList{QStringLiteral("UWF.ini")});
  }

  void existingEntriesAreNeverOverwritten_data() {
    QTest::addColumn<bool>("directoryTarget");
    QTest::newRow("file") << false;
    QTest::newRow("directory") << true;
  }

  void existingEntriesAreNeverOverwritten() {
    QFETCH(bool, directoryTarget);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = directory.filePath(QStringLiteral("UWF.ini"));
    if (directoryTarget) QVERIFY(QDir().mkdir(path));
    const auto preservedPath = directoryTarget ? QDir(path).filePath(QStringLiteral("preserve.txt")) : path;
    const QByteArray original("user-maintained contents\n");
    QFile preserved(preservedPath);
    QVERIFY(preserved.open(QIODevice::WriteOnly));
    QCOMPARE(preserved.write(original), original.size());
    preserved.close();
    const auto modified = QFileInfo(preservedPath).lastModified();

    const auto result = uwf::windows::createUtf8TextFile(path, QStringLiteral("replacement"));
    QCOMPARE(result.outcome, Outcome::AlreadyExists);
    QCOMPARE(QFileInfo(path).isDir(), directoryTarget);
    QCOMPARE(QFileInfo(preservedPath).lastModified(), modified);
    QVERIFY(preserved.open(QIODevice::ReadOnly));
    QCOMPARE(preserved.readAll(), original);
    QCOMPARE(entries(directory.path()), QStringList{QStringLiteral("UWF.ini")});
    if (directoryTarget) QCOMPARE(entries(path), QStringList{QStringLiteral("preserve.txt")});
  }

  void concurrentCreatorsPublishExactlyOneCompleteFile() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = directory.filePath(QStringLiteral("UWF.ini"));
    const QString firstText = QStringLiteral("Creator A · 第一份\n").repeated(4096);
    const QString secondText = QStringLiteral("Creator B · 第二份\n").repeated(4096);
    std::promise<void> ready;
    const auto start = ready.get_future().share();
    auto first = std::async(std::launch::async, [path, firstText, start] {
      start.wait();
      return uwf::windows::createUtf8TextFile(path, firstText);
    });
    auto second = std::async(std::launch::async, [path, secondText, start] {
      start.wait();
      return uwf::windows::createUtf8TextFile(path, secondText);
    });
    ready.set_value();
    const auto firstResult = first.get();
    const auto secondResult = second.get();
    QVERIFY2((firstResult.outcome == Outcome::Created && secondResult.outcome == Outcome::AlreadyExists) ||
                 (firstResult.outcome == Outcome::AlreadyExists && secondResult.outcome == Outcome::Created),
             qUtf8Printable(QStringLiteral("First creator: %1; second creator: %2").arg(firstResult.error, secondResult.error)));
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), (firstResult.outcome == Outcome::Created ? firstText : secondText).toUtf8());
    QCOMPARE(entries(directory.path()), QStringList{QStringLiteral("UWF.ini")});
  }

  void missingParentFailsWithoutCreatingDirectoriesOrPartialFiles() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto result = uwf::windows::createUtf8TextFile(directory.filePath(QStringLiteral("missing/nested/UWF.ini")), QStringLiteral("contents"));
    QCOMPARE(result.outcome, Outcome::Failed);
    QVERIFY(!result.error.isEmpty());
    QVERIFY(entries(directory.path()).isEmpty());
  }

  void rejectsRelativePaths_data() {
    QTest::addColumn<QString>("path");
    QTest::newRow("empty") << QString{};
    QTest::newRow("current-directory") << QStringLiteral(".");
    QTest::newRow("filename") << QStringLiteral("UWF.ini");
    QTest::newRow("relative-directory") << QStringLiteral("relative/UWF.ini");
    QTest::newRow("drive-relative") << QStringLiteral("C:UWF.ini");
  }

  void rejectsRelativePaths() {
    QFETCH(QString, path);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto previous = QDir::currentPath();
    const auto restore = qScopeGuard([&] { QDir::setCurrent(previous); });
    QVERIFY(QDir::setCurrent(directory.path()));
    const auto result = uwf::windows::createUtf8TextFile(path, QStringLiteral("contents"));
    QCOMPARE(result.outcome, Outcome::Failed);
    QVERIFY(!result.error.isEmpty());
    QVERIFY(entries(directory.path()).isEmpty());
  }
};
}  // namespace

QTEST_APPLESS_MAIN(WindowsTextFileCreationTests)
#include "WindowsTextFileCreationTests.moc"
