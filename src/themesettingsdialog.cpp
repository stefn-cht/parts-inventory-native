#include "themesettingsdialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QPushButton>
#include <QSpinBox>

ThemeSettingsDialog::ThemeSettingsDialog(bool windowsDefaults, QWidget* parent)
                : QDialog(parent), windowsDefaults_(windowsDefaults),
                    baseColor_(windowsDefaults ? QStringLiteral("#f0f0f0") : QStringLiteral("#83cceb")),
                    alternateColor_(windowsDefaults ? QStringLiteral("#d9d9d9") : QStringLiteral("#c0e6f5")),
                    textColor_(QStringLiteral("#000000")),
                    headerColor_(windowsDefaults ? QStringLiteral("#b0b0b0") : QStringLiteral("#4b9fc4")) {
    setWindowTitle(QStringLiteral("Theme Settings"));
    setModal(true);
    resize(420, 260);

    auto* layout = new QFormLayout(this);
    themeSelector_ = new QComboBox(this);
    themeSelector_->addItems({QStringLiteral("Classic XP"), QStringLiteral("Fusion Light"),
        QStringLiteral("Dark"), QStringLiteral("KDE Plasma"), QStringLiteral("Frost")});
    themeSelector_->setCurrentText(windowsDefaults_ ? QStringLiteral("Classic XP") : QStringLiteral("Dark"));
    widgetStyleSelector_ = new QComboBox(this);
    widgetStyleSelector_->addItems({QStringLiteral("Windows"), QStringLiteral("Fusion")});
    widgetStyleSelector_->setCurrentText(windowsDefaults_ ? QStringLiteral("Windows") : QStringLiteral("Fusion"));
    fontSelector_ = new QComboBox(this);
    fontSelector_->addItems(QFontDatabase::families());
    fontSelector_->setCurrentText(font().family());
    fontSizeSelector_ = new QSpinBox(this);
    fontSizeSelector_->setRange(8, 32);
    fontSizeSelector_->setValue(font().pointSize() > 0 ? font().pointSize() : 10);
    alternatingRows_ = new QCheckBox(QStringLiteral("Use alternating spreadsheet row colours"), this);
    alternatingRows_->setChecked(true);

    auto* baseColorButton = new QPushButton(QStringLiteral("Choose entry colour"), this);
    auto* alternateColorButton = new QPushButton(QStringLiteral("Choose alternating row colour"), this);
    auto* textColorButton = new QPushButton(QStringLiteral("Choose spreadsheet text colour"), this);
    auto* headerColorButton = new QPushButton(QStringLiteral("Choose spreadsheet header colour"), this);
    layout->addRow(QStringLiteral("Base theme:"), themeSelector_);
    layout->addRow(QStringLiteral("Widget style:"), widgetStyleSelector_);
    layout->addRow(QStringLiteral("Application font:"), fontSelector_);
    layout->addRow(QStringLiteral("Text size:"), fontSizeSelector_);
    layout->addRow(alternatingRows_);
    layout->addRow(baseColorButton);
    layout->addRow(alternateColorButton);
    layout->addRow(textColorButton);
    layout->addRow(headerColorButton);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close, this);
    auto* restoreDefaultsButton = new QPushButton(QStringLiteral("Restore defaults"), this);
    layout->addRow(buttons);
    layout->addRow(restoreDefaultsButton);
    connect(baseColorButton, &QPushButton::clicked, this, &ThemeSettingsDialog::chooseBaseColor);
    connect(alternateColorButton, &QPushButton::clicked, this, &ThemeSettingsDialog::chooseAlternateColor);
    connect(textColorButton, &QPushButton::clicked, this, &ThemeSettingsDialog::chooseTextColor);
    connect(headerColorButton, &QPushButton::clicked, this, &ThemeSettingsDialog::chooseHeaderColor);
    connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked,
        this, &ThemeSettingsDialog::emitSettings);
    connect(buttons->button(QDialogButtonBox::Close), &QPushButton::clicked, this, &QDialog::reject);
    connect(restoreDefaultsButton, &QPushButton::clicked, this, &ThemeSettingsDialog::restoreDefaults);
}

void ThemeSettingsDialog::chooseBaseColor() {
    const QColor color = QColorDialog::getColor(QColor(baseColor_), this, QStringLiteral("Entry colour"));
    if (color.isValid()) {
        baseColor_ = color.name();
    }
}

void ThemeSettingsDialog::chooseAlternateColor() {
    const QColor color = QColorDialog::getColor(QColor(alternateColor_), this,
        QStringLiteral("Alternating row colour"));
    if (color.isValid()) {
        alternateColor_ = color.name();
    }
}

void ThemeSettingsDialog::chooseTextColor() {
    const QColor color = QColorDialog::getColor(QColor(textColor_), this,
        QStringLiteral("Spreadsheet text colour"));
    if (color.isValid()) {
        textColor_ = color.name();
    }
}

void ThemeSettingsDialog::chooseHeaderColor() {
    const QColor color = QColorDialog::getColor(QColor(headerColor_), this,
        QStringLiteral("Spreadsheet header colour"));
    if (color.isValid()) {
        headerColor_ = color.name();
    }
}

void ThemeSettingsDialog::restoreDefaults() {
    if (windowsDefaults_) {
    themeSelector_->setCurrentText(QStringLiteral("Classic XP"));
    widgetStyleSelector_->setCurrentText(QStringLiteral("Windows"));
    baseColor_ = QStringLiteral("#f0f0f0");
    alternateColor_ = QStringLiteral("#d9d9d9");
    headerColor_ = QStringLiteral("#b0b0b0");
    } else {
    themeSelector_->setCurrentText(QStringLiteral("Dark"));
    widgetStyleSelector_->setCurrentText(QStringLiteral("Fusion"));
    baseColor_ = QStringLiteral("#83cceb");
    alternateColor_ = QStringLiteral("#c0e6f5");
    headerColor_ = QStringLiteral("#4b9fc4");
    }
    fontSelector_->setCurrentText(font().family());
    fontSizeSelector_->setValue(10);
    alternatingRows_->setChecked(true);
    textColor_ = QStringLiteral("#000000");
}

void ThemeSettingsDialog::emitSettings() {
    emit settingsChanged(themeSelector_->currentText(), fontSelector_->currentText(),
        fontSizeSelector_->value(), alternatingRows_->isChecked(), baseColor_, alternateColor_,
        textColor_, headerColor_, widgetStyleSelector_->currentText());
}
