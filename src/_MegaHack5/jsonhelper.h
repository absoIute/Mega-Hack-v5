#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QtCore>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QComboBox>
#include <QListWidget>
#include <QListWidgetItem>

struct Opcode
{
    QString addr, lib, on, off, tri;
};

struct Hack
{
    QString name, desc;
    bool tristate;
    std::vector<Opcode> opcodes;
};

struct Pointer
{
    QString type, lib;
    std::vector<uint32_t> offsets;
    double value_offset;
    bool reset;
};

struct SetValue
{
    QString name;
    bool isref;
    std::vector<Pointer> pointers;
};

namespace JsonHelper
{
    QJsonDocument GetJsonDoc(QString filename);

    bool SetupListHacks(QString file, QString name, QListWidget *parent, QMap<QString, std::vector<Hack>> &map);

    bool SetupSets(QString file, QString name, QComboBox *box, QMap<QString, std::vector<SetValue>> &map);
}

#endif // JSONHELPER_H
