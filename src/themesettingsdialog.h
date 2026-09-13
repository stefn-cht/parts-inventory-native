#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QPushButton;
class QSpinBox;

class ThemeSettingsDialog final : public QDialog {
    Q_OBJECT

public:
    explicit ThemeSettingsDialog(bool windowsDefaults, QWidget* parent = nullptr);

signals:
    void settingsChanged(const QString& theme, const QString& fontFamily, int fontSize,
        bool alternatingRows, const QString& baseColor, const QString& alternateColor,
        const QString& textColor, const QString& headerColor, const QString& widgetStyle);

private:
    void chooseBaseColor();
    void chooseAlternateColor();
    void chooseTextColor();
    void chooseHeaderColor();
    void restoreDefaults();
    void emitSettings();

    QComboBox* themeSelector_ = nullptr;
    QComboBox* widgetStyleSelector_ = nullptr;
    QComboBox* fontSelector_ = nullptr;
    QSpinBox* fontSizeSelector_ = nullptr;
    QCheckBox* alternatingRows_ = nullptr;
    QString baseColor_;
    QString alternateColor_;
    QString textColor_;
    QString headerColor_;
    bool windowsDefaults_ = false;
};
