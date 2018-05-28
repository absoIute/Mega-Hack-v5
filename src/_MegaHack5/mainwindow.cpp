#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#define UNRANDOMISER_DLL "cosmiclove.dll"
#define FRAME_BYPASS_DLL "hotmilk.dll"
#define DEATH_SOUNDS_DLL "whitemagic.dll"
#define RANDOM_ICONS_DLL "sorryforlovingyou.dll"
#define SPEEEED_HACK_DLL "futurecandy.dll"

#define GAME_NAME "GeometryDash.exe"

#define GLOBAL_STR "Global"
#define BYPASS_STR "Bypass"
#define PLAYER_STR "Player"
#define CREATOR_STR "Creator"

#define OFFSET_SCALE_X	0x28
#define OFFSET_SCALE_Y	0x2C
#define OFFSET_X		0x34
#define OFFSET_Y		0x38
#define OFFSET_SCALE	0x35C
#define OFFSET_SELECTED	0x3DA

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);
    this->ui->tabWidget->setCurrentIndex(0);

    QPixmap pixmap = QPixmap(":/icon_small.png").scaled(this->ui->bannerLabel->size(), Qt::KeepAspectRatio);
    this->ui->bannerLabel->setPixmap(pixmap);
    this->ui->bannerLabel->setFixedSize(pixmap.size());

    this->ui->uTypeComboBox->addItems({"Add", "Sub"});

    this->ui->searchTreeWidget->setHeaderLabels({"Name", "Description"});

    this->dir = QDir::currentPath().replace('/', '\\');

    QDesktopServices::openUrl(QUrl("https://nettik.co.uk/_/r.php?v"));

    QTimer::singleShot(0, this, SLOT(refresh()));

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateValues()));
    timer->start(100);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QJsonDocument MainWindow::GetJsonDoc(QString filename)
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

QByteArray MainWindow::hexstr2bytes(const QString& str)
{
    return QByteArray::fromHex(str.toUtf8());
}

void MainWindow::refresh()
{
    this->hacks.clear();
    this->ui->searchTreeWidget->clear();

    JsonHelper::SetupListHacks(tr("%0\\hacks\\global.json").arg(this->dir), GLOBAL_STR, this->ui->globalListWidget, this->hacks);
    JsonHelper::SetupListHacks(tr("%0\\hacks\\creator.json").arg(this->dir), CREATOR_STR, this->ui->creatorListWidget, this->hacks);
    JsonHelper::SetupListHacks(tr("%0\\hacks\\player.json").arg(this->dir), PLAYER_STR, this->ui->playerListWidget, this->hacks);
    JsonHelper::SetupListHacks(tr("%0\\hacks\\bypass.json").arg(this->dir), BYPASS_STR, this->ui->bypassListWidget, this->hacks);

    JsonHelper::SetupSets(tr("%0\\hacks\\player.json").arg(this->dir), PLAYER_STR, this->ui->playerValueComboBox, this->sets);
    JsonHelper::SetupSets(tr("%0\\hacks\\creator.json").arg(this->dir), CREATOR_STR, this->ui->creatorValueComboBox, this->sets);

    this->SetupSearch();

    if (!this->gmd.AttemptAttach("Geometry Dash", GAME_NAME))
        QMessageBox::warning(this, "Error", "Failed to bind to Geometry Dash.");
    else
        this->sym_mng.SetProcessId(this->gmd.GetPID());

    this->ui->pidLabel->setText(tr("PID:%0").arg(this->gmd.GetPID()));

    size_t hacks = 17; //count of hard-coded hacks
    for (auto it = this->hacks.begin(); it != this->hacks.end(); ++it) hacks += it->size();
    for (auto it = this->sets.begin(); it != this->sets.end(); ++it)
    {
        for (const auto &set : *it)
        {
            if (set.isref)
                this->gmd.MemNew(set.name.toStdString(), 4);
        }
        hacks += it->size();
    }
    this->gmd.MemNew("__opacity__", 4);

    this->ui->infoTextBrowser->setFontFamily("Courier New");
    this->ui->infoTextBrowser->setText(tr("Mega Hack v5 By Absolute (for 2.113).\n\nTotal features: %0.\n\n>> ABOUT <<\nPowered by Qt.\nGUI Icons by Yusuke Kamiyamane.\nSHA1 lib: https://github.com/vog/sha1/\nSymbol enumerator: https://github.com/absoIute/Viviz/\n\n>> SPECIAL THANKS <<\nAdafcaefc\nBodya\nCos8o\nItalian Apk Downloader\nmgostIH\nPavlukivan\nSkyvlan\nYoanndp").arg(hacks));
}

void MainWindow::SetupSearch()
{
    for (auto it = this->hacks.begin(); it != this->hacks.end(); ++it)
    {
        QTreeWidgetItem *tli = new QTreeWidgetItem();
        tli->setText(0, it.key());
        for (const auto &hack : it.value())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(tli);
            if (hack.tristate) item->setFlags(item->flags() | Qt::ItemIsUserTristate);
            item->setCheckState(0, Qt::Unchecked);
            item->setText(0, hack.name);
            item->setText(1, hack.desc);
        }
        this->ui->searchTreeWidget->addTopLevelItem(tli); //setting parent here to avoid signal
        tli->sortChildren(0, Qt::AscendingOrder);
    }
    this->ui->searchTreeWidget->expandAll();
    this->ui->searchTreeWidget->resizeColumnToContents(0);

    for (auto it = this->sets.begin(); it != this->sets.end(); ++it)
    {
        for (const auto &set : it.value())
            this->ui->searchValueComboBox->addItem(set.name);
    }
}

void MainWindow::updateValues()
{
    this->GetSetValue(this->ui->playerValueComboBox, this->ui->playerValueDoubleSpinBox, this->ui->playerCurrentValueDoubleSpinBox, PLAYER_STR);
    this->GetSetValue(this->ui->creatorValueComboBox, this->ui->creatorValueDoubleSpinBox, this->ui->creatorCurrentValueDoubleSpinBox, CREATOR_STR);
    this->GetSetValue(this->ui->searchValueComboBox, this->ui->searchValueDoubleSpinBox, this->ui->searchCurrentValueDoubleSpinBox, QString(), true);

}

void MainWindow::ItemTriggered(Qt::CheckState checkstate, bool tristate, QString name, QString group, bool searched)
{
    std::vector<Hack> h = this->hacks.value(group);
    std::vector<Hack>::iterator it = std::find_if(h.begin(), h.end(), [name](const Hack& hack) -> bool { return hack.name == name; });

    if (it != h.end())
    {
        for (const auto &opcode : it->opcodes)
        {
            QByteArray bytes;
            if (tristate && checkstate == Qt::PartiallyChecked)
                bytes = hexstr2bytes(opcode.tri);
            else
                bytes = hexstr2bytes(checkstate ? opcode.on : opcode.off);

            uint32_t addr = this->gmd.GetModuleBase(opcode.lib.toUtf8().data()) + opcode.addr.toUInt(nullptr, 0);
            uint32_t old = this->gmd.Protect(addr, bytes.size(), PAGE_EXECUTE_READWRITE);
            this->gmd.Write(addr, bytes.data(), bytes.size());
            this->gmd.Protect(addr, bytes.size(), old);
        }

        if (searched)
        {
            QListWidget *lw;
            if (group == GLOBAL_STR)
                lw = this->ui->globalListWidget;
            else if (group == BYPASS_STR)
                lw = this->ui->bypassListWidget;
            else if (group == CREATOR_STR)
                lw = this->ui->creatorListWidget;
            else if (group == PLAYER_STR)
                lw = this->ui->playerListWidget;
            else
                return;
            lw->findItems(name, Qt::MatchFixedString).first()->setCheckState(checkstate);
        }
        else
        {
            for (const auto &item : this->ui->searchTreeWidget->findItems(name, Qt::MatchFixedString | Qt::MatchRecursive))
            {
                if (item->parent())
                    item->setCheckState(0, checkstate);
            }
        }
    }
}

void MainWindow::SetTriggered(QComboBox *box, QDoubleSpinBox *spinbox, QString group, bool searched)
{
    std::vector<SetValue> v;

    if (searched)
    {
        for (const auto &setv : this->sets)
            v.insert(v.end(), setv.begin(), setv.end());
    }
    else v = this->sets.value(group);

    std::vector<SetValue>::iterator it = std::find_if(v.begin(), v.end(), [box](const SetValue &a) -> bool { return a.name == box->currentText(); });

    if (it != v.end())
    {
        for (const auto &ptr : it->pointers)
        {
            if (it->isref)
            {
                if (ptr.type == "int")
                    this->gmd.MemWriteKey(it->name.toStdString(), static_cast<int>(spinbox->value()));
                else if (ptr.type == "float")
                    this->gmd.MemWriteKey(it->name.toStdString(), static_cast<float>(spinbox->value()));

                uint32_t addr = this->gmd.GetPointerAddress(ptr.offsets, ptr.lib.toUtf8().data());
                uint32_t old = this->gmd.Protect(addr, 4, PAGE_EXECUTE_READWRITE);
                this->gmd.Write(addr, this->gmd.MemGetKeyAddress(it->name.toStdString()));
                this->gmd.Protect(addr, 4, old);
            }
            else
            {
                uint32_t addr = this->gmd.GetPointerAddress(ptr.offsets, ptr.lib.toUtf8().data());
                uint32_t old = this->gmd.Protect(addr, 4, PAGE_EXECUTE_READWRITE);

                if (ptr.type == "int")
                    this->gmd.Write(addr, static_cast<int>(ptr.reset ? 0 : spinbox->value() + ptr.value_offset));
                else if (ptr.type == "float")
                    this->gmd.Write(addr, static_cast<float>(ptr.reset ? 0 : spinbox->value() + ptr.value_offset));
                this->gmd.Protect(addr, 4, old);
            }
        }
    }
}

void MainWindow::GetSetValue(QComboBox *box, QDoubleSpinBox *input, QDoubleSpinBox *output, QString group, bool searched)
{
    std::vector<SetValue> v;

    if (searched)
    {
        for (const auto &setv : this->sets)
            v.insert(v.end(), setv.begin(), setv.end());
    }
    else v = this->sets.value(group);

    std::vector<SetValue>::iterator it = std::find_if(v.begin(), v.end(), [box](const SetValue &a) -> bool { return a.name == box->currentText(); });

    if (it != v.end())
    {
        Pointer ptr = it->pointers.front();
        uint32_t addr = it->isref ? this->gmd.Read<uint32_t>(this->gmd.GetPointerAddress(ptr.offsets, ptr.lib.toUtf8().data())) : this->gmd.GetPointerAddress(ptr.offsets, ptr.lib.toUtf8().data());
        if (ptr.type == "int")
        {
            input->setDecimals(0);
            output->setDecimals(0);
            output->setValue(this->gmd.Read<int>(addr));
        }
        else if (ptr.type == "float")
        {
            input->setDecimals(3);
            output->setDecimals(3);
            output->setValue(this->gmd.Read<float>(addr));
        }
    }
}

void MainWindow::on_globalListWidget_itemChanged(QListWidgetItem *item)
{
    this->ItemTriggered(item->checkState(), item->flags() & Qt::ItemIsUserTristate, item->text(), GLOBAL_STR);
}

void MainWindow::on_creatorListWidget_itemChanged(QListWidgetItem *item)
{
    this->ItemTriggered(item->checkState(), item->flags() & Qt::ItemIsUserTristate, item->text(), CREATOR_STR);
}

void MainWindow::on_playerListWidget_itemChanged(QListWidgetItem *item)
{
    this->ItemTriggered(item->checkState(), item->flags() & Qt::ItemIsUserTristate, item->text(), PLAYER_STR);
}

void MainWindow::on_bypassListWidget_itemChanged(QListWidgetItem *item)
{
    this->ItemTriggered(item->checkState(), item->flags() & Qt::ItemIsUserTristate, item->text(), BYPASS_STR);
}

/* -------------------------------- ABOUT -------------------------------- */

void MainWindow::on_versionCheckPushButton_clicked()
{
    char *path = this->gmd.FilePath();
    QMessageBox::information(this, "Version", tr("Your game version is %1").arg(get_version(path).c_str()));
    delete[] path;
}

void MainWindow::on_refreshPushButton_clicked()
{
    this->refresh();
}

/* -------------------------------- GLOBAL -------------------------------- */

void MainWindow::on_dllInjectPushButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Choose DLL", "/home", "Dynamic-link Libraries (*.dll);;Executable (*.exe);;All Files (*.*)");
    if (!path.isEmpty())
    {
        if (this->gmd.Inject(path.replace('/', '\\').toUtf8().data()))
            QMessageBox::information(this, "Success", "DLL probably injected.");
        else
            QMessageBox::warning(this, "Failed", "DLL injection failed.");
    }
}

void MainWindow::on_moduleInfoPushButton_clicked()
{
    QStringList modules;
    for (const auto &str : this->gmd.GetModules())
        modules << QString::fromStdString(str);

    bool ok = false;
    QString name = QInputDialog::getItem(this, "Module Info", "Choose a module", modules, 0, false, &ok);
    if (!ok)
        return;

    char *module = name.toUtf8().data();

    MODULEINFO mi;
    char *path(this->gmd.FilePath(module));

    if (GetModuleInformation(this->gmd.GetHandle(), reinterpret_cast<HMODULE>(this->gmd.GetModuleBase(module)), &mi, sizeof(MODULEINFO)) && mi.lpBaseOfDll != nullptr)
        QMessageBox::information(this, "Module Info", tr("File Path: %0\nBase: 0x%1\nImage Size: 0x%2\nEntry Point: 0x%3").arg(path, QString::number(reinterpret_cast<uint32_t>(mi.lpBaseOfDll), 16), QString::number(mi.SizeOfImage, 16), QString::number(reinterpret_cast<uint32_t>(mi.EntryPoint), 16)));
    else
        QMessageBox::warning(this, "Error", "Failed to get module info.");

    delete[] path;
}

void MainWindow::on_uApplyPushButton_clicked()
{
    if (!this->gmd.GetModuleBase(UNRANDOMISER_DLL))
    {
        if (!this->gmd.Inject(tr("%0\\dlls\\%1").arg(this->dir, UNRANDOMISER_DLL).toLocal8Bit().data()))
        {
            QMessageBox::warning(this, "Error", "Failed to inject DLL.");
            return;
        }
        Sleep(100); //bit of time for dll to load.
        this->sym_mng.Refresh();
    }

    auto module_syms = this->sym_mng.FromModule(UNRANDOMISER_DLL);
    auto it = std::find_if(module_syms.begin(), module_syms.end(), [](const WinSymbol& a) -> bool { return a.name == "hooker"; });

    if (it != module_syms.end())
    {
        std::vector<int> data = { this->ui->uStartValSpinBox->value(), this->ui->uDiffValSpinBox->value(), this->ui->uTypeComboBox->currentIndex() };

        for (const auto &sym : this->sym_mng.FromName("rand"))
            data.push_back(sym.address);
        data.push_back(0); //zero-value so the dll knows when to stop reading.

        uint32_t alloc = this->gmd.Allocate(0x1000);

        if (this->gmd.Write(alloc, &data[0], data.size() * sizeof(int)) && this->gmd.NewThread(it->address, reinterpret_cast<void*>(alloc)))
            Sleep(50); //bit of time for the thread to start & finish.
        else
            QMessageBox::warning(this, "Error", "Failed updating settings.");

        this->gmd.Free(alloc); //freeing the memory, we don't want to cause memory leak in another process!
    }
    else QMessageBox::warning(this, "Error", "Failed to find hooker function.");
}

void MainWindow::on_fpsBypassPushButton_clicked()
{
    if (!this->gmd.GetModuleBase(FRAME_BYPASS_DLL))
    {
        if (!this->gmd.Inject(tr("%0\\dlls\\%1").arg(this->dir, FRAME_BYPASS_DLL).toLocal8Bit().data()))
        {
            QMessageBox::warning(this, "Error", "Failed to inject DLL.");
            return;
        }
        Sleep(100); //bit of time for dll to load.
    }

    if (!this->gmd.Write(this->gmd.GetModuleBase(FRAME_BYPASS_DLL) + 0x13168 /*address of new anim interval*/, 1.0f / this->ui->fpsBypassSpinBox->value()))
        QMessageBox::warning(this, "Error", "Failed to write memory.");
}

void MainWindow::on_fpsBypassRecommendPushButton_clicked()
{
    DEVMODE dm;
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        this->ui->fpsBypassSpinBox->setValue(dm.dmDisplayFrequency * 4);
    else
        this->ui->fpsBypassSpinBox->setValue(240); //assuming 60hz
}

void MainWindow::on_anticheatPushButton_clicked()
{
    for (auto i = 0; i < this->ui->globalListWidget->count(); ++i)
    {
        QListWidgetItem *item = this->ui->globalListWidget->item(i);
        if (item->text().startsWith("AntiCheat:"))
            item->setCheckState(Qt::Checked);
    }
}

void MainWindow::on_labelTextPushButton_clicked()
{
    static QString prev_text = QString();
    static uint32_t addr = 0;

    bool ok;
    QString text = QInputDialog::getText(this, "Label Text", "Enter new label text (leave blank to reset)", QLineEdit::Normal, prev_text, &ok, Qt::WindowCloseButtonHint);
    if (ok)
    {
        uint32_t cclbmf_create = this->gmd.GetModuleBase("libcocos2d.dll") + 0x9C58F, cclbmf_setstr = this->gmd.GetModuleBase("libcocos2d.dll") + 0x9FB63;
        if (addr) this->gmd.Free(addr);

        if (text.isEmpty())
        {
            this->gmd.Write(cclbmf_create, "\xFF\x75\x08\xE8\x69\x00\x00\x00\x83\xC4\x18\x5D\xC3\xCC\xCC", 15);
            this->gmd.Write(cclbmf_setstr, "\x8B\x45\x08\x85\xC0", 5);
        }
        else
        {
            if ((addr = this->gmd.Allocate(text.size())) && this->gmd.Write(addr, text.toLocal8Bit().data(), text.size()))
            {
                this->gmd.Write(cclbmf_create, "\x68\x00\x00\x00\x00\xE8\x67\x00\x00\x00\x83\xC4\x18\x5D\xC3", 15);
                this->gmd.Write(cclbmf_create + 1, addr);
                this->gmd.Write(cclbmf_setstr, "\xB8", 1);
                this->gmd.Write(cclbmf_setstr + 1, addr);
            }
            else QMessageBox::warning(this, "Error", "Failed to allocate/write memory.");
        }
    }
}

void MainWindow::on_transitionPushButton_clicked()
{
    static const QStringList transitname = {"Fade (default)", "CrossFade", "FadeBL", "FadeDown", "FadeTR", "FadeUp", "FlipAngular", "FlipX", "FlipY", "JumpZoom", "MoveInB", "MoveInL", "MoveInR", "MoveInT", "RotoZoom", "ShrinkGrow", "SlideInB", "SlideInL", "SlideInR", "SlideInT", "SplitCols", "SplitRows", "TurnOffTiles", "ZoomFlipAngular", "ZoomFlipX", "ZoomFlipY", "PageTurn", "ProgressHorizontal", "ProgressInOut", "ProgressOutIn", "ProgressRadialCCW", "ProgressRadialCW", "ProgressVertical"};
    static const QVector<uint32_t> transitaddr = {0xA53D0, 0xA5320, 0xA54F0, 0xA55C0, 0xA5690, 0xA5760, 0xA5830, 0xA5950, 0xA5A70, 0xA5B90, 0xA5C40, 0xA5D10, 0xA5DE0, 0xA5EB0, 0xA5F80, 0xA6170, 0xA6240, 0xA6310, 0xA63E0, 0xA64B0, 0xA6580, 0xA6650, 0xA6720, 0xA67F0, 0xA6910, 0xA6A30, 0xA8D50, 0xA91D0, 0xA92A0, 0xA9370, 0xA9440, 0xA9510, 0xA95E0};
    static int last = 1;

    QStringList tmp(transitname);
    std::sort(tmp.begin(), tmp.end());

    bool ok;
    QString choice = QInputDialog::getItem(this, "Transition Customiser", "Choose a transition", tmp, tmp.indexOf(transitname[last]), false, &ok, Qt::WindowCloseButtonHint);

    if (ok)
    {
        last = transitname.indexOf(choice);
        uint32_t base = this->gmd.GetModuleBase("libcocos2d.dll");

        if (last)
            this->gmd.WriteJump(base + transitaddr[0], base + transitaddr[last]); //detouring default transition
        else
            this->gmd.Write(base + transitaddr[0], "\x55\x8B\xEC\x6A\xFF", 5); //setting default transition
    }
}

/* -------------------------------- PLAYER -------------------------------- */

void MainWindow::on_playerSetValuePushButton_clicked()
{
    this->SetTriggered(this->ui->playerValueComboBox, this->ui->playerValueDoubleSpinBox, PLAYER_STR);
}

void MainWindow::on_playerColourToolButton_clicked()
{
    bool ok = false;

    QString choice = QInputDialog::getItem(this, "Pick Icon Color", "Pick a colour to overwrite", {"Main", "Secondary"}, 0, false, &ok, Qt::WindowCloseButtonHint);

    if (ok)
    {
        QColor c = QColorDialog::getColor(Qt::white, this, "Pick Icon Colour");
        uint32_t addr = choice == "Main" ? 0xC8D29 : 0xC8D42;

        this->gmd.Write(this->gmd.GetBase() + addr, '\xB2'); //mov dl,imm8
        this->gmd.Write(this->gmd.GetBase() + addr + 1, static_cast<uint8_t>(c.blue()));
        this->gmd.Write(this->gmd.GetBase() + addr + 6, static_cast<uint8_t>(c.red()));
        this->gmd.Write(this->gmd.GetBase() + addr + 7, static_cast<uint8_t>(c.green()));
    }
}

void MainWindow::on_dsrInjectPushButton_clicked()
{
    if (!this->gmd.GetModuleBase(DEATH_SOUNDS_DLL))
    {
        if (!this->gmd.Inject(tr("%0\\dlls\\%1").arg(this->dir, DEATH_SOUNDS_DLL).toLocal8Bit().data()))
            QMessageBox::warning(this, "Error", "Failed to inject DLL.");
    }
    else QMessageBox::information(this, "Injector", "DLL already injected.");

    QDesktopServices::openUrl(QUrl::fromLocalFile(tr("%0\\Resources\\deathSounds").arg(tr(this->gmd.FilePath()).replace(GAME_NAME, ""))));
}

void MainWindow::on_iconRandomiserPushButton_clicked()
{
    if (!this->gmd.GetModuleBase(RANDOM_ICONS_DLL))
    {
        if (!this->gmd.Inject(tr("%0\\dlls\\%1").arg(this->dir, RANDOM_ICONS_DLL).toLocal8Bit().data()))
        {
            QMessageBox::warning(this, "Error", "Failed to inject DLL.");
            return;
        }
        Sleep(100); //bit of time for dll to load
        this->sym_mng.Refresh();
    }

    auto module_syms = this->sym_mng.FromModule(RANDOM_ICONS_DLL);
    auto it = std::find_if(module_syms.begin(), module_syms.end(), [](const WinSymbol& a) -> bool { return a.name == "toggle"; });

    if (it != module_syms.end())
    {
        if (!this->gmd.NewThread(it->address, reinterpret_cast<void*>(!this->ui->iconRandomiserPushButton->isChecked())))
            QMessageBox::warning(this, "Error", "Failed to create thread.");
    }
    else QMessageBox::warning(this, "Error", "Failed to find toggle function.");
}

void MainWindow::on_lvlPassPushButton_clicked()
{
    uint32_t addr = this->gmd.GetPointerAddress({0x003222D0, 0x164, 0x488, 0x2C4}, GAME_NAME);

    int pswd = this->gmd.Read<int>(addr) - this->gmd.Read<int>(addr + 4);

    switch (pswd)
    {
    case 0:
        QMessageBox::warning(this, "Error", "Level not copyable.");
        break;
    case 1:
        QMessageBox::warning(this, "Error", "Copyable w/o password.");
        break;
    default:
        QMessageBox::information(this, "Level Password", tr("Password: %0").arg(QString::number(pswd).remove(0, 1)));
        break;
    }
}

void MainWindow::on_setOpacityPushButton_clicked()
{
    static float x = 0;

    bool ok;
    x = QInputDialog::getDouble(this, "Set Opacity", "Enter new level opacity (-1 for default).", x * 100, -1, 100, 0, &ok) / 100;

    if (ok)
    {
        if (x == -0.01f)
            this->gmd.Write(this->gmd.GetBase() + 0xE5414, "\xF3\x0F\x10\x88\xF4\x00\x00\x00", 8);
        else
        {
            this->gmd.MemWriteKey("__opacity__", x);
            this->gmd.Write(this->gmd.GetBase() + 0xE5417, '\x0D');
            this->gmd.Write(this->gmd.GetBase() + 0xE5418, this->gmd.MemGetKeyAddress("__opacity__"));
        }
    }
}

void MainWindow::on_iconStealerPushButton_clicked()
{
    static const uint32_t offsets[9] = {0x1AC, 0x1B0, 0x1B4, 0x1B8, 0x1BC, 0x1C0, 0x1C4, 0x148, 0x14C};

    if (this->gmd.GetPointerAddress({0x3222C8, 0x1DC, 0x10, 0x0}, "GeometryDash.exe"))
    {
        for (auto i = 0; i < 9; ++i)
        {
            int val = this->gmd.Read<int>(this->gmd.GetPointerAddress({0x3222C8, 0x1DC, 0x10, offsets[i]}, GAME_NAME));
            uint32_t addr = this->gmd.GetPointerAddress({0x3222D0, 0x1E8 + static_cast<uint32_t>(0xC * i)}, GAME_NAME);
            this->gmd.Write(addr, val);
            this->gmd.Write(addr + 4, 0);
        }
    }
    else QMessageBox::warning(this, "Error", "Open the profile you want to steal from.");
}

void MainWindow::on_accountUnlinkerPushButton_clicked()
{
    if (QMessageBox::Yes == QMessageBox::question(this, "Account Unlinker", "This will log you, but no data will be lost. Continue?"))
        this->gmd.Write(this->gmd.GetPointerAddress({0x3222D8, 0x120}, "GeometryDash.exe"), 0);
}

void MainWindow::on_gamemodePushButton_clicked()
{
    static const QStringList gamemode = {"Cube", "Ship", "UFO", "Ball", "Wave", "Robot", "Spider"};

    bool ok;
    QString res = QInputDialog::getItem(this, "Gamemode", "Pick a new gamemode", gamemode, 0, false, &ok, Qt::WindowCloseButtonHint);
    if (ok)
    {
        uint32_t addr = this->gmd.GetPointerAddress({0x3222D0, 0x164, 0x224, 0x638}, GAME_NAME);
        this->gmd.Write(addr, "\x00\x00\x00\x00\x00\x00", 6);
        int choice = gamemode.indexOf(res);
        if (choice)
            this->gmd.Write(addr + choice - 1, '\x01');
    }
}


/* -------------------------------- CREATOR -------------------------------- */

void MainWindow::on_creatorSetValuePushButton_clicked()
{
    this->SetTriggered(this->ui->creatorValueComboBox, this->ui->creatorValueDoubleSpinBox, CREATOR_STR);
}

void MainWindow::on_editorColourToolButton_clicked()
{
    bool ok;
    int cid = QInputDialog::getInt(this, "Colour Picker", "Pick a ColourID", 1, 1, 999, 1, &ok);
    if (!ok) return;

    uint32_t addr = this->gmd.GetPointerAddress({0x3222D0, 0x168, 0x22C, 0xEC, 0x13C, static_cast<uint32_t>(0x4 * cid), 0xF8}, GAME_NAME);

    QColor c = QColorDialog::getColor(Qt::white, this, tr("Pick Colour %0 colour.").arg(cid));

    if (!this->gmd.Write(addr, static_cast<uint8_t>(c.red())) || !this->gmd.Write(addr + 1, static_cast<uint8_t>(c.green())) || !this->gmd.Write(addr + 2, static_cast<uint8_t>(c.blue())))
        QMessageBox::warning(this, "Error", "Colour selection not open or colour not allocated.");
}

void MainWindow::on_multiscalePushButton_clicked()
{
    bool ok;
    float f = QInputDialog::getDouble(this, "MultiScaler", "Enter new relative scale", 0, std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), 2, &ok, Qt::WindowCloseButtonHint);
    if (!ok) return;

    if (f != 0)
    {
        std::vector<uint32_t> selected; //pointers to 'GameObject's
        float avgx = 0, avgy = 0;

        int count = this->gmd.Read<int>(this->gmd.GetPointerAddress({ 0x3222D0, 0x168, 0x144, 0xB4, 0xB4, 0x3A0 }, GAME_NAME));

        for (auto i = 0; i < count; ++i)
        {
            uint32_t obj = this->gmd.GetPointerAddress({ 0x3222D0, 0x168, 0x234, 0x20, 0x8, static_cast<uint32_t>(0x4 * i), 0 }, GAME_NAME);
            if (this->gmd.Read<bool>(obj + OFFSET_SELECTED))
            {
                selected.push_back(obj);
                avgx += this->gmd.Read<float>(obj + OFFSET_X);
                avgy += this->gmd.Read<float>(obj + OFFSET_Y);
            }
        }

        avgx /= selected.size();
        avgy /= selected.size();

        for (const auto &obj : selected)
        {
            float scale = this->gmd.Read<float>(obj + OFFSET_SCALE)*f;
            this->gmd.Write(obj + OFFSET_X, avgx + (this->gmd.Read<float>(obj + OFFSET_X) - avgx)*f);
            this->gmd.Write(obj + OFFSET_Y, avgy + (this->gmd.Read<float>(obj + OFFSET_Y) - avgy)*f);
            this->gmd.Write(obj + OFFSET_SCALE, scale);
            this->gmd.Write(obj + OFFSET_SCALE_X, scale);
            this->gmd.Write(obj + OFFSET_SCALE_Y, scale);
        }
    }
    else QMessageBox::warning(this, "Error", "Cannot scale to zero.");
}

void MainWindow::on_rateLevelPushButton_clicked()
{
    static const QStringList diffs = {"NA", "Easy", "Normal", "Hard", "Harder", "Insane"};

    bool ok1, ok2;
    int stars = QInputDialog::getInt(this, "Rate Level", "Enter new star count", 0, 0, 2147483647, 1, &ok1, Qt::WindowCloseButtonHint);
    int diff = diffs.indexOf(QInputDialog::getItem(this, "Rate Level", "Choose difficulty", diffs, 0, false, &ok2, Qt::WindowCloseButtonHint)) * 10; //times 10 because that's how difficulties in this game work.

    if (ok1 && ok2)
    {
        uint32_t addr = this->gmd.GetPointerAddress({0x3222D0, 0x164, 0x22C, 0x114, 0}, "GeometryDash.exe");
        if (!(this->gmd.Write(addr + 0x2A4, stars) && this->gmd.Write(addr + 0x2A8, 0) && this->gmd.Write(addr + 0x1E4, diff)))
            QMessageBox::warning(this, "Error", "Failed to write memory, are you in the level?");
    }
}

/* -------------------------------- SEARCH -------------------------------- */

void MainWindow::on_searchLineEdit_textChanged(const QString &arg1)
{
    for (auto i = 0; i < this->ui->searchTreeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = this->ui->searchTreeWidget->topLevelItem(i);
        bool found = false;
        for (auto j = 0; j < item->childCount(); ++j)
        {
            QTreeWidgetItem *child = item->child(j);
            bool r = child->text(0).contains(arg1, Qt::CaseInsensitive) || child->text(1).contains(arg1, Qt::CaseInsensitive);
            child->setHidden(!r);
            found |= r;
        }
        item->setHidden(!found);
    }

    this->ui->searchValueComboBox->clear();
    for (auto it = this->sets.begin(); it != this->sets.end(); ++it)
    {
        for (const auto &set : it.value())
        {
            if (set.name.contains(arg1, Qt::CaseInsensitive))
                this->ui->searchValueComboBox->addItem(set.name);
        }
    }
}

void MainWindow::on_searchTreeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    this->ItemTriggered(item->checkState(column), item->flags() & Qt::ItemIsUserTristate, item->text(column), item->parent()->text(0), true);
}

void MainWindow::on_searchSetValuePushButton_clicked()
{
    this->SetTriggered(this->ui->searchValueComboBox, this->ui->searchValueDoubleSpinBox, QString(), true);
}
