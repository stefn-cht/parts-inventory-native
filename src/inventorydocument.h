#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class InventoryDocument {
public:
    bool load(const QString& filePath, QString* errorMessage = nullptr);
    bool save(const QString& filePath, QString* errorMessage = nullptr) const;

    QJsonArray fields() const;
    QJsonArray parts() const;
    void setFields(const QJsonArray& fields);
    void setParts(const QJsonArray& parts);

    QJsonObject toJson() const;
    static InventoryDocument fromJson(const QJsonObject& object, QString* errorMessage = nullptr);

private:
    QJsonArray fields_;
    QJsonArray parts_;
};
