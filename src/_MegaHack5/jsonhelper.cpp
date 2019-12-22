#include "jsonhelper.h"

namespace JsonHelper
{
    QJsonDocument GetJsonDoc(QString filename)
    {
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray data = file.readAll();
            file.close();
            return QJsonDocument::fromJson(data);
        }
        return QJsonDocument();
    }

    bool SetupListHacks(QString file, QString name, QListWidget *parent, QMap<QString, std::vector<Hack>> &map)
    {
        parent->clear();

        std::vector<Hack> hacks;
        QJsonDocument json(GetJsonDoc(file));

        if (json.isEmpty())
            return false;

        QJsonArray hack_arr = json.object().value("hacks").toArray();

        for (const auto &hack_val : hack_arr)
        {
            QJsonObject hack_obj(hack_val.toObject());
            Hack hack;
            hack.name = hack_obj.value("name").toString();
            hack.desc = hack_obj.value("desc").toString();
            hack.tristate = hack_obj.value("tristate").toBool();

            QListWidgetItem *item = new QListWidgetItem(hack.name);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable | (hack.tristate ? Qt::ItemIsUserTristate : Qt::ItemFlags()));
            item->setCheckState(Qt::Unchecked);
            parent->addItem(item);

            for (const auto &opcode : hack_obj.value("opcodes").toArray())
            {
                QJsonObject opcode_obj = opcode.toObject();
                Opcode oc;
                oc.addr = opcode_obj["addr"].toString();
                oc.on = opcode_obj["on"].toString();
                oc.off = opcode_obj["off"].toString();
                oc.tri = opcode_obj["tri"].toString();
                oc.lib = opcode_obj["lib"].toString("SpaceDash.exe");

                hack.opcodes.push_back(oc);
            }
            hacks.push_back(hack);
        }
        parent->sortItems();
        map.insert(name, hacks);
        return true;
    }

    bool SetupSets(QString file, QString name, QComboBox *box, QMap<QString, std::vector<SetValue>> &map)
    {
        box->clear();

        std::vector<SetValue> sets;
        QJsonDocument json(GetJsonDoc(file));
        QJsonArray set_arr = json.object().value("sets").toArray();

        for (const auto &set : set_arr)
        {
            QJsonObject set_obj = set.toObject();
            SetValue sv;
            sv.name = set_obj.value("name").toString();
            sv.isref = set_obj.value("isref").toBool();
            box->addItem(sv.name);

            for (const auto &ptr_val : set_obj.value("pointers").toArray())
            {
                QJsonObject ptr_obj = ptr_val.toObject();

                Pointer ptr;
                ptr.lib = ptr_obj.value("lib").toString("SpaceDash.exe");
                ptr.type = ptr_obj.value("type").toString();
                ptr.reset = ptr_obj.value("reset").toBool();
                ptr.value_offset = ptr_obj.value("offset").toString("0").toDouble();

                for (const auto &str: ptr_obj["offsets"].toString().split(';'))
                    ptr.offsets.push_back(str.toInt(nullptr, 0));

                sv.pointers.push_back(ptr);
            }
            sets.push_back(sv);
        }
        box->model()->sort(0);
        box->setCurrentIndex(0);
        map.insert(name, sets);
        return true;
    }
}
