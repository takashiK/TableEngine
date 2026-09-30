#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPixmap>
#include <QSettings>
#include <QStandardItem>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <atomic>
#include "utils/TeRemoteAccess.h"
#include "utils/TeFileIconProvider.h"
#include "utils/TeFileInfo.h"
#include "utils/TeThumbnailProvider.h"
#include "TeSettings.h"

using namespace ::testing;
using TeRemoteAccess::Kind;

namespace {

QImage iconImage(const QIcon& icon) { return icon.pixmap(16, 16).toImage(); }

TeRemoteAccess::DriveKindResolver resolver()
{
    return [](QChar letter) {
        if (letter == QLatin1Char('W')) return Kind::Wsl;
        if (letter == QLatin1Char('Z')) return Kind::Network;
        return Kind::Local;
    };
}

class CountingProvider : public QAbstractFileIconProvider
{
public:
    using QAbstractFileIconProvider::icon;
    QIcon icon(const QFileInfo&) const override { ++iconCalls; return QIcon(); }
    QString type(const QFileInfo&) const override { ++typeCalls; return QStringLiteral("native"); }
    mutable std::atomic<int> iconCalls{0};
    mutable std::atomic<int> typeCalls{0};
};

} // namespace

class tst_TeRemoteAccess : public ::testing::Test {
protected:
    void SetUp() override {
        QCoreApplication::setOrganizationName("TableEngine_Test");
        QCoreApplication::setApplicationName("TeRemoteAccessTest");
        QSettings s;
        s.remove("folder/remote_access");
        TeRemoteAccess::resetModes();
    }
    void TearDown() override {
        TeRemoteAccess::setClassifierForTesting({});
        QSettings s;
        s.remove("folder/remote_access");
        TeRemoteAccess::resetModes();
    }
};

// ── classification ───────────────────────────────────────────────────────────

TEST_F(tst_TeRemoteAccess, classify_wsl_unc_paths)
{
    EXPECT_EQ(TeRemoteAccess::classifyPath("//wsl$/Ubuntu/home", resolver(), {}), Kind::Wsl);
    EXPECT_EQ(TeRemoteAccess::classifyPath("\\\\wsl.localhost\\Ubuntu\\home", resolver(), {}), Kind::Wsl);
    EXPECT_EQ(TeRemoteAccess::classifyPath("\\\\?\\UNC\\wsl$\\Ubuntu", resolver(), {}), Kind::Wsl);
}

TEST_F(tst_TeRemoteAccess, classify_network_unc_paths)
{
    EXPECT_EQ(TeRemoteAccess::classifyPath("//server/share/a", resolver(), {}), Kind::Network);
    EXPECT_EQ(TeRemoteAccess::classifyPath("\\\\server\\share", resolver(), {}), Kind::Network);
}

TEST_F(tst_TeRemoteAccess, classify_drive_letters_use_resolver)
{
    EXPECT_EQ(TeRemoteAccess::classifyPath("w:/home", resolver(), {}), Kind::Wsl);
    EXPECT_EQ(TeRemoteAccess::classifyPath("Z:\\share", resolver(), {}), Kind::Network);
    EXPECT_EQ(TeRemoteAccess::classifyPath("C:/Windows", resolver(), {}), Kind::Local);
    EXPECT_EQ(TeRemoteAccess::classifyPath("C:/Windows", nullptr, {}), Kind::Local);
}

TEST_F(tst_TeRemoteAccess, classify_onedrive_only_under_configured_roots)
{
	const QStringList roots{"C:\\Users\\me\\OneDrive - Corp", "C:/Users/me/OneDrive/"};
	EXPECT_EQ(TeRemoteAccess::classifyPath("C:/Users/me/OneDrive - Corp/doc", resolver(), roots), Kind::OneDrive);
	EXPECT_EQ(TeRemoteAccess::classifyPath("c:/users/me/onedrive - corp", resolver(), roots), Kind::OneDrive);
	EXPECT_EQ(TeRemoteAccess::classifyPath("C:/Users/me/OneDrive/x", resolver(), roots), Kind::OneDrive);
}

TEST_F(tst_TeRemoteAccess, classify_onedrive_named_folders_without_root_are_local)
{
	EXPECT_EQ(TeRemoteAccess::classifyPath("C:/Users/me/OneDrive/x", resolver(), {}), Kind::Local);
	EXPECT_EQ(TeRemoteAccess::classifyPath("D:/work/OneDrive - Backup/x", resolver(), {"C:/Users/me/OneDrive"}), Kind::Local);
	EXPECT_EQ(TeRemoteAccess::classifyPath("C:/Users/me/OneDrive - Corp2/x", resolver(), {"C:/Users/me/OneDrive - Corp"}), Kind::Local);
}

// ── drive probe / lexical root ───────────────────────────────────────────────

TEST_F(tst_TeRemoteAccess, drive_probe_connected_network_is_cacheable)
{
	const auto r = TeRemoteAccess::resolveDriveProbe(4, 0, "\\\\server\\share");
	EXPECT_EQ(r.kind, Kind::Network);
	EXPECT_TRUE(r.cacheable);
	const auto w = TeRemoteAccess::resolveDriveProbe(4, 0, "\\\\wsl$\\Ubuntu");
	EXPECT_EQ(w.kind, Kind::Wsl);
	EXPECT_TRUE(w.cacheable);
}

TEST_F(tst_TeRemoteAccess, drive_probe_disconnected_mapping_is_not_cached)
{
	const auto r = TeRemoteAccess::resolveDriveProbe(1, 1201, "\\\\server\\share");
	EXPECT_EQ(r.kind, Kind::Network);
	EXPECT_FALSE(r.cacheable);
	const auto w = TeRemoteAccess::resolveDriveProbe(1, 1201, "\\\\wsl.localhost\\Ubuntu");
	EXPECT_EQ(w.kind, Kind::Wsl);
	EXPECT_FALSE(w.cacheable);
}

TEST_F(tst_TeRemoteAccess, drive_probe_uncertain_results_are_not_cached)
{
	const auto remoteUnreadable = TeRemoteAccess::resolveDriveProbe(4, 2250, {});
	EXPECT_EQ(remoteUnreadable.kind, Kind::Network);
	EXPECT_FALSE(remoteUnreadable.cacheable);

	const auto missing = TeRemoteAccess::resolveDriveProbe(1, 2250, {});
	EXPECT_EQ(missing.kind, Kind::Local);
	EXPECT_FALSE(missing.cacheable);

	const auto fixed = TeRemoteAccess::resolveDriveProbe(3, 0, {});
	EXPECT_EQ(fixed.kind, Kind::Local);
	EXPECT_TRUE(fixed.cacheable);
}

TEST_F(tst_TeRemoteAccess, uncertain_drive_probe_is_reused_within_ttl_then_reprobed)
{
    TeRemoteAccess::clearDriveCache();
    int probes = 0;
    Kind next = Kind::Network;
    const auto probe = [&](QChar) {
        ++probes;
        return TeRemoteAccess::DriveResolution{next, false};
    };

    EXPECT_EQ(TeRemoteAccess::resolveDriveCached('Y', probe, 1000), Kind::Network);
    EXPECT_EQ(TeRemoteAccess::resolveDriveCached('Y', probe, 1000 + TeRemoteAccess::DRIVE_UNCERTAIN_TTL_MS - 1), Kind::Network);
    EXPECT_EQ(probes, 1);

    next = Kind::Local; // reconnected
    EXPECT_EQ(TeRemoteAccess::resolveDriveCached('Y', probe, 1000 + TeRemoteAccess::DRIVE_UNCERTAIN_TTL_MS), Kind::Local);
    EXPECT_EQ(probes, 2);
    TeRemoteAccess::clearDriveCache();
}

TEST_F(tst_TeRemoteAccess, clear_drive_cache_drops_uncertain_and_certain_entries)
{
    TeRemoteAccess::clearDriveCache();
    int probes = 0;
    bool cacheable = false;
    const auto probe = [&](QChar) {
        ++probes;
        return TeRemoteAccess::DriveResolution{Kind::Network, cacheable};
    };

    TeRemoteAccess::resolveDriveCached('Y', probe, 0);
    TeRemoteAccess::clearDriveCache();
    TeRemoteAccess::resolveDriveCached('Y', probe, 1);
    EXPECT_EQ(probes, 2);

    cacheable = true;
    TeRemoteAccess::clearDriveCache();
    TeRemoteAccess::resolveDriveCached('Y', probe, 0);
    TeRemoteAccess::resolveDriveCached('Y', probe, 1'000'000);
    EXPECT_EQ(probes, 3);
    TeRemoteAccess::clearDriveCache();
    TeRemoteAccess::resolveDriveCached('Y', probe, 2);
    EXPECT_EQ(probes, 4);
    TeRemoteAccess::clearDriveCache();
}

TEST_F(tst_TeRemoteAccess, lexical_root_detection)
{
	EXPECT_TRUE(TeRemoteAccess::isLexicalRoot("C:"));
	EXPECT_TRUE(TeRemoteAccess::isLexicalRoot("C:\\"));
	EXPECT_TRUE(TeRemoteAccess::isLexicalRoot("c:/"));
	EXPECT_TRUE(TeRemoteAccess::isLexicalRoot("/"));
	EXPECT_TRUE(TeRemoteAccess::isLexicalRoot("//server/share"));
	EXPECT_TRUE(TeRemoteAccess::isLexicalRoot("\\\\wsl$\\Ubuntu\\"));
	EXPECT_FALSE(TeRemoteAccess::isLexicalRoot("C:/Users"));
	EXPECT_FALSE(TeRemoteAccess::isLexicalRoot("//server/share/dir"));
	EXPECT_FALSE(TeRemoteAccess::isLexicalRoot("/home"));
	EXPECT_FALSE(TeRemoteAccess::isLexicalRoot(""));
}

TEST_F(tst_TeRemoteAccess, classify_local_paths_stay_local)
{
    EXPECT_EQ(TeRemoteAccess::classifyPath("C:/Users/me/Documents", resolver(), {"C:/Users/me/OneDrive"}), Kind::Local);
    EXPECT_EQ(TeRemoteAccess::classifyPath("C:/Users/me/OneDriveBackup/x", resolver(), {"C:/Users/me/OneDrive"}), Kind::Local);
    EXPECT_EQ(TeRemoteAccess::classifyPath("/home/user", resolver(), {}), Kind::Local);
    EXPECT_EQ(TeRemoteAccess::classifyPath("", resolver(), {}), Kind::Local);
}

// ── modes ────────────────────────────────────────────────────────────────────

TEST_F(tst_TeRemoteAccess, default_mode_is_icon_suppression)
{
    for (Kind k : {Kind::Wsl, Kind::Network, Kind::OneDrive})
        EXPECT_EQ(TeRemoteAccess::mode(k), TeSettings::REMOTE_ACCESS_SUPPRESS_ICON);
}

TEST_F(tst_TeRemoteAccess, local_is_always_normal)
{
    TeRemoteAccess::setMode(Kind::Local, TeSettings::REMOTE_ACCESS_SUPPRESS_AUTO);
    EXPECT_EQ(TeRemoteAccess::mode(Kind::Local), TeSettings::REMOTE_ACCESS_NORMAL);
    EXPECT_EQ(TeRemoteAccess::settingKey(Kind::Local), nullptr);
}

TEST_F(tst_TeRemoteAccess, kinds_are_independent)
{
    TeRemoteAccess::setMode(Kind::Wsl, TeSettings::REMOTE_ACCESS_NORMAL);
    TeRemoteAccess::setMode(Kind::Network, TeSettings::REMOTE_ACCESS_SUPPRESS_AUTO);
    EXPECT_EQ(TeRemoteAccess::mode(Kind::Wsl), TeSettings::REMOTE_ACCESS_NORMAL);
    EXPECT_EQ(TeRemoteAccess::mode(Kind::Network), TeSettings::REMOTE_ACCESS_SUPPRESS_AUTO);
    EXPECT_EQ(TeRemoteAccess::mode(Kind::OneDrive), TeSettings::REMOTE_ACCESS_SUPPRESS_ICON);
}

TEST_F(tst_TeRemoteAccess, settings_round_trip_and_clamp)
{
    QSettings s;
    s.setValue(TeRemoteAccess::settingKey(Kind::Wsl), int(TeSettings::REMOTE_ACCESS_NORMAL));
    s.setValue(TeRemoteAccess::settingKey(Kind::Network), int(TeSettings::REMOTE_ACCESS_SUPPRESS_AUTO));
    s.setValue(TeRemoteAccess::settingKey(Kind::OneDrive), 99);

    EXPECT_TRUE(TeRemoteAccess::loadFromSettings());
    EXPECT_EQ(TeRemoteAccess::mode(Kind::Wsl), TeSettings::REMOTE_ACCESS_NORMAL);
    EXPECT_EQ(TeRemoteAccess::mode(Kind::Network), TeSettings::REMOTE_ACCESS_SUPPRESS_AUTO);
    EXPECT_EQ(TeRemoteAccess::mode(Kind::OneDrive), TeSettings::REMOTE_ACCESS_NORMAL);
    EXPECT_FALSE(TeRemoteAccess::loadFromSettings());
}

TEST_F(tst_TeRemoteAccess, settings_removed_keys_restore_default)
{
    QSettings s;
    s.setValue(TeRemoteAccess::settingKey(Kind::Wsl), int(TeSettings::REMOTE_ACCESS_NORMAL));
    EXPECT_TRUE(TeRemoteAccess::loadFromSettings());
    EXPECT_EQ(TeRemoteAccess::mode(Kind::Wsl), TeSettings::REMOTE_ACCESS_NORMAL);

    s.remove("folder/remote_access");
    EXPECT_TRUE(TeRemoteAccess::loadFromSettings());
    EXPECT_EQ(TeRemoteAccess::mode(Kind::Wsl), TeSettings::REMOTE_ACCESS_SUPPRESS_ICON);
}

TEST_F(tst_TeRemoteAccess, policy_queries_follow_mode)
{
    TeRemoteAccess::setClassifierForTesting([](const QString& p) {
        return p.startsWith("remote") ? Kind::Wsl : Kind::Local;
    });

    TeRemoteAccess::setMode(Kind::Wsl, TeSettings::REMOTE_ACCESS_SUPPRESS_ICON);
    EXPECT_TRUE(TeRemoteAccess::suppressIconAccess("remote/a"));
    EXPECT_FALSE(TeRemoteAccess::suppressAutoContent("remote/a"));

    TeRemoteAccess::setMode(Kind::Wsl, TeSettings::REMOTE_ACCESS_SUPPRESS_AUTO);
    EXPECT_TRUE(TeRemoteAccess::suppressIconAccess("remote/a"));
    EXPECT_TRUE(TeRemoteAccess::suppressAutoContent("remote/a"));

    TeRemoteAccess::setMode(Kind::Wsl, TeSettings::REMOTE_ACCESS_NORMAL);
    EXPECT_FALSE(TeRemoteAccess::suppressIconAccess("remote/a"));
    EXPECT_FALSE(TeRemoteAccess::suppressAutoContent("remote/a"));

    TeRemoteAccess::setMode(Kind::Wsl, TeSettings::REMOTE_ACCESS_SUPPRESS_AUTO);
    EXPECT_FALSE(TeRemoteAccess::suppressIconAccess("local/a"));
    EXPECT_FALSE(TeRemoteAccess::suppressAutoContent("local/a"));
}

// ── icon provider ────────────────────────────────────────────────────────────

TEST_F(tst_TeRemoteAccess, icon_provider_skips_fallback_when_suppressed)
{
    TeRemoteAccess::setClassifierForTesting([](const QString&) { return Kind::Network; });
    auto fallback = std::make_unique<CountingProvider>();
    CountingProvider* counter = fallback.get();
    TeFileIconProvider provider(std::move(fallback));

    provider.icon(QFileInfo("remote/a.txt"));
    EXPECT_EQ(provider.type(QFileInfo("remote/a.txt")), QString("TXT File"));
    EXPECT_EQ(provider.type(QFileInfo("remote/noext")), QString("File"));
    provider.folderIcon("remote");
    EXPECT_EQ(counter->iconCalls.load(), 0);
    EXPECT_EQ(counter->typeCalls.load(), 0);
}

TEST_F(tst_TeRemoteAccess, icon_provider_delegates_in_normal_mode_and_locally)
{
    auto fallback = std::make_unique<CountingProvider>();
    CountingProvider* counter = fallback.get();
    TeFileIconProvider provider(std::move(fallback));

    TeRemoteAccess::setClassifierForTesting([](const QString&) { return Kind::Wsl; });
    TeRemoteAccess::setMode(Kind::Wsl, TeSettings::REMOTE_ACCESS_NORMAL);
    provider.icon(QFileInfo("//wsl$/x/a.txt"));
    EXPECT_EQ(provider.type(QFileInfo("//wsl$/x/a.txt")), QString("native"));
    EXPECT_EQ(counter->iconCalls.load(), 1);
    EXPECT_EQ(counter->typeCalls.load(), 1);

    TeRemoteAccess::setClassifierForTesting([](const QString&) { return Kind::Local; });
    TeRemoteAccess::setMode(Kind::Wsl, TeSettings::REMOTE_ACCESS_SUPPRESS_AUTO);
    provider.icon(QFileInfo("a.txt"));
    EXPECT_EQ(provider.type(QFileInfo("a.txt")), QString("native"));
    EXPECT_EQ(counter->iconCalls.load(), 2);
    EXPECT_EQ(counter->typeCalls.load(), 2);
}

TEST_F(tst_TeRemoteAccess, folders_only_provider_returns_folder_without_stat_when_suppressed)
{
    class KindIconProvider : public CountingProvider {
    public:
        using CountingProvider::icon;
        QIcon icon(IconType type) const override
        {
            QPixmap pm(16, 16);
            pm.fill(type == Folder ? Qt::red : Qt::blue);
            return QIcon(pm);
        }
    };

    TeRemoteAccess::setClassifierForTesting([](const QString&) { return Kind::Network; });
    TeFileIconProvider mixed(std::make_unique<KindIconProvider>());
    TeFileIconProvider foldersOnly(std::make_unique<KindIconProvider>(), true);

    // The path does not exist, so a stat-based isDir() would report a file.
    const QFileInfo missing("Q:/te_missing_dir/dir");
    EXPECT_EQ(iconImage(mixed.icon(missing)), iconImage(mixed.icon(QAbstractFileIconProvider::File)));
    EXPECT_EQ(iconImage(foldersOnly.icon(missing)), iconImage(foldersOnly.icon(QAbstractFileIconProvider::Folder)));
}

TEST_F(tst_TeRemoteAccess, icon_provider_detects_root_lexically)
{
    TeRemoteAccess::setClassifierForTesting([](const QString&) { return Kind::Network; });
    auto fallback = std::make_unique<CountingProvider>();
    TeFileIconProvider provider(std::move(fallback));

    EXPECT_EQ(provider.type(QFileInfo("Q:/")), QString("Drive"));
    EXPECT_EQ(provider.type(QFileInfo("//server/share")), QString("Drive"));
    EXPECT_EQ(provider.type(QFileInfo("Q:/dir")), QString("File"));
}

// ── direct icon bypass routes ────────────────────────────────────────────────

TEST_F(tst_TeRemoteAccess, thumbnail_provider_placeholder_is_generic_when_suppressed)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString png = dir.filePath("a.png");
    QImage img(8, 8, QImage::Format_RGB32);
    img.fill(Qt::red);
    ASSERT_TRUE(img.save(png));

    TeRemoteAccess::setClassifierForTesting([](const QString&) { return Kind::Network; });
    const QIcon icon = TeThumbnailProvider::iconProvider()->icon(QFileInfo(png));
    EXPECT_EQ(iconImage(icon), iconImage(TeFileIconProvider::instance()->icon(QAbstractFileIconProvider::File)));
}

TEST_F(tst_TeRemoteAccess, file_info_export_uses_type_icon_on_remote_paths)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString txt = dir.filePath("a.txt");
    QFile f(txt);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    TeFileInfo info;
    info.path = txt;
    info.type = TeFileInfo::EN_DIR; // type-based icon on remote paths, never the on-disk file

    TeRemoteAccess::setClassifierForTesting([](const QString&) { return Kind::Wsl; });
    QList<QStandardItem*> items = info.exportItem(true);
    ASSERT_FALSE(items.isEmpty());
    EXPECT_EQ(iconImage(items.first()->icon()),
              iconImage(TeFileIconProvider::instance()->icon(QAbstractFileIconProvider::Folder)));
    qDeleteAll(items);
}

// ── translations ─────────────────────────────────────────────────────────────

TEST_F(tst_TeRemoteAccess, japanese_translations_cover_remote_access_labels)
{
    const QString tsPath = QDir(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath())
        .filePath("../../translations/tableengine_ja.ts");
    QFile file(tsPath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly)) << tsPath.toStdString();

    QHash<QString, QString> translated; // "context\nsource" -> translation
    QXmlStreamReader xml(&file);
    QString context, source;
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement())
            continue;
        if (xml.name() == QLatin1String("name")) {
            context = xml.readElementText();
        } else if (xml.name() == QLatin1String("source")) {
            source = xml.readElementText();
        } else if (xml.name() == QLatin1String("translation")) {
            const bool unfinished = xml.attributes().value("type") == QLatin1String("unfinished");
            const QString translation = xml.readElementText();
            if (!unfinished && !translation.isEmpty())
                translated.insert(context + "\n" + source, translation);
        }
    }
    ASSERT_FALSE(xml.hasError());
    const QStringList required = {
        "TeOptionSetting\nRemote Locations (WSL / Network Share / OneDrive)",
        "TeOptionSetting\nWSL:",
        "TeOptionSetting\nNetwork share (Samba):",
        "TeOptionSetting\nOneDrive:",
        "TeOptionSetting\nIcons only (no per-file icon lookups)",
        "TeOptionSetting\nIcons and previews (no automatic previews)",
        "TeOptionSetting\nNormal (existing behavior)",
        "TeFileIconProvider\nDrive",
        "TeFileIconProvider\nFolder",
        "TeFileIconProvider\nFile",
        "TeFileIconProvider\n%1 File",
    };
    for (const QString& key : required)
        EXPECT_TRUE(translated.contains(key)) << key.toStdString();
}
