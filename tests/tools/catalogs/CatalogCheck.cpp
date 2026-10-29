// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include "CatalogCheck.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QSet>
#include <QXmlStreamReader>
#include <stdexcept>

namespace uwf::test::tools {
namespace {

[[noreturn]] void invalid(const QString& message) { throw std::runtime_error(message.toStdString()); }

QMap<QString, int> placeholders(const QString& text) {
  QMap<QString, int> result;
  for (qsizetype i = 0; i < text.size(); ++i) {
    if (text[i] != u'%') continue;
    qsizetype end = i + 1;
    if (end < text.size() && text[end] == u'L') ++end;
    if (end < text.size() && text[end] == u'n') {
      ++end;
    } else if (end < text.size() && text[end].isDigit()) {
      while (end < text.size() && text[end].isDigit()) ++end;
    } else if (text.mid(i).startsWith(QLatin1String("%ProgramFiles%"))) {
      end = i + 14;
    } else {
      continue;
    }
    ++result[text.mid(i, end - i)];
    i = end - 1;
  }
  return result;
}

QStringList filterPatterns(const QString& text) {
  QStringList patterns;
  for (const auto filters = text.split(QLatin1String(";;")); const auto& filter : filters) {
    const auto trimmed = filter.trimmed();
    const auto start = trimmed.lastIndexOf(u'(');
    if (start < 0 || !trimmed.endsWith(u')')) invalid(QStringLiteral("invalid file filter"));
    auto wildcards = trimmed.mid(start + 1, trimmed.size() - start - 2).simplified().split(u' ', Qt::SkipEmptyParts);
    if (wildcards.isEmpty()) invalid(QStringLiteral("empty file filter"));
    wildcards.sort();
    wildcards.removeDuplicates();
    patterns.append(wildcards.join(u' '));
  }
  return patterns;
}

QString xmlMarkup(const QString& text) {
  static const QSet<QString> voidTags{QStringLiteral("br"),   QStringLiteral("hr"),   QStringLiteral("img"),
                                      QStringLiteral("meta"), QStringLiteral("link"), QStringLiteral("input")};
  QString result;
  QStringList openTags;
  for (qsizetype i = 0; i < text.size(); ++i) {
    if (text[i] == u'&') {
      const auto tail = text.mid(i);
      if (!tail.startsWith(QLatin1String("&amp;")) && !tail.startsWith(QLatin1String("&lt;")) && !tail.startsWith(QLatin1String("&gt;")) &&
          !tail.startsWith(QLatin1String("&quot;")) && !tail.startsWith(QLatin1String("&apos;")) && !tail.startsWith(QLatin1String("&#"))) {
        result += QLatin1String("&amp;");
        continue;
      }
    }
    if (text[i] != u'<') {
      result += text[i];
      continue;
    }
    qsizetype end = i + 1;
    QChar quote;
    for (; end < text.size(); ++end) {
      const auto character = text[end];
      if (!quote.isNull()) {
        if (character == quote) quote = QChar();
      } else if (character == u'\'' || character == u'"') {
        quote = character;
      } else if (character == u'>') {
        break;
      }
    }
    if (end == text.size()) invalid(QStringLiteral("unclosed rich-text tag"));
    QString tag = text.mid(i + 1, end - i - 1).trimmed();
    const bool closing = tag.startsWith(u'/');
    const qsizetype nameStart = closing ? 1 : 0;
    qsizetype nameEnd = nameStart;
    while (nameEnd < tag.size() && tag[nameEnd].isLetterOrNumber()) ++nameEnd;
    const QString name = tag.mid(nameStart, nameEnd - nameStart).toLower();
    tag.replace(nameStart, nameEnd - nameStart, name);
    if (!closing && voidTags.contains(name) && !tag.endsWith(u'/')) tag += u'/';
    // Qt accepts HTML paragraphs whose closing tag is implicit at the next
    // paragraph or end of the fragment; TS markup is not necessarily XHTML.
    if (!closing && name == QLatin1String("p") && !openTags.isEmpty() && openTags.back() == QLatin1String("p")) {
      result += QLatin1String("</p>");
      openTags.removeLast();
    }
    if (closing && !openTags.isEmpty() && openTags.back() == name)
      openTags.removeLast();
    else if (!closing && !name.isEmpty() && !tag.endsWith(u'/'))
      openTags.append(name);
    result += u'<' + tag + u'>';
    i = end;
  }
  while (!openTags.isEmpty() && openTags.back() == QLatin1String("p")) {
    result += QLatin1String("</p>");
    openTags.removeLast();
  }
  return QStringLiteral("<root>") + result + QStringLiteral("</root>");
}

QMap<QString, int> markupStructure(const QString& text) {
  QXmlStreamReader xml(xmlMarkup(text));
  QMap<QString, int> structure;
  while (!xml.atEnd()) {
    xml.readNext();
    if (!xml.isStartElement() || xml.name() == QLatin1String("root")) continue;
    QStringList attributes;
    for (const auto xmlAttributes = xml.attributes(); const auto& attribute : xmlAttributes) {
      const auto name = attribute.name().toString();
      // These attributes contain human-readable text; links and styling retain
      // their original values. Placeholder validation covers the whole message.
      const auto value = (name == QLatin1String("title") || name == QLatin1String("alt")) && !attribute.value().isEmpty() ? QStringLiteral("translated-text")
                                                                                                                          : attribute.value().toString();
      attributes.append(name + u'=' + value);
    }
    attributes.sort();
    ++structure[xml.name().toString() + u'|' + QString::fromUtf8(QJsonDocument(QJsonArray::fromStringList(attributes)).toJson(QJsonDocument::Compact))];
  }
  if (xml.hasError()) invalid(QStringLiteral("invalid rich text: ") + xml.errorString());
  return structure;
}

bool identicalIsValid(const QString& source, const QString& language, const bool qtSupplement) {
  if (qtSupplement) {
    return QSet<QString>{QStringLiteral("OK"), QStringLiteral("Alt+Left"), QStringLiteral("Alt+Right"), QStringLiteral("Alt+Up")}.contains(source) ||
           (language == QLatin1String("id") && QSet<QString>{QStringLiteral("Drive"), QStringLiteral("Folder"), QStringLiteral("Alias")}.contains(source));
  }
  static const QSet<QString> shared{QStringLiteral("Microsoft Defender"), QStringLiteral("System Center Endpoint Protection"), QStringLiteral("#")};
  static const QMap<QString, QSet<QString>> reviewed{
      {QStringLiteral("de"),
       {QStringLiteral("OK"), QStringLiteral("Filter"), QStringLiteral("Overlay"), QStringLiteral("Name"), QStringLiteral("✓ Filter: %1"),
        QStringLiteral("Tag"), QStringLiteral("Status"), QStringLiteral("Administrator"), QStringLiteral("Information"), QStringLiteral("System"),
        QStringLiteral("Version %1"), QStringLiteral("Desktop"), QStringLiteral("Downloads")}},
      {QStringLiteral("es"), {QStringLiteral("No"), QStringLiteral("Editable"), QStringLiteral("Error")}},
      {QStringLiteral("fr"),
       {QStringLiteral("OK"), QStringLiteral("Type"), QStringLiteral("Overlay"), QStringLiteral("Message"), QStringLiteral("Architecture"),
        QStringLiteral("Information"), QStringLiteral("Module"), QStringLiteral("Version %1"), QStringLiteral("Documents")}},
      {QStringLiteral("id"),
       {QStringLiteral("OK"), QStringLiteral("File…"), QStringLiteral("Folder…"), QStringLiteral("Filter"), QStringLiteral("Overlay"), QStringLiteral("Data"),
        QStringLiteral("(Default)"), QStringLiteral("Log"), QStringLiteral("✓ Filter: %1"), QStringLiteral("Tag"), QStringLiteral("Status"),
        QStringLiteral("Detail"), QStringLiteral("Administrator"), QStringLiteral("Debug"), QStringLiteral("Disk"), QStringLiteral("File"),
        QStringLiteral("Folder"), QStringLiteral("Target"), QStringLiteral("Desktop")}},
      {QStringLiteral("it"),
       {QStringLiteral("OK"), QStringLiteral("No"), QStringLiteral("File…"), QStringLiteral("Overlay"), QStringLiteral("Tag"), QStringLiteral("Account"),
        QStringLiteral("Debug"), QStringLiteral("File"), QStringLiteral("Desktop")}},
      {QStringLiteral("ja"), {QStringLiteral("OK")}},
      {QStringLiteral("nl"),
       {QStringLiteral("OK"), QStringLiteral("Filter"), QStringLiteral("Type"), QStringLiteral("Overlay"), QStringLiteral("✓ Filter: %1"),
        QStringLiteral("Tag"), QStringLiteral("Status"), QStringLiteral("Background Intelligent Transfer Service (BITS)"), QStringLiteral("── Batch %1 ──"),
        QStringLiteral("%1 items"), QStringLiteral("%1 items · %2"), QStringLiteral("%1 records"), QStringLiteral("Account"), QStringLiteral("Module"),
        QStringLiteral("Downloads")}},
      {QStringLiteral("pl"),
       {QStringLiteral("OK"), QStringLiteral("Folder…"), QStringLiteral("Tag"), QStringLiteral("Administrator"), QStringLiteral("Folder"),
        QStringLiteral("System")}},
      {QStringLiteral("pt_BR"), {QStringLiteral("OK"), QStringLiteral("Log"), QStringLiteral("Tag"), QStringLiteral("Status"), QStringLiteral("Downloads")}},
      {QStringLiteral("pt_PT"), {QStringLiteral("OK"), QStringLiteral("Tag"), QStringLiteral("Status")}},
      {QStringLiteral("vi"), {QStringLiteral("OK")}},
      {QStringLiteral("tr"), {QStringLiteral("Disk")}},
  };
  return shared.contains(source) || reviewed.value(language).contains(source);
}

}  // namespace

TranslationCatalog readCatalog(QIODevice& input) {
  QXmlStreamReader xml(&input);
  if (!xml.readNextStartElement() || xml.name() != QLatin1String("TS")) invalid(QStringLiteral("expected a TS document"));
  TranslationCatalog catalog;
  catalog.language = xml.attributes().value(QLatin1String("language")).toString();
  while (xml.readNextStartElement()) {
    if (xml.name() != QLatin1String("context")) {
      xml.skipCurrentElement();
      continue;
    }
    QString context;
    while (xml.readNextStartElement()) {
      if (xml.name() == QLatin1String("name")) {
        context = xml.readElementText();
      } else if (xml.name() == QLatin1String("message")) {
        TranslationMessage message;
        message.plural = xml.attributes().value(QLatin1String("numerus")) == QLatin1String("yes");
        QString source;
        QString comment;
        while (xml.readNextStartElement()) {
          if (xml.name() == QLatin1String("source")) {
            source = xml.readElementText(QXmlStreamReader::IncludeChildElements);
          } else if (xml.name() == QLatin1String("comment")) {
            comment = xml.readElementText();
          } else if (xml.name() == QLatin1String("translation")) {
            message.status = xml.attributes().value(QLatin1String("type")).toString();
            if (message.plural) {
              while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("numerusform"))
                  message.forms.append(xml.readElementText());
                else
                  xml.skipCurrentElement();
              }
            } else {
              message.forms.append(xml.readElementText(QXmlStreamReader::IncludeChildElements));
            }
          } else {
            xml.skipCurrentElement();
          }
        }
        if (context.isEmpty() || source.isEmpty()) invalid(QStringLiteral("message has no context or source"));
        if (!catalog.messages.emplace(MessageKey{context, source, comment}, message).second) invalid(QStringLiteral("duplicate message: ") + source);
      } else {
        xml.skipCurrentElement();
      }
    }
  }
  while (!xml.atEnd()) xml.readNext();
  if (xml.hasError()) invalid(xml.errorString());
  if (catalog.messages.empty()) invalid(QStringLiteral("empty translation catalog"));
  return catalog;
}

TranslationCatalog readCatalog(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) invalid(path + QStringLiteral(": ") + file.errorString());
  return readCatalog(file);
}

QStringList checkMessage(const MessageKey& key, const TranslationMessage& message, const QString& language, const bool qtSupplement) {
  const auto& source = std::get<1>(key);
  QStringList problems;
  if (message.forms.isEmpty() || message.status == QLatin1String("unfinished") || message.status == QLatin1String("vanished") ||
      message.status == QLatin1String("obsolete"))
    problems.append(QStringLiteral("missing or unfinished translation"));
  static const QStringList literals{QStringLiteral("HKLM\\SOFTWARE\\HsingYun\\UWF Manager"),
                                    QStringLiteral("HKLM\\SECURITY\\Policy\\Secrets\\$MACHINE.ACC"),
                                    QStringLiteral("HKLM\\BCD00000000"),
                                    QStringLiteral("HKLM\\SYSTEM"),
                                    QStringLiteral("HKLM\\SOFTWARE"),
                                    QStringLiteral("HKLM\\SAM"),
                                    QStringLiteral("HKLM\\SECURITY"),
                                    QStringLiteral("HKLM\\COMPONENTS"),
                                    QStringLiteral("HKLM\\SYSTEM\\CurrentControlSet\\Services\\UWFManagerEnhanced"),
                                    QStringLiteral("%ProgramFiles%\\UWF Manager\\Service\\UWF.exe"),
                                    QStringLiteral("Interfaces\\{GUID}\\{GUID}.xml"),
                                    QStringLiteral("UWF_Overlay.GetOverlayFiles"),
                                    QStringLiteral("DomainSecretKey"),
                                    QStringLiteral("TSCAL"),
                                    QStringLiteral("NTUSER.DAT"),
                                    QStringLiteral("bootstatuspolicy"),
                                    QStringLiteral("ignoreallfailures")};
  for (const auto& translated : message.forms) {
    if (translated.trimmed().isEmpty()) problems.append(QStringLiteral("empty translation"));
    if (placeholders(source) != placeholders(translated)) problems.append(QStringLiteral("placeholder mismatch"));
    const bool command = source.startsWith(QLatin1String("uwfmgr filter enable\n"));
    if (command) {
      if (source != translated) problems.append(QStringLiteral("modified command"));
    } else if (translated == source && !identicalIsValid(source, language, qtSupplement)) {
      problems.append(QStringLiteral("unreviewed untranslated source"));
    }
    for (const auto& literal : literals)
      if (source.contains(literal) && !translated.contains(literal)) problems.append(QStringLiteral("modified technical literal: ") + literal);
    if (source.startsWith(QLatin1String("Text files (")) || source.startsWith(QLatin1String("Command files (")) ||
        source.startsWith(QLatin1String("All files (")) ||
        (qtSupplement && source.endsWith(QLatin1String("(*)")))) {
      try {
        if (filterPatterns(source) != filterPatterns(translated)) problems.append(QStringLiteral("modified file filter"));
      } catch (const std::exception& error) {
        problems.append(QString::fromUtf8(error.what()));
      }
    }
    if (source.contains(QLatin1String("<p")) || source.contains(QLatin1String("<b")) || source.contains(QLatin1String("<span")) ||
        (qtSupplement && source.contains(u'<'))) {
      try {
        if (markupStructure(source) != markupStructure(translated)) problems.append(QStringLiteral("modified rich-text structure or attributes"));
      } catch (const std::exception& error) {
        problems.append(QString::fromUtf8(error.what()));
      }
    }
  }
  return problems;
}

QStringList checkCatalog(const TranslationCatalog& catalog, const TranslationCatalog& reference, const QString& language, const bool qtSupplement) {
  QStringList problems;
  if (catalog.language != language) problems.append(QStringLiteral("incorrect TS language"));
  for (const auto& [key, message] : reference.messages) {
    const auto found = catalog.messages.find(key);
    if (found == catalog.messages.end()) problems.append(QStringLiteral("missing message: ") + std::get<1>(key));
  }
  for (const auto& [key, message] : catalog.messages) {
    if (!reference.messages.contains(key)) problems.append(QStringLiteral("unexpected message: ") + std::get<1>(key));
    if (reference.messages.contains(key) && reference.messages.at(key).plural != message.plural)
      problems.append(QStringLiteral("changed numerus: ") + std::get<1>(key));
    for (const auto messageProblems = checkMessage(key, message, language, qtSupplement); const auto& problem : messageProblems)
      problems.append(std::get<1>(key).left(90) + QStringLiteral(": ") + problem);
  }
  return problems;
}


}  // namespace uwf::test::tools
