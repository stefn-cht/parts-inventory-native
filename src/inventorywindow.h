#pragma once

#include "inventorydocument.h"

#include <QMainWindow>

class QLabel;
class QLineEdit;
class QTableWidget;

class InventoryWindow final : public QMainWindow {
public:
    explicit InventoryWindow(const QString& dataPath, QWidget* parent = nullptr);
    void setDataPath(const QString& dataPath);
    QString dataPath() const;
    void setAlternatingRows(bool enabled);
    void setSpreadsheetColors(const QString& baseColor, const QString& alternateColor,
        const QString& textColor, const QString& headerColor);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void loadInventory();
    void saveInventory();
    void refreshTable();
    void addPart();
    void editSelectedPart();
    void deleteSelectedPart();
    void activateTableCell(int row, int column);
    void setStatus(const QString& message);

    QString dataPath_;
    InventoryDocument document_;
    QTableWidget* partsTable_ = nullptr;
    QLineEdit* searchEdit_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QVector<int> displayedPartIndices_;
};
