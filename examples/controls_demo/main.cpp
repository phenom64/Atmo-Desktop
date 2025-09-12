#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSlider>
#include <QProgressBar>
#include <QStyleFactory>

static QWidget* makeControlsPage(QWidget *parent = nullptr)
{
    QWidget *w = new QWidget(parent);
    QGridLayout *g = new QGridLayout(w);
    int r = 0;

    // Row 0: tabs
    QTabWidget *tabs = new QTabWidget(w);
    tabs->addTab(new QWidget, "Tab");
    tabs->addTab(new QWidget, "View");
    g->addWidget(tabs, r++, 0, 1, 4);

    // Row 1: line edit and buttons
    QLineEdit *le = new QLineEdit(w);
    le->setPlaceholderText("Text field");
    g->addWidget(le, r, 0, 1, 2);
    g->addWidget(new QPushButton("Button", w), r, 2);
    g->addWidget(new QPushButton("Button", w), r++, 3);

    // Row 2: popup menus / combo boxes
    QComboBox *pop = new QComboBox(w); pop->addItems({"Pop Up Menu","One","Two"});
    g->addWidget(pop, r, 0, 1, 2);
    QComboBox *combo = new QComboBox(w); combo->addItems({"Combo Box","Alpha","Beta"});
    g->addWidget(combo, r++, 2, 1, 2);

    // Row 3: radio / checks
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

    // Row 4: search field and date
    QLineEdit *search = new QLineEdit(w); search->setPlaceholderText("Search");
    g->addWidget(search, r, 0, 1, 2);
    QDateEdit *date = new QDateEdit(QDate(1984, 1, 24), w);
    date->setDisplayFormat("M/d/yyyy");
    g->addWidget(date, r++, 2, 1, 2);

    // Row 5: segmented buttons
    QWidget *seg = new QWidget(w);
    QGridLayout *sg = new QGridLayout(seg);
    sg->setContentsMargins(0,0,0,0);
    sg->addWidget(new QPushButton("Test"), 0, 0);
    sg->addWidget(new QPushButton("One"), 0, 1);
    sg->addWidget(new QPushButton("Two"), 0, 2);
    g->addWidget(seg, r++, 0, 1, 2);

    // Row 6: slider
    QSlider *slider = new QSlider(Qt::Horizontal, w);
    g->addWidget(slider, r++, 0, 1, 4);

    // Row 7: progress bars
    QProgressBar *pb1 = new QProgressBar(w); pb1->setRange(0, 100); pb1->setValue(60);
    g->addWidget(pb1, r++, 0, 1, 4);
    QProgressBar *pb2 = new QProgressBar(w); pb2->setRange(0, 0); // busy
    g->addWidget(pb2, r++, 0, 1, 4);

    return w;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    // Let user select Atmo in systemsettings; here just ensure a clean look
    QMainWindow win;
    win.setWindowTitle("SynOS Canora Controls");
    win.setCentralWidget(makeControlsPage());
    win.resize(700, 480);
    win.show();
    return app.exec();
}

