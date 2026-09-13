#include "inventorydocument.h"

#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>

bool InventoryDocument::load(const QString& filePath, QString* errorMessage) {
    QFile inputFile(filePath);
    if (!inputFile.exists()) {
        fields_ = QJsonArray();
        parts_ = QJsonArray();
        return true;
    }

    if (!inputFile.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Could not open inventory file: %1").arg(inputFile.errorString());
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(inputFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid inventory JSON: %1").arg(parseError.errorString());
        }
        return false;
    }

    *this = fromJson(document.object(), errorMessage);
    return errorMessage == nullptr || errorMessage->isEmpty();
}

bool InventoryDocument::save(const QString& filePath, QString* errorMessage) const {
    QSaveFile outputFile(filePath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Could not write inventory file: %1").arg(outputFile.errorString());
        }
        return false;
    }

    const QByteArray content = QJsonDocument(toJson()).toJson(QJsonDocument::Indented);
    if (outputFile.write(content) != content.size() || !outputFile.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Could not commit inventory file: %1").arg(outputFile.errorString());
        }
        return false;
    }
    return true;
}

QJsonArray InventoryDocument::fields() const {
    return fields_;
}

QJsonArray InventoryDocument::parts() const {
    return parts_;
}

void InventoryDocument::setFields(const QJsonArray& fields) {
    fields_ = fields;
}

void InventoryDocument::setParts(const QJsonArray& parts) {
    parts_ = parts;
}

QJsonObject InventoryDocument::toJson() const {
    return {
        {QStringLiteral("parts"), parts_},
        {QStringLiteral("fields"), fields_}
    };
}

InventoryDocument InventoryDocument::fromJson(const QJsonObject& object, QString* errorMessage) {
    InventoryDocument document;
    const QJsonValue fieldsValue = object.value(QStringLiteral("fields"));
    const QJsonValue partsValue = object.value(QStringLiteral("parts"));

    if (!fieldsValue.isUndefined() && !fieldsValue.isArray()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Inventory fields must be an array.");
        }
        return document;
    }
    if (!partsValue.isUndefined() && !partsValue.isArray()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Inventory parts must be an array.");
        }
        return document;
    }

    document.fields_ = fieldsValue.isArray() ? fieldsValue.toArray() : QJsonArray();
    document.parts_ = partsValue.isArray() ? partsValue.toArray() : QJsonArray();
    return document;
}
