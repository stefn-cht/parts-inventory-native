#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleFactory>

#include "inventorywindow.h"
#include "themesettingsdialog.h"

namespace {

void applyTheme(QApplication& application, const QString& themeName, bool windowsPlatform) {
    const bool dark = themeName == QStringLiteral("Dark");
    const bool frost = themeName == QStringLiteral("Frost");
    const bool plasma = themeName == QStringLiteral("KDE Plasma");

    if (themeName == QStringLiteral("Classic XP")) {
    if (windowsPlatform) {
        QStyle* windowsStyle = QStyleFactory::create(QStringLiteral("windowsvista"));
        application.setStyle(windowsStyle ? windowsStyle : QStyleFactory::create(QStringLiteral("Fusion")));
    } else {
        application.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    }
    } else {
        application.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    }

    QPalette palette;
    if (dark) {
        palette.setColor(QPalette::Window, QColor("#252525"));
        palette.setColor(QPalette::WindowText, QColor("#f0f0f0"));
        palette.setColor(QPalette::Base, QColor("#1b1b1b"));
        palette.setColor(QPalette::AlternateBase, QColor("#303030"));
        palette.setColor(QPalette::Text, QColor("#f0f0f0"));
        palette.setColor(QPalette::Button, QColor("#353535"));
        palette.setColor(QPalette::ButtonText, QColor("#f0f0f0"));
        palette.setColor(QPalette::Highlight, QColor("#3974b8"));
        palette.setColor(QPalette::HighlightedText, Qt::white);
    } else if (frost) {
        palette.setColor(QPalette::Window, QColor("#eaf4fb"));
        palette.setColor(QPalette::WindowText, QColor("#193044"));
        palette.setColor(QPalette::Base, QColor("#ffffff"));
        palette.setColor(QPalette::AlternateBase, QColor("#dcecf7"));
        palette.setColor(QPalette::Text, QColor("#193044"));
        palette.setColor(QPalette::Button, QColor("#d8ebf7"));
        palette.setColor(QPalette::ButtonText, QColor("#193044"));
        palette.setColor(QPalette::Highlight, QColor("#77b7dc"));
        palette.setColor(QPalette::HighlightedText, Qt::white);
    } else if (plasma) {
        palette.setColor(QPalette::Window, QColor("#eff1f5"));
        palette.setColor(QPalette::WindowText, QColor("#232629"));
        palette.setColor(QPalette::Base, Qt::white);
        palette.setColor(QPalette::AlternateBase, QColor("#e8eaf0"));
        palette.setColor(QPalette::Text, QColor("#232629"));
        palette.setColor(QPalette::Button, QColor("#e4e7ed"));
        palette.setColor(QPalette::ButtonText, QColor("#232629"));
        palette.setColor(QPalette::Highlight, QColor("#3daee9"));
        palette.setColor(QPalette::HighlightedText, Qt::white);
    }

    if (dark || frost || plasma) {
        application.setPalette(palette);
    } else {
        application.setPalette(application.style()->standardPalette());
    }
}

void applyWidgetStyle(QApplication& application, const QString& styleName) {
    QStyle* style = QStyleFactory::create(styleName);
    if (!style && styleName == QStringLiteral("Windows")) {
        style = QStyleFactory::create(QStringLiteral("Fusion"));
    }
    if (style) {
        application.setStyle(style);
    }
}

} // namespace

QString inventoryDataPath() {
#ifdef Q_OS_WIN
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
        .filePath(QStringLiteral("parts-inventory/inventory.json"));
#else
    return QDir(QDir::homePath()).filePath(QStringLiteral(".local/share/parts-inventory/inventory.json"));
#endif
}

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("Parts Inventory"));
    application.setApplicationVersion(QStringLiteral(PARTS_INVENTORY_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Native Parts Inventory Manager"));
    parser.addHelpOption();
    QCommandLineOption simulateOsOption(QStringList() << QStringLiteral("simulate-os"),
        QStringLiteral("Use OS defaults for testing: windows or linux."), QStringLiteral("os"));
    parser.addOption(simulateOsOption);
    parser.process(application);
    const QString simulatedOs = parser.value(simulateOsOption).trimmed().toLower();
    const bool windowsPlatform = simulatedOs == QStringLiteral("windows") ||
#ifdef Q_OS_WIN
        simulatedOs != QStringLiteral("linux");
#else
        false;
#endif
    const bool forceSimulatedDefaults = !simulatedOs.isEmpty();

    const QString defaultTheme = windowsPlatform ? QStringLiteral("Classic XP") : QStringLiteral("Dark");
    const QString defaultWidgetStyle = windowsPlatform ? QStringLiteral("Windows") : QStringLiteral("Fusion");
    const QString defaultBaseColor = windowsPlatform ? QStringLiteral("#f0f0f0") : QStringLiteral("#83cceb");
    const QString defaultAlternateColor = windowsPlatform ? QStringLiteral("#d9d9d9") : QStringLiteral("#c0e6f5");
    const QString defaultHeaderColor = windowsPlatform ? QStringLiteral("#b0b0b0") : QStringLiteral("#4b9fc4");

    QSettings settings;
    const QString savedPath = settings.value(QStringLiteral("inventoryPath"), inventoryDataPath()).toString();
    const QString savedTheme = forceSimulatedDefaults ? defaultTheme : settings.value(QStringLiteral("theme"), defaultTheme).toString();
    const QString savedFont = forceSimulatedDefaults ? application.font().family() : settings.value(QStringLiteral("fontFamily"), application.font().family()).toString();
    const int savedFontSize = forceSimulatedDefaults ? 10 : settings.value(QStringLiteral("fontSize"), application.font().pointSize()).toInt();
    const bool savedAlternatingRows = forceSimulatedDefaults || settings.value(QStringLiteral("alternatingRows"), true).toBool();
    const QString savedBaseColor = forceSimulatedDefaults ? defaultBaseColor : settings.value(QStringLiteral("baseColor"), defaultBaseColor).toString();
    const QString savedAlternateColor = forceSimulatedDefaults ? defaultAlternateColor : settings.value(QStringLiteral("alternateColor"), defaultAlternateColor).toString();
    const QString savedTextColor = forceSimulatedDefaults ? QStringLiteral("#000000") : settings.value(QStringLiteral("textColor"), QStringLiteral("#000000")).toString();
    const QString savedHeaderColor = forceSimulatedDefaults ? defaultHeaderColor : settings.value(QStringLiteral("headerColor"), defaultHeaderColor).toString();
    const QString savedWidgetStyle = forceSimulatedDefaults ? defaultWidgetStyle : settings.value(QStringLiteral("widgetStyle"), defaultWidgetStyle).toString();
    InventoryWindow window(savedPath);
    window.setAlternatingRows(savedAlternatingRows);
    window.setSpreadsheetColors(savedBaseColor, savedAlternateColor, savedTextColor, savedHeaderColor);
    auto* configMenu = window.menuBar()->addMenu(QStringLiteral("Config"));
    auto* themeSettingsAction = configMenu->addAction(QStringLiteral("Theme settings..."));
    QObject::connect(themeSettingsAction, &QAction::triggered,
        [&application, &settings, &window, windowsPlatform](bool) {
        ThemeSettingsDialog dialog(windowsPlatform, &window);
        QObject::connect(&dialog, &ThemeSettingsDialog::settingsChanged,
            [&application, &settings, &window, windowsPlatform](const QString& theme, const QString& fontFamily,
                int fontSize, bool alternatingRows, const QString& baseColor,
                const QString& alternateColor, const QString& textColor,
                const QString& headerColor, const QString& widgetStyle) {
                applyTheme(application, theme, windowsPlatform);
                applyWidgetStyle(application, widgetStyle);
                QFont font(fontFamily, fontSize);
                application.setFont(font);
                window.setAlternatingRows(alternatingRows);
                window.setSpreadsheetColors(baseColor, alternateColor, textColor, headerColor);
                settings.setValue(QStringLiteral("theme"), theme);
                settings.setValue(QStringLiteral("fontFamily"), fontFamily);
                settings.setValue(QStringLiteral("fontSize"), fontSize);
                settings.setValue(QStringLiteral("alternatingRows"), alternatingRows);
                settings.setValue(QStringLiteral("baseColor"), baseColor);
                settings.setValue(QStringLiteral("alternateColor"), alternateColor);
                settings.setValue(QStringLiteral("textColor"), textColor);
                settings.setValue(QStringLiteral("headerColor"), headerColor);
                settings.setValue(QStringLiteral("widgetStyle"), widgetStyle);
            });
        dialog.exec();
    });
    configMenu->addSeparator();
    auto* chooseDirectoryAction = configMenu->addAction(QStringLiteral("Choose inventory directory..."));
    QObject::connect(chooseDirectoryAction, &QAction::triggered, [&window, &settings]() {
        const QString selectedDirectory = QFileDialog::getExistingDirectory(&window,
            QStringLiteral("Choose inventory directory"), QFileInfo(window.dataPath()).absolutePath());
        if (selectedDirectory.isEmpty()) {
            return;
        }
        const QString selectedFile = QDir(selectedDirectory).filePath(QStringLiteral("inventory.json"));
        settings.setValue(QStringLiteral("inventoryPath"), selectedFile);
        window.setDataPath(selectedFile);
    });
    applyTheme(application, savedTheme, windowsPlatform);
    applyWidgetStyle(application, savedWidgetStyle);
    application.setFont(QFont(savedFont, savedFontSize));
    window.show();
    return application.exec();
}
