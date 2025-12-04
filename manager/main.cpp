/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 */

#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QSplitter>
#include <QScrollArea>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QProcess>
#include <QDialog>
#include <QDateEdit>
#include <QRadioButton>
#include <QProgressBar>
#include <QStyleFactory>
#include <QKeySequence>
#include <QMessageBox>
#include <QPainter>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariant>
#include <QMap>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QEvent>

#include "../nse.h"
#include "../atmolib/color.h"
#include "../config/settings.h"
#include <QSettings>

using namespace NSE;

static QWidget* makePreview(QWidget *parent)
{
    QWidget *w = new QWidget(parent);
    QGridLayout *g = new QGridLayout(w);
    int r = 0;
    QTabWidget *tabs = new QTabWidget(w);
    tabs->addTab(new QWidget, "Tab");
    tabs->addTab(new QWidget, "View");
    g->addWidget(tabs, r++, 0, 1, 4);
    QLineEdit *le = new QLineEdit(w);
    le->setPlaceholderText("Text field");
    g->addWidget(le, r, 0, 1, 2);
    g->addWidget(new QPushButton("Button", w), r, 2);
    g->addWidget(new QPushButton("Button", w), r++, 3);
    QComboBox *pop = new QComboBox(w); pop->addItems({"Pop Up Menu","One","Two"});
    g->addWidget(pop, r, 0, 1, 2);
    QComboBox *combo = new QComboBox(w); combo->addItems({"Combo Box","Alpha","Beta"});
    g->addWidget(combo, r++, 2, 1, 2);
    QGroupBox *radios = new QGroupBox(w);
    QGridLayout *rg = new QGridLayout(radios);
    rg->addWidget(new QRadioButton("Radio"), 0, 0);
    rg->addWidget(new QRadioButton("Radio"), 1, 0);
    g->addWidget(radios, r, 2, 2, 1);
    QGroupBox *checks = new QGroupBox(w);
    QGridLayout *cg = new QGridLayout(checks);
    cg->addWidget(new QCheckBox("Check"), 0, 0);
    cg->addWidget(new QCheckBox("Check"), 1, 0);
    g->addWidget(checks, r, 3, 2, 1);
    QLineEdit *search = new QLineEdit(w); search->setPlaceholderText("Search");
    g->addWidget(search, r, 0, 1, 2);
    QDateEdit *date = new QDateEdit(QDate(1984, 1, 24), w);
    date->setDisplayFormat("M/d/yyyy");
    g->addWidget(date, r++, 2, 1, 2);
    QWidget *seg = new QWidget(w);
    QGridLayout *sg = new QGridLayout(seg); sg->setContentsMargins(0,0,0,0);
    sg->addWidget(new QPushButton("Test"), 0, 0);
    sg->addWidget(new QPushButton("One"), 0, 1);
    sg->addWidget(new QPushButton("Two"), 0, 2);
    g->addWidget(seg, r++, 0, 1, 2);
    QSlider *slider = new QSlider(Qt::Horizontal, w);
    g->addWidget(slider, r++, 0, 1, 4);
    QProgressBar *pb1 = new QProgressBar(w); pb1->setRange(0, 100); pb1->setValue(60);
    g->addWidget(pb1, r++, 0, 1, 4);
    QProgressBar *pb2 = new QProgressBar(w); pb2->setRange(0, 0);
    g->addWidget(pb2, r++, 0, 1, 4);
    return w;
}

class GradientEditor : public QGroupBox {
    Q_OBJECT
public:
    explicit GradientEditor(QWidget *parent=nullptr) : QGroupBox("Gradient", parent) {
        QGridLayout *gl = new QGridLayout(this);
        m_table = new QTableWidget(6, 3, this);
        m_table->setHorizontalHeaderLabels({"Use","Position","Value"});
        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        for (int i=0;i<6;++i){
            auto *chk = new QCheckBox(this); m_table->setCellWidget(i,0,chk);
            auto *pos = new QDoubleSpinBox(this); pos->setRange(0.0,1.0); pos->setDecimals(2); m_table->setCellWidget(i,1,pos);
            auto *val = new QSpinBox(this); val->setRange(-100,100); m_table->setCellWidget(i,2,val);
        }
        gl->addWidget(m_table,0,0,1,3);
    }
    void setFromString(const QString &s){
        // format: "0.0:5, 1.0:-5"
        for (int i=0;i<6;++i){ ((QCheckBox*)m_table->cellWidget(i,0))->setChecked(false);} 
        const auto pairs = s.split(',', Qt::SkipEmptyParts);
        for (int i=0;i<pairs.size() && i<6; ++i){
            auto pp = pairs.at(i).trimmed().split(':');
            if (pp.size()!=2) continue; bool ok1=false, ok2=false;
            double p = pp[0].toDouble(&ok1); int v = pp[1].toInt(&ok2);
            if (!ok1||!ok2) continue;
            ((QCheckBox*)m_table->cellWidget(i,0))->setChecked(true);
            ((QDoubleSpinBox*)m_table->cellWidget(i,1))->setValue(p);
            ((QSpinBox*)m_table->cellWidget(i,2))->setValue(v);
        }
    }
    QString toString() const{
        QStringList out;
        for (int i=0;i<6;++i){
            if (!((QCheckBox*)m_table->cellWidget(i,0))->isChecked()) continue;
            double p = ((QDoubleSpinBox*)m_table->cellWidget(i,1))->value();
            int v = ((QSpinBox*)m_table->cellWidget(i,2))->value();
            out << QString::number(p,'f',2) + ":" + QString::number(v);
        }
        return out.join(", ");
    }
private:
    QTableWidget *m_table;
};

struct VarEditor {
    Settings::Key key; QWidget *widget; QString type; // "bool", "int", "float", "string", "gradient", "stringlist"
};

class ManagerWindow : public QMainWindow {
    Q_OBJECT
public:
    ManagerWindow(){
        setWindowTitle("Atmo Framework Manager");
        setMinimumSize(800, 600);
        resize(920, 640);

        QMenu *fileMenu = menuBar()->addMenu(tr("File"));
        fileMenu->addAction(tr("Quit"), this, &QWidget::close, QKeySequence::Quit);
        QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
        helpMenu->addAction(tr("About"), this, [this]
        {
            QMessageBox::about(this, tr("About Atmo Framework Manager"),
                               tr("Atmo Framework Manager\nConfigure NSE / Atmo desktop visuals.\nMac-inspired control surface with live previews."));
        });

        auto *central = new QWidget(this);
        auto *mainLay = new QVBoxLayout(central);
        mainLay->setContentsMargins(20, 20, 20, 20);
        mainLay->setSpacing(15);

        QGroupBox *generalBox = new QGroupBox(tr("General"), central);
        QVBoxLayout *generalLay = new QVBoxLayout(generalBox);

        auto *tabs = new QTabWidget(generalBox);
        QWidget *settingsTab = new QWidget(tabs);
        QWidget *defaultsTab = new QWidget(tabs);
        tabs->addTab(settingsTab, "Settings");
        tabs->addTab(defaultsTab, "Defaults");
        generalLay->addWidget(tabs);

        // Settings tab layout with splitter
        auto *split = new QSplitter(settingsTab);
        auto *left = new QTreeWidget(); left->setHeaderHidden(true); left->setMinimumWidth(180);
        auto *scroll = new QScrollArea(); scroll->setWidgetResizable(true);
        auto *rightStack = new QStackedWidget(); rightStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        scroll->setWidget(rightStack);
        split->addWidget(left); split->addWidget(scroll);
        split->setStretchFactor(0,0); split->setStretchFactor(1,1);
        auto *settingsLay = new QVBoxLayout(settingsTab);
        settingsLay->addWidget(split);

        // Build variable editors grouped by prefix
        buildEditors(left, rightStack);
        buildDefaultsPage(defaultsTab);

        generalBox->setLayout(generalLay);
        mainLay->addWidget(generalBox);

        QGroupBox *decoBox = new QGroupBox(tr("Decoration"), central);
        QVBoxLayout *decoLay = new QVBoxLayout(decoBox);
        m_preview = makePreview(decoBox);
        decoLay->addWidget(m_preview);
        decoBox->setLayout(decoLay);
        mainLay->addWidget(decoBox);

        QGroupBox *colorBox = new QGroupBox(tr("Colors"), central);
        QGridLayout *colorLay = new QGridLayout(colorBox);
        QLabel *colorNote = new QLabel(tr("Palette follows your system accent. Use Apply to commit changes, or open the palette file for manual tweaks."), colorBox);
        colorNote->setWordWrap(true);
        colorLay->addWidget(colorNote, 0, 0, 1, 2);
        const QList<QPair<QPalette::ColorRole, QString> > roles = {
            qMakePair(QPalette::Window, tr("Window")),
            qMakePair(QPalette::WindowText, tr("Window Text")),
            qMakePair(QPalette::Base, tr("Base")),
            qMakePair(QPalette::Text, tr("Text")),
            qMakePair(QPalette::Button, tr("Button")),
            qMakePair(QPalette::ButtonText, tr("Button Text")),
            qMakePair(QPalette::Highlight, tr("Highlight"))
        };
        int row = 1;
        for (const auto &pr : roles)
        {
            QPushButton *picker = new QPushButton(colorBox);
            picker->setAutoFillBackground(true);
            picker->setProperty("role", pr.first);
            picker->setFixedHeight(28);
            picker->setText(pr.second);
            updateColorButton(picker, QApplication::palette().color(pr.first));
            colorLay->addWidget(new QLabel(pr.second, colorBox), row, 0);
            colorLay->addWidget(picker, row, 1);
            connect(picker, &QPushButton::clicked, this, [picker]()
            {
                const QColor current = picker->palette().color(QPalette::Button);
                QColor chosen = QColorDialog::getColor(current, picker, picker->text());
                if (chosen.isValid())
                    updateColorButton(picker, chosen);
            });
            ++row;
        }
        QPushButton *openPalette = new QPushButton(tr("Open User Palette"), colorBox);
        colorLay->addWidget(openPalette, row, 0, 1, 2, Qt::AlignLeft);
        mainLay->addWidget(colorBox);

        // Bottom buttons
        auto *btnLay = new QHBoxLayout();
        QPushButton *btnDemo = new QPushButton("Demo", central);
        QPushButton *btnWrite = new QPushButton("Apply", central);
        QPushButton *btnRevert = new QPushButton("Revert to Defaults", central);
        QPushButton *btnClose = new QPushButton("Close", central);
        btnLay->addStretch();
        btnLay->addWidget(btnDemo);
        btnLay->addWidget(btnWrite);
        btnLay->addWidget(btnRevert);
        btnLay->addWidget(btnClose);
        mainLay->addLayout(btnLay);
        setCentralWidget(central);

        connect(left, &QTreeWidget::currentItemChanged, this, [=](QTreeWidgetItem *cur){ if(!cur) return; rightStack->setCurrentIndex(cur->data(0, Qt::UserRole).toInt()); if (m_desc) m_desc->setText(cur->text(0));});
        m_desc = new QLabel(tr("Select a category or control to see details."), central);
        m_desc->setWordWrap(true);
        mainLay->addWidget(m_desc);

        connect(btnDemo, &QPushButton::clicked, this, &ManagerWindow::showDemo);
        connect(btnClose, &QPushButton::clicked, this, &QWidget::close);
        connect(btnWrite, &QPushButton::clicked, this, &ManagerWindow::writeConfig);
        connect(btnRevert, &QPushButton::clicked, this, &ManagerWindow::revertDefaults);
        connect(openPalette, &QPushButton::clicked, this, [this]
        {
            const QString userPalette = QDir::homePath()+"/.config/NSE/NSE.conf";
            QProcess::startDetached("xdg-open", QStringList() << QFileInfo(userPalette).absolutePath());
        });
    }

private:
    QMap<Settings::Key, VarEditor> m_editors;
    QWidget *m_preview = nullptr; QDialog *m_demoDialog = nullptr; QWidget *m_demoPreview = nullptr; QLabel *m_desc = nullptr;

    QString categoryOf(const QString &key) const{
        if (!key.contains('.')) return QStringLiteral("General");
        return key.left(key.indexOf('.'));
    }

    static QString prettyCategory(const QString &c){
        static QMap<QString, QString> prettyNames{
            {"General", "General"},
            {"deco","Window Decoration"},
            {"pushbtn","Push Buttons"},
            {"toolbtn","Toolbar Buttons"},
            {"input","Input Fields"},
            {"tabs","Tabs"},
            {"uno","Unified Titlebar (UNO)"},
            {"menues","Menus"},
            {"sliders","Sliders"},
            {"scrollers","Scroll Bars"},
            {"views","Views"},
            {"progressbars","Progress Bars"},
            {"windows","Windows"},
            {"shadows","Shadows"}
        };
        return prettyNames.value(c, c.left(1).toUpper()+c.mid(1));
    }

    static QString prettyLabelFor(Settings::Key k){ return QString::fromLatin1(Settings::description(k)); }

    QWidget* createPage(const QString &cat){
        QWidget *w = new QWidget; auto *lay = new QFormLayout(w);
        lay->setLabelAlignment(Qt::AlignRight);
        lay->setRowWrapPolicy(QFormLayout::WrapLongRows);
        lay->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        lay->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        for (int i=0;i<Settings::Keycount;++i){
            Settings::Key k = static_cast<Settings::Key>(i);
            QString name = Settings::key(k);
            if (categoryOf(name) != cat) continue;
            QVariant def = Settings::defaultValue(k);
            QVariant cur = Settings::readVal(k);
            QWidget *ed = nullptr; QString type;
            if (def.type()==QVariant::Bool){ auto *cb = new QCheckBox; cb->setChecked(cur.toBool()); ed=cb; type="bool"; }
            else if (def.type()==QVariant::Int){ auto *sp = new QSpinBox; sp->setRange(-9999, 9999); sp->setValue(cur.toInt()); ed=sp; type="int"; }
            else if (def.type()==QVariant::Double){ auto *ds = new QDoubleSpinBox; ds->setRange(-1e6, 1e6); ds->setDecimals(2); ds->setValue(cur.toDouble()); ed=ds; type="float"; }
            else if (def.type()==QVariant::StringList){ auto *le = new QLineEdit; le->setText(cur.toStringList().join(", ")); ed=le; type="stringlist"; }
            else {
                // QString: detect gradient-like keys
                QString lname = name.toLower();
                if (lname.endsWith("gradient") || lname.contains(".gradient")){
                    auto *ge = new GradientEditor; ge->setFromString(cur.toString()); ed=ge; type="gradient";
                } else {
                    auto *le = new QLineEdit; le->setText(cur.toString()); ed=le; type="string";
                }
            }
            QString labelText = prettyLabelFor(k);
            auto *lab = new QLabel(labelText); lab->setWordWrap(true); lab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            lab->setToolTip(QString("%1\nKey: %2\nDefault: %3").arg(labelText).arg(QString::fromLatin1(Settings::key(k))).arg(Settings::defaultValue(k).toString()));
            ed->setToolTip(QString::fromLatin1(Settings::description(k)));
            ed->setProperty("desc", QString::fromLatin1(Settings::description(k)));
            ed->installEventFilter(this);
            lay->addRow(lab, ed);
            m_editors.insert(k, {k, ed, type});
        }
        // Synthetic control for progressbars.style
        if (cat == "progressbars"){
            QComboBox *pbStyleBox = new QComboBox(w);
            pbStyleBox->addItem("Modern (flat)", 0);
            pbStyleBox->addItem("Traditional", 1);
            pbStyleBox->addItem("Tinted", 2);
            pbStyleBox->addItem("Aqua (skeuomorphic)", 3);
            pbStyleBox->setCurrentIndex(dConf.progressbars.style);
            QLabel *lab = new QLabel("Progress Bars Style");
            lab->setToolTip("0=Modern flat, 1=Traditional, 2=Tinted, 3=Aqua skeuomorphic");
            pbStyleBox->setToolTip(lab->toolTip());
            lay->addRow(lab, pbStyleBox);
            // store pointer for save
            w->setProperty("pbStyleBox", QVariant::fromValue((void*)pbStyleBox));
        }
        return w;
    }

    void buildEditors(QTreeWidget *tree, QStackedWidget *stack){
        // Determine categories present
        QMap<QString, QList<Settings::Key>> cats;
        for (int i=0;i<Settings::Keycount;++i){
            Settings::Key k = static_cast<Settings::Key>(i);
            cats[categoryOf(Settings::key(k))].append(k);
        }
        // Stable order
        const QStringList order = {"General","deco","pushbtn","toolbtn","input","tabs","uno","menues","sliders","scrollers","views","progressbars","windows","shadows"};
        QStringList keys = cats.keys(); for (const QString &o: order){ if (keys.removeOne(o)) keys.prepend(o); }
        int idx = 0; tree->clear();
        for (const QString &cat : keys){
            QWidget *page = createPage(cat);
            stack->addWidget(page);
            auto *it = new QTreeWidgetItem(QStringList() << prettyCategory(cat));
            it->setData(0, Qt::UserRole, idx++);
            tree->addTopLevelItem(it);
        }
        if (tree->topLevelItemCount()>0){ tree->setCurrentItem(tree->topLevelItem(0)); }
    }

    void buildDefaultsPage(QWidget *page){
        auto *v = new QVBoxLayout(page);
        auto *tw = new QTreeWidget(page);
        tw->setColumnCount(3); tw->setHeaderLabels({"Setting","Default","Description"});
        for (int i=0;i<Settings::Keycount;++i){
            Settings::Key k = static_cast<Settings::Key>(i);
            auto *it = new QTreeWidgetItem({QString::fromLatin1(Settings::key(k)), Settings::defaultValue(k).toString(), QString::fromLatin1(Settings::description(k))});
            tw->addTopLevelItem(it);
        }
        tw->header()->setSectionResizeMode(QHeaderView::ResizeToContents); tw->header()->setStretchLastSection(true);
        v->addWidget(new QLabel("Defaults are sourced from the installed template NSE.conf. Use Revert to restore."));
        v->addWidget(tw);
    }

private slots:
    void showDemo(){
        writeConfig(); // persist then re-read
        Settings::read();
        if (!m_demoDialog)
        { 
            m_demoDialog = new QDialog(this); m_demoDialog->setWindowTitle("Atmo Demo Preview"); QVBoxLayout *v = new QVBoxLayout(m_demoDialog); m_demoPreview = makePreview(m_demoDialog); v->addWidget(m_demoPreview); m_demoDialog->resize(700,480);
        } 
        m_demoDialog->setStyle(qApp->style());
        repolishPreview(); m_demoDialog->show();
    }

    void writeConfig(){
        for (auto it = m_editors.begin(); it != m_editors.end(); ++it){
            const VarEditor &ve = it.value(); QVariant setVal;
            if (ve.type=="bool") setVal = ((QCheckBox*)ve.widget)->isChecked();
            else if (ve.type=="int") setVal = ((QSpinBox*)ve.widget)->value();
            else if (ve.type=="float") setVal = ((QDoubleSpinBox*)ve.widget)->value();
            else if (ve.type=="stringlist") setVal = ((QLineEdit*)ve.widget)->text().split(',', Qt::SkipEmptyParts);
            else if (ve.type=="gradient") setVal = ((GradientEditor*)ve.widget)->toString();
            else setVal = ((QLineEdit*)ve.widget)->text();
            Settings::writeVal(ve.key, setVal);
        }
        // Save synthetic progressbars.style
        QList<QWidget*> pages = this->findChildren<QWidget*>();
        for (QWidget *page : pages){
            QVariant v = page->property("pbStyleBox");
            if (v.isValid()){
                QComboBox *box = reinterpret_cast<QComboBox*>(v.value<void*>());
                if (box && NSE::Settings::settings()) NSE::Settings::settings()->setValue("progressbars.style", box->currentIndex());
            }
        }
        Settings::read(); if (m_demoDialog) repolishPreview();
        QMessageBox::information(this, "Atmo", "Settings applied. Some apps may need restart to reflect changes.");
    }

    void repolishWidget(QWidget *root){ if (!root) return; auto *st = root->style(); QList<QWidget*> ws = root->findChildren<QWidget*>(); ws << root; for (QWidget *w : ws){ st->unpolish(w); st->polish(w); w->update(); } }
    void repolishPreview(){ repolishWidget(m_preview); repolishWidget(m_demoPreview); }
    static void updateColorButton(QPushButton *btn, const QColor &c)
    {
        QPalette pal(btn->palette());
        pal.setColor(QPalette::Button, c);
        pal.setColor(QPalette::ButtonText, Color::lum(c) > 140 ? Qt::black : Qt::white);
        btn->setPalette(pal);
        btn->update();
    }
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (m_desc && (e->type() == QEvent::FocusIn || e->type() == QEvent::Enter))
        {
            const QString desc = o->property("desc").toString();
            if (!desc.isEmpty())
                m_desc->setText(desc);
        }
        return QMainWindow::eventFilter(o, e);
    }

    bool copyDefaultNSEConf(){ const QString userDir = QDir::homePath()+"/.config/NSE"; QDir().mkpath(userDir); const QString userFile=userDir+"/NSE.conf"; const QString tmpl = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("atmo/NSE.conf")); if (tmpl.isEmpty()) return false; QFile::remove(userFile); return QFile::copy(tmpl, userFile);} 

    void revertDefaults(){ if (!copyDefaultNSEConf()){ QMessageBox::warning(this, "Atmo", "Could not locate default template. Falling back to internal defaults."); Settings::writeDefaults(); } Settings::read(); for (auto it=m_editors.begin(); it!=m_editors.end(); ++it){ const VarEditor &ve = it.value(); QVariant cur = Settings::readVal(ve.key); if (ve.type=="bool") ((QCheckBox*)ve.widget)->setChecked(cur.toBool()); else if (ve.type=="int") ((QSpinBox*)ve.widget)->setValue(cur.toInt()); else if (ve.type=="float") ((QDoubleSpinBox*)ve.widget)->setValue(cur.toDouble()); else if (ve.type=="stringlist") ((QLineEdit*)ve.widget)->setText(cur.toStringList().join(", ")); else if (ve.type=="gradient") ((GradientEditor*)ve.widget)->setFromString(cur.toString()); else ((QLineEdit*)ve.widget)->setText(cur.toString()); } if (m_demoDialog) repolishPreview(); QMessageBox::information(this, "Atmo", "Defaults restored."); }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setStyle(new NSE::Style());
    ManagerWindow win; win.show();
    return app.exec();
}

#include "main.moc"
