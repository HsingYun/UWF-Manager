// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 HsingYun (iakext@gmail.com)
#include <QTest>
#include <array>
#include <cstdint>
#include <limits>
#include <string>

#include "application/overlay/files/OverlayFileCatalog.h"

namespace {

class OverlayCatalogTests final : public QObject {
  Q_OBJECT

 private slots:
  void overlayCatalogPreservesStreamsAndSaturatesSizes();
  void onlyActualRootMetadataNamesAreDiagnostic_data();
  void onlyActualRootMetadataNamesAreDiagnostic();
  void entryKindsPreserveDiagnosticPrecedence();
};

void OverlayCatalogTests::entryKindsPreserveDiagnosticPrecedence() {
  using namespace uwf::app;
  struct Case {
    bool directory, metadata;
    OverlayFileKind kind;
    const char* code;
  };
  constexpr std::array cases{Case{false, false, OverlayFileKind::File, "file"}, Case{true, false, OverlayFileKind::Directory, "dir"},
                             Case{false, true, OverlayFileKind::SystemMetadata, "metadata"}, Case{true, true, OverlayFileKind::SystemMetadata, "metadata"}};
  for (const auto& item : cases) {
    OverlayFileEntry entry;
    entry.isDirectory = item.directory;
    entry.isSystemMetadata = item.metadata;
    QCOMPARE(overlayFileKind(entry), item.kind);
    QCOMPARE(QByteArray(overlayFileKindCode(item.kind)), QByteArray(item.code));
  }
}

void OverlayCatalogTests::onlyActualRootMetadataNamesAreDiagnostic_data() {
  QTest::addColumn<QString>("rawName");
  QTest::addColumn<QString>("absolutePath");
  QTest::addColumn<bool>("metadata");
  QTest::addColumn<bool>("directory");
  struct Path {
    const char* label;
    const char16_t* path;
    bool metadata;
  };
  constexpr std::array roots{Path{"ordinary-root", u"\\$Work", false},
                             Path{"ordinary-nested", u"\\Docs\\$Budget.txt", false},
                             Path{"ordinary-tree", u"\\$Work\\file.txt", false},
                             Path{"ordinary-reserved-leaf", u"\\Docs\\$Mft", false},
                             Path{"ordinary-reserved-tree", u"\\Docs\\$Extend\\$Quota", false},
                             Path{"metadata-file", u"\\$Mft", true},
                             Path{"metadata-case", u"\\$mFT", true},
                             Path{"metadata-mirror", u"\\$MftMirr", true},
                             Path{"metadata-log", u"\\$LogFile", true},
                             Path{"metadata-volume", u"\\$Volume", true},
                             Path{"metadata-attributes", u"\\$AttrDef", true},
                             Path{"metadata-bitmap", u"\\$Bitmap", true},
                             Path{"metadata-boot", u"\\$Boot", true},
                             Path{"metadata-bad-clusters", u"\\$BadClus", true},
                             Path{"metadata-security", u"\\$Secure", true},
                             Path{"metadata-case-table", u"\\$UpCase", true},
                             Path{"metadata-extended", u"\\$Extend", true},
                             Path{"metadata-subtree", u"\\$eXtEnD\\$UsnJrnl", true},
                             Path{"near-file", u"\\$MftBackup", false},
                             Path{"near-tree", u"\\$ExtendBackup\\$Quota", false},
                             Path{"literal-space", u"\\$Mft ", false},
                             Path{"ordinal-distinct", u"\\$\u017fecure", false}};
  for (const auto& row : roots)
    for (const bool alternateStream : {false, true}) {
      const auto path = QString::fromUtf16(row.path);
      QTest::newRow(qPrintable(QString::fromLatin1(row.label) + (alternateStream ? QStringLiteral("/ADS") : QStringLiteral("/plain"))))
          << path + (alternateStream ? QStringLiteral(":diagnostic:$DATA") : QString{}) << QStringLiteral("C:") + path << row.metadata << false;
    }
  QTest::newRow("directory-stream-type") << QStringLiteral("\\Directory:$INDEX_ALLOCATION") << QStringLiteral("C:\\Directory") << false << true;
  QTest::newRow("named-directory-stream") << QStringLiteral("\\Directory:$I30:$index_allocation") << QStringLiteral("C:\\Directory") << false << true;
  QTest::newRow("data-stream-with-type-like-name") << QStringLiteral("\\Document:$INDEX_ALLOCATION_notes:$DATA") << QStringLiteral("C:\\Document") << false
                                                   << false;
  QTest::newRow("data-stream-with-type-name") << QStringLiteral("\\Document:$INDEX_ALLOCATION:$DATA") << QStringLiteral("C:\\Document") << false << false;
}
void OverlayCatalogTests::onlyActualRootMetadataNamesAreDiagnostic() {
  QFETCH(QString, rawName);
  QFETCH(QString, absolutePath);
  QFETCH(bool, metadata);
  QFETCH(bool, directory);
  const auto entries = uwf::app::buildOverlayFileCatalog(QStringLiteral("C:"), {{rawName, {}, false, false, 1}});
  QCOMPARE(entries.size(), 1);
  QCOMPARE(entries.front().rawName, rawName);
  QCOMPARE(entries.front().absolutePath, absolutePath);
  QCOMPARE(entries.front().isSystemMetadata, metadata);
  QCOMPARE(entries.front().isDirectory, directory);
}

void OverlayCatalogTests::overlayCatalogPreservesStreamsAndSaturatesSizes() {
  const auto maximum = std::numeric_limits<qulonglong>::max();
  const auto entries = uwf::app::buildOverlayFileCatalog(QStringLiteral("C:"), {{QStringLiteral("\\Folder\\Item:stream"), {}, false, false, maximum - 4},
                                                                                {QStringLiteral("\\folder\\ITEM"), {}, false, false, 10},
                                                                                {QStringLiteral("\\$Mft:stream"), {}, false, false, 3},
                                                                                {QStringLiteral("\\DIR:$INDEX_ALLOCATION"), {}, false, false, 0}});
  QCOMPARE(entries.size(), 3);
  QVERIFY(entries[0].isSystemMetadata);
  QCOMPARE(entries[0].absolutePath, QStringLiteral("C:\\$Mft"));
  QVERIFY(entries[1].isDirectory);
  QCOMPARE(entries[1].absolutePath, QStringLiteral("C:\\DIR"));
  QCOMPARE(entries[2].rawName, QStringLiteral("\\Folder\\Item:stream"));
  QCOMPARE(entries[2].absolutePath, QStringLiteral("C:\\Folder\\Item"));
  QCOMPARE(entries[2].fileSize, maximum);
  QCOMPARE(uwf::app::overlayFileCatalogSize(entries), maximum);
  QCOMPARE(uwf::app::overlayFileCatalogSize({}), qulonglong{0});
}

}  // namespace

QTEST_APPLESS_MAIN(OverlayCatalogTests)

#include "OverlayCatalogTests.moc"
