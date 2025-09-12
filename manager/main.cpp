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
#include <QProcess>
#include <QDialog>
#include <QDateEdit>
#include <QRadioButton>
#include <QProgressBar>
#include <QStyleFactory>
#include <QMessageBox>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariant>
#include <QMap>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

#include "../config/settings.h"

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
        resize(920, 640);

        auto *central = new QWidget(this);
        auto *mainLay = new QVBoxLayout(central);

        auto *tabs = new QTabWidget(central);
        QWidget *settingsTab = new QWidget(tabs);
        QWidget *defaultsTab = new QWidget(tabs);
        tabs->addTab(settingsTab, "Settings");
        tabs->addTab(defaultsTab, "Defaults");
        mainLay->addWidget(tabs);

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

        // Build variable editors grouped by prefix
        buildEditors(left, rightStack);
        buildDefaultsPage(defaultsTab);

        connect(left, &QTreeWidget::currentItemChanged, this, [=](QTreeWidgetItem *cur){ if(!cur) return; rightStack->setCurrentIndex(cur->data(0, Qt::UserRole).toInt());});
        connect(btnDemo, &QPushButton::clicked, this, &ManagerWindow::showDemo);
        connect(btnClose, &QPushButton::clicked, this, &QWidget::close);
        connect(btnWrite, &QPushButton::clicked, this, &ManagerWindow::writeConfig);
        connect(btnRevert, &QPushButton::clicked, this, &ManagerWindow::revertDefaults);
    }

private:
    QMap<Settings::Key, VarEditor> m_editors;
    QWidget *m_preview = nullptr; QDialog *m_demoDialog = nullptr;

    QString categoryOf(const QString &key) const{
        if (!key.contains('.')) return QStringLiteral("General");
        return key.left(key.indexOf('.'));
    }

    static QString prettyCategory(const QString &c){
        static QMap<QString, QString> map{{"deco","Decoration"},{"pushbtn","Push Button"},{"toolbtn","Toolbar Button"},{"input","Input Fields"},{"tabs","Tabs"},{"uno","UNO"},{"menues","Menus"},{"sliders","Sliders"},{"scrollers","Scroll Bars"},{"views","Views"},{"progressbars","Progress Bars"},{"windows","Windows"},{"shadows","Shadows"}};
        if (c=="General") return c; return map.value(c, c.left(1).toUpper()+c.mid(1));
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
            lay->addRow(lab, ed);
            m_editors.insert(k, {k, ed, type});
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
        if (!m_demoDialog){ m_demoDialog = new QDialog(this); m_demoDialog->setWindowTitle("Atmo Demo Preview"); QVBoxLayout *v = new QVBoxLayout(m_demoDialog); m_preview = makePreview(m_demoDialog); v->addWidget(m_preview); m_demoDialog->resize(700,480);} 
        if (QStyle *s = QStyleFactory::create("Atmo")) m_demoDialog->setStyle(s);
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
        Settings::read(); if (m_demoDialog) repolishPreview();
        QMessageBox::information(this, "Atmo", "Settings applied. Some apps may need restart to reflect changes.");
    }

    void repolishPreview(){ if (!m_preview) return; auto *st = m_preview->style(); QList<QWidget*> ws = m_preview->findChildren<QWidget*>(); ws << m_preview; for (QWidget *w : ws){ st->unpolish(w); st->polish(w); w->update(); } }

    bool copyDefaultNSEConf(){ const QString userDir = QDir::homePath()+"/.config/NSE"; QDir().mkpath(userDir); const QString userFile=userDir+"/NSE.conf"; const QString tmpl = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("atmo/NSE.conf")); if (tmpl.isEmpty()) return false; QFile::remove(userFile); return QFile::copy(tmpl, userFile);} 

    void revertDefaults(){ if (!copyDefaultNSEConf()){ QMessageBox::warning(this, "Atmo", "Could not locate default template. Falling back to internal defaults."); Settings::writeDefaults(); } Settings::read(); for (auto it=m_editors.begin(); it!=m_editors.end(); ++it){ const VarEditor &ve = it.value(); QVariant cur = Settings::readVal(ve.key); if (ve.type=="bool") ((QCheckBox*)ve.widget)->setChecked(cur.toBool()); else if (ve.type=="int") ((QSpinBox*)ve.widget)->setValue(cur.toInt()); else if (ve.type=="float") ((QDoubleSpinBox*)ve.widget)->setValue(cur.toDouble()); else if (ve.type=="stringlist") ((QLineEdit*)ve.widget)->setText(cur.toStringList().join(", ")); else if (ve.type=="gradient") ((GradientEditor*)ve.widget)->setFromString(cur.toString()); else ((QLineEdit*)ve.widget)->setText(cur.toString()); } if (m_demoDialog) repolishPreview(); QMessageBox::information(this, "Atmo", "Defaults restored."); }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    ManagerWindow win; win.show();
    return app.exec();
}

#include "main.moc"
