#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QColorDialog>
#include <QComboBox>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QMap>
#include <QMessageBox>
#include <QTimer>
#include <QTreeWidgetItem>

#include <algorithm>
#include <vector>

#include "handler.h"
#include "jsonhelper.h"
#include "sha1.h"
#include "viviz.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void ItemTriggered(Qt::CheckState checkstate, bool tristate, QString name, QString group, bool searched = false);
    void SetTriggered(QComboBox *box, QDoubleSpinBox *spinbox, QString group, bool searched = false);
    void GetSetValue(QComboBox *box, QDoubleSpinBox *input, QDoubleSpinBox *output, QString group, bool searched = false);

    void SetupSearch();

    QJsonDocument GetJsonDoc(QString filename);

    QByteArray hexstr2bytes(const QString &str);

private slots:
    void refresh();

    void updateValues();

    void on_creatorListWidget_itemChanged(QListWidgetItem *item);

    void on_playerListWidget_itemChanged(QListWidgetItem *item);

    void on_bypassListWidget_itemChanged(QListWidgetItem *item);

    void on_versionCheckPushButton_clicked();

    void on_refreshPushButton_clicked();

    void on_dllInjectPushButton_clicked();

    void on_moduleInfoPushButton_clicked();

    void on_uApplyPushButton_clicked();

    void on_fpsBypassPushButton_clicked();

    void on_fpsBypassRecommendPushButton_clicked();

    void on_dsrInjectPushButton_clicked();

    void on_playerSetValuePushButton_clicked();

    void on_iconRandomiserPushButton_clicked();

    void on_creatorSetValuePushButton_clicked();

    void on_playerColourToolButton_clicked();

    void on_lvlPassPushButton_clicked();

    void on_globalListWidget_itemChanged(QListWidgetItem *item);

    void on_searchLineEdit_textChanged(const QString &arg1);

    void on_searchTreeWidget_itemChanged(QTreeWidgetItem *item, int column);

    void on_multiscalePushButton_clicked();

    void on_editorColourToolButton_clicked();

    void on_anticheatPushButton_clicked();

    void on_accountUnlinkerPushButton_clicked();

    void on_transitionPushButton_clicked();

    void on_iconStealerPushButton_clicked();

    void on_rateLevelPushButton_clicked();

    void on_setOpacityPushButton_clicked();

    void on_labelTextPushButton_clicked();

    void on_searchSetValuePushButton_clicked();

    void on_gamemodePushButton_clicked();

private:
    Ui::MainWindow *ui;

    Handler gmd;
    Viviz sym_mng;

    QMap<QString, std::vector<Hack>> hacks;
    QMap<QString, std::vector<SetValue>> sets;

    QString dir;
};

#endif // MAINWINDOW_H
