#include "inventorywindow.h"

#include <QDir>
#include <QDateTime>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QJsonArray>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

namespace {

bool fieldIsVisible(const QJsonObject& field) {
    return !field.contains(QStringLiteral("visible")) || field.value(QStringLiteral("visible")).toBool();
}

QString fieldType(const QJsonObject& field) {
    return field.value(QStringLiteral("type")).toString(QStringLiteral("text"));
}

void showDescriptionDialog(QWidget* parent, const QString& description) {
    QDialog dialog(parent);
    dialog.setWindowTitle(QStringLiteral("Description"));
    dialog.resize(560, 360);
    auto* layout = new QVBoxLayout(&dialog);
    auto* editor = new QPlainTextEdit(&dialog);
    editor->setPlainText(description);
    editor->setReadOnly(true);
    auto* closeButton = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    layout->addWidget(editor);
    layout->addWidget(closeButton);
    QObject::connect(closeButton, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}

QJsonObject showPartDialog(QWidget* parent, const QJsonArray& fields,
    const QJsonObject& initialPart, bool editing, bool* accepted) {
    QDialog dialog(parent);
    dialog.setWindowTitle(editing ? QStringLiteral("Edit Part") : QStringLiteral("Add Part"));
    auto* formLayout = new QFormLayout(&dialog);
    QHash<QString, QWidget*> editors;

    for (const QJsonValue& fieldValue : fields) {
        const QJsonObject field = fieldValue.toObject();
        if (!fieldIsVisible(field)) {
            continue;
        }
        const QString fieldId = field.value(QStringLiteral("id")).toString();
        if (fieldId.isEmpty()) {
            continue;
        }
        const QString label = field.value(QStringLiteral("label")).toString(fieldId)
            + (field.value(QStringLiteral("required")).toBool() ? QStringLiteral(" *") : QString());
        const QJsonValue initialValue = initialPart.value(fieldId);
        QWidget* editor = nullptr;

        if (fieldType(field) == QStringLiteral("textarea")) {
            auto* textEditor = new QPlainTextEdit(&dialog);
            textEditor->setPlainText(initialValue.toString());
            editor = textEditor;
        } else if (fieldType(field) == QStringLiteral("number")) {
            auto* numberEditor = new QSpinBox(&dialog);
            if (fieldId == QStringLiteral("drawerBoxCell")) {
                numberEditor->setRange(0, 32);
            } else {
                numberEditor->setRange(0, 1000000000);
            }
            numberEditor->setValue(initialValue.toInt());
            editor = numberEditor;
        } else if (fieldType(field) == QStringLiteral("checkbox")) {
            auto* checkboxEditor = new QCheckBox(&dialog);
            checkboxEditor->setChecked(initialValue.toBool());
            editor = checkboxEditor;
        } else if (fieldType(field) == QStringLiteral("date")) {
            auto* dateEditor = new QDateEdit(&dialog);
            dateEditor->setCalendarPopup(true);
            const QDate initialDate = QDate::fromString(initialValue.toString(), Qt::ISODate);
            dateEditor->setDate(initialDate.isValid() ? initialDate : QDate::currentDate());
            editor = dateEditor;
        } else if (fieldType(field) == QStringLiteral("select") && field.value(QStringLiteral("options")).isArray()) {
            auto* selectEditor = new QComboBox(&dialog);
            for (const QJsonValue& option : field.value(QStringLiteral("options")).toArray()) {
                selectEditor->addItem(option.toString());
            }
            selectEditor->setCurrentText(initialValue.toString());
            editor = selectEditor;
        } else {
            auto* textEditor = new QLineEdit(initialValue.toString(), &dialog);
            editor = textEditor;
        }

        editors.insert(fieldId, editor);
        formLayout->addRow(label, editor);
    }

    auto* drawerCheckbox = qobject_cast<QCheckBox*>(editors.value(QStringLiteral("isInDrawerBox")));
    auto* drawerCellEditor = editors.value(QStringLiteral("drawerBoxCell"));
    if (drawerCheckbox && drawerCellEditor) {
        const auto updateDrawerCellState = [drawerCellEditor](bool checked) {
            drawerCellEditor->setEnabled(checked);
        };
        QObject::connect(drawerCheckbox, &QCheckBox::toggled, &dialog, updateDrawerCellState);
        updateDrawerCellState(drawerCheckbox->isChecked());
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    formLayout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        *accepted = false;
        return initialPart;
    }

    QJsonObject result = initialPart;
    for (const QJsonValue& fieldValue : fields) {
        const QJsonObject field = fieldValue.toObject();
        const QString fieldId = field.value(QStringLiteral("id")).toString();
        QWidget* editor = editors.value(fieldId);
        if (!editor) {
            continue;
        }
        const QString type = fieldType(field);
        if (type == QStringLiteral("textarea")) {
            result.insert(fieldId, qobject_cast<QPlainTextEdit*>(editor)->toPlainText());
        } else if (type == QStringLiteral("number")) {
            result.insert(fieldId, qobject_cast<QSpinBox*>(editor)->value());
        } else if (type == QStringLiteral("checkbox")) {
            result.insert(fieldId, qobject_cast<QCheckBox*>(editor)->isChecked());
        } else if (type == QStringLiteral("date")) {
            result.insert(fieldId, qobject_cast<QDateEdit*>(editor)->date().toString(Qt::ISODate));
        } else if (type == QStringLiteral("select") && qobject_cast<QComboBox*>(editor)) {
            result.insert(fieldId, qobject_cast<QComboBox*>(editor)->currentText());
        } else {
            result.insert(fieldId, qobject_cast<QLineEdit*>(editor)->text());
        }
    }

    for (const QJsonValue& fieldValue : fields) {
        const QJsonObject field = fieldValue.toObject();
        if (field.value(QStringLiteral("required")).toBool()) {
            const QString fieldId = field.value(QStringLiteral("id")).toString();
            if (result.value(fieldId).toString().trimmed().isEmpty()) {
                QMessageBox::warning(parent, QStringLiteral("Required field"),
                    QStringLiteral("%1 is required.").arg(field.value(QStringLiteral("label")).toString(fieldId)));
                *accepted = false;
                return initialPart;
            }
        }
    }

    *accepted = true;
    return result;
}

}

InventoryWindow::InventoryWindow(const QString& dataPath, QWidget* parent)
    : QMainWindow(parent), dataPath_(dataPath) {
    setWindowTitle(QStringLiteral("Parts Inventory"));
    resize(960, 640);

    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);
    auto* title = new QLabel(QStringLiteral("Parts Inventory"), centralWidget);
    partsTable_ = new QTableWidget(centralWidget);
    partsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    partsTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    partsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    partsTable_->installEventFilter(this);
    partsTable_->setAlternatingRowColors(true);
    partsTable_->horizontalHeader()->setStretchLastSection(true);

    auto* toolbar = new QToolBar(QStringLiteral("Inventory actions"), this);
    auto* addButton = new QPushButton(QStringLiteral("Add part"), toolbar);
    auto* editButton = new QPushButton(QStringLiteral("Edit selected"), toolbar);
    auto* deleteButton = new QPushButton(QStringLiteral("Delete selected"), toolbar);
    auto* saveButton = new QPushButton(QStringLiteral("Save"), toolbar);
    auto* reloadButton = new QPushButton(QStringLiteral("Reload"), toolbar);
    toolbar->addWidget(addButton);
    toolbar->addWidget(editButton);
    toolbar->addWidget(deleteButton);
    toolbar->addWidget(saveButton);
    toolbar->addWidget(reloadButton);
    searchEdit_ = new QLineEdit(toolbar);
    searchEdit_->setPlaceholderText(QStringLiteral("Search parts..."));
    searchEdit_->setClearButtonEnabled(true);
    toolbar->addWidget(searchEdit_);
    addToolBar(toolbar);

    layout->addWidget(title);
    layout->addWidget(partsTable_);
    setCentralWidget(centralWidget);
    statusLabel_ = new QLabel(this);
    statusBar()->addPermanentWidget(statusLabel_);

    connect(addButton, &QPushButton::clicked, this, &InventoryWindow::addPart);
    connect(editButton, &QPushButton::clicked, this, &InventoryWindow::editSelectedPart);
    connect(deleteButton, &QPushButton::clicked, this, &InventoryWindow::deleteSelectedPart);
    connect(saveButton, &QPushButton::clicked, this, &InventoryWindow::saveInventory);
    connect(reloadButton, &QPushButton::clicked, this, &InventoryWindow::loadInventory);
    connect(searchEdit_, &QLineEdit::textChanged, this, &InventoryWindow::refreshTable);
    connect(partsTable_, &QTableWidget::cellDoubleClicked, this,
        [this](int row, int column) { activateTableCell(row, column); });
    loadInventory();
}

bool InventoryWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == partsTable_ && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Space || keyEvent->key() == Qt::Key_Return
            || keyEvent->key() == Qt::Key_Enter) {
            activateTableCell(partsTable_->currentRow(), partsTable_->currentColumn());
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void InventoryWindow::activateTableCell(int row, int column) {
    if (row < 0 || row >= displayedPartIndices_.size()) {
        return;
    }
    const QJsonArray fields = document_.fields();
    if (column >= 0 && column < fields.size()
        && fields.at(column).toObject().value(QStringLiteral("id")).toString()
            == QStringLiteral("description")) {
        const int partIndex = displayedPartIndices_.at(row);
        showDescriptionDialog(this,
            document_.parts().at(partIndex).toObject().value(QStringLiteral("description")).toString());
        return;
    }
    editSelectedPart();
}

void InventoryWindow::setDataPath(const QString& dataPath) {
    if (dataPath.isEmpty() || dataPath == dataPath_) {
        return;
    }
    dataPath_ = dataPath;
    loadInventory();
}

QString InventoryWindow::dataPath() const {
    return dataPath_;
}

void InventoryWindow::setAlternatingRows(bool enabled) {
    partsTable_->setAlternatingRowColors(enabled);
}

void InventoryWindow::setSpreadsheetColors(const QString& baseColor, const QString& alternateColor,
    const QString& textColor, const QString& headerColor) {
    QPalette tablePalette = partsTable_->palette();
    tablePalette.setColor(QPalette::Base, QColor(baseColor));
    tablePalette.setColor(QPalette::AlternateBase, QColor(alternateColor));
    tablePalette.setColor(QPalette::Text, QColor(textColor));
    tablePalette.setColor(QPalette::WindowText, QColor(textColor));
    tablePalette.setColor(QPalette::Button, QColor(headerColor));
    tablePalette.setColor(QPalette::ButtonText, QColor(textColor));
    partsTable_->setPalette(tablePalette);
    partsTable_->horizontalHeader()->setStyleSheet(
        QStringLiteral("QHeaderView::section { background-color: %1; color: %2; }")
            .arg(headerColor, textColor));
}

void InventoryWindow::loadInventory() {
    QString errorMessage;
    if (!document_.load(dataPath_, &errorMessage)) {
        setStatus(errorMessage);
        return;
    }
    if (document_.fields().isEmpty()) {
        document_.setFields({
            QJsonObject{{QStringLiteral("id"), QStringLiteral("partName")},
                        {QStringLiteral("label"), QStringLiteral("Part Name")},
                        {QStringLiteral("type"), QStringLiteral("text")},
                        {QStringLiteral("visible"), true},
                        {QStringLiteral("required"), true}},
            QJsonObject{{QStringLiteral("id"), QStringLiteral("location")},
                        {QStringLiteral("label"), QStringLiteral("Location")},
                        {QStringLiteral("type"), QStringLiteral("text")},
                        {QStringLiteral("visible"), true},
                        {QStringLiteral("required"), false}},
            QJsonObject{{QStringLiteral("id"), QStringLiteral("quantity")},
                        {QStringLiteral("label"), QStringLiteral("Quantity")},
                        {QStringLiteral("type"), QStringLiteral("number")},
                        {QStringLiteral("visible"), true},
                        {QStringLiteral("required"), false}},
            QJsonObject{{QStringLiteral("id"), QStringLiteral("description")},
                        {QStringLiteral("label"), QStringLiteral("Description")},
                        {QStringLiteral("type"), QStringLiteral("textarea")},
                        {QStringLiteral("visible"), true},
                        {QStringLiteral("required"), false}},
            QJsonObject{{QStringLiteral("id"), QStringLiteral("isInDrawerBox")},
                        {QStringLiteral("label"), QStringLiteral("Is in drawer box")},
                        {QStringLiteral("type"), QStringLiteral("checkbox")},
                        {QStringLiteral("visible"), true},
                        {QStringLiteral("required"), false}},
            QJsonObject{{QStringLiteral("id"), QStringLiteral("drawerBoxCell")},
                        {QStringLiteral("label"), QStringLiteral("Drawer Box Cell (0-32)")},
                        {QStringLiteral("type"), QStringLiteral("number")},
                        {QStringLiteral("visible"), true},
                        {QStringLiteral("required"), false}}
        });
    }
    refreshTable();
    setStatus(QStringLiteral("Loaded %1 part(s) from %2").arg(document_.parts().size()).arg(dataPath_));
}

void InventoryWindow::saveInventory() {
    QDir().mkpath(QFileInfo(dataPath_).absolutePath());
    QString errorMessage;
    if (!document_.save(dataPath_, &errorMessage)) {
        setStatus(errorMessage);
        return;
    }
    setStatus(QStringLiteral("Saved inventory to %1").arg(dataPath_));
}

void InventoryWindow::refreshTable() {
    const QJsonArray fields = document_.fields();
    const QJsonArray parts = document_.parts();
    partsTable_->clear();
    partsTable_->setColumnCount(fields.size());
    displayedPartIndices_.clear();
    const QString searchTerm = searchEdit_ ? searchEdit_->text().trimmed() : QString();
    for (int partIndex = 0; partIndex < parts.size(); ++partIndex) {
        const QJsonObject part = parts.at(partIndex).toObject();
        bool matches = searchTerm.isEmpty();
        if (!matches) {
            for (const QJsonValue& value : part) {
                if (value.toVariant().toString().contains(searchTerm, Qt::CaseInsensitive)) {
                    matches = true;
                    break;
                }
            }
        }
        if (matches) {
            displayedPartIndices_.append(partIndex);
        }
    }
    partsTable_->setRowCount(displayedPartIndices_.size());

    QStringList headers;
    for (const QJsonValue& fieldValue : fields) {
        headers.append(fieldValue.toObject().value(QStringLiteral("label")).toString(
            fieldValue.toObject().value(QStringLiteral("id")).toString()));
    }
    partsTable_->setHorizontalHeaderLabels(headers);

    for (int row = 0; row < displayedPartIndices_.size(); ++row) {
        const QJsonObject part = parts.at(displayedPartIndices_.at(row)).toObject();
        for (int column = 0; column < fields.size(); ++column) {
            const QString fieldId = fields.at(column).toObject().value(QStringLiteral("id")).toString();
            const QJsonValue value = part.value(fieldId);
            QString displayValue;
            if (value.isBool()) {
                displayValue = value.toBool() ? QStringLiteral("Yes") : QStringLiteral("No");
            } else if (!value.isUndefined() && !value.isNull()) {
                displayValue = value.toVariant().toString();
            }
            if (fieldId == QStringLiteral("description") && displayValue.size() > 25) {
                displayValue = displayValue.left(22) + QStringLiteral("...");
            }
            partsTable_->setItem(row, column, new QTableWidgetItem(displayValue));
        }
    }
    partsTable_->resizeColumnsToContents();
    for (int column = 0; column < fields.size(); ++column) {
        if (fields.at(column).toObject().value(QStringLiteral("id")).toString()
            == QStringLiteral("description")) {
            partsTable_->setColumnWidth(column, 220);
        }
    }
}

void InventoryWindow::addPart() {
    bool accepted = false;
    QJsonObject part = showPartDialog(this, document_.fields(),
        {{QStringLiteral("id"), QString::number(QDateTime::currentMSecsSinceEpoch())}}, false, &accepted);
    if (!accepted) {
        return;
    }

    QJsonArray parts = document_.parts();
    parts.append(part);
    document_.setParts(parts);
    refreshTable();
    saveInventory();
}

void InventoryWindow::editSelectedPart() {
    const int row = partsTable_->currentRow();
    if (row < 0 || row >= displayedPartIndices_.size()) {
        setStatus(QStringLiteral("Select a part first."));
        return;
    }
    QJsonArray parts = document_.parts();
    const int partIndex = displayedPartIndices_.at(row);
    bool accepted = false;
    const QJsonObject editedPart = showPartDialog(this, document_.fields(),
        parts.at(partIndex).toObject(), true, &accepted);
    if (!accepted) {
        return;
    }
    parts.replace(partIndex, editedPart);
    document_.setParts(parts);
    refreshTable();
    saveInventory();
}

void InventoryWindow::deleteSelectedPart() {
    const int row = partsTable_->currentRow();
    if (row < 0) {
        setStatus(QStringLiteral("Select a part first."));
        return;
    }
    QJsonArray parts = document_.parts();
    parts.removeAt(displayedPartIndices_.at(row));
    document_.setParts(parts);
    refreshTable();
    saveInventory();
}

void InventoryWindow::setStatus(const QString& message) {
    statusLabel_->setText(message);
}
