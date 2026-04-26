/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
#include <QStyle>
#include <QKeySequence>
#include <QMessageBox>
#include <QPainter>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVariant>
#include <QMetaType>
#include <QIcon>
#include <QMap>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QEvent>
#include <QListWidget>
#include <QSlider>
#include <QDialogButtonBox>
#include <QSettings>
#include <QFrame>

#include "../atmolib/color.h"
#include "../config/settings.h"

using namespace NSE;

// --------------------------------------------------------------------------------
//  Demo Preview Widget (for Demo Dialog)
// --------------------------------------------------------------------------------
static QWidget *makePreview(QWidget *parent)
{
    QWidget *w = new QWidget(parent);
    QGridLayout *g = new QGridLayout(w);
    int r = 0;

    QTabWidget *tabs = new QTabWidget(w);
    tabs->addTab(new QWidget, QStringLiteral("Tab"));
    tabs->addTab(new QWidget, QStringLiteral("View"));
    g->addWidget(tabs, r++, 0, 1, 4);

    QLineEdit *le = new QLineEdit(w);
    le->setPlaceholderText(QStringLiteral("Text field"));
    g->addWidget(le, r, 0, 1, 2);
    g->addWidget(new QPushButton(QStringLiteral("Button"), w), r, 2);
    g->addWidget(new QPushButton(QStringLiteral("Button"), w), r++, 3);

    QComboBox *pop = new QComboBox(w);
    pop->addItems(QStringList{QStringLiteral("Pop Up Menu"), QStringLiteral("One"), QStringLiteral("Two")});
    g->addWidget(pop, r, 0, 1, 2);
    QComboBox *combo = new QComboBox(w);
    combo->addItems(QStringList{QStringLiteral("Combo Box"), QStringLiteral("Alpha"), QStringLiteral("Beta")});
    combo->setEditable(true);
    g->addWidget(combo, r++, 2, 1, 2);

    QGroupBox *radios = new QGroupBox(w);
    QGridLayout *rg = new QGridLayout(radios);
    rg->addWidget(new QRadioButton(QStringLiteral("Radio")), 0, 0);
    rg->addWidget(new QRadioButton(QStringLiteral("Radio")), 1, 0);
    g->addWidget(radios, r, 2, 2, 1);

    QGroupBox *checks = new QGroupBox(w);
    QGridLayout *cg = new QGridLayout(checks);
    cg->addWidget(new QCheckBox(QStringLiteral("Check")), 0, 0);
    cg->addWidget(new QCheckBox(QStringLiteral("Check")), 1, 0);
    g->addWidget(checks, r, 3, 2, 1);

    QLineEdit *search = new QLineEdit(w);
    search->setPlaceholderText(QStringLiteral("Search"));
    g->addWidget(search, r, 0, 1, 2);

    QDateEdit *date = new QDateEdit(QDate(1984, 1, 24), w);
    date->setDisplayFormat(QStringLiteral("d/M/yyyy"));
    g->addWidget(date, r++, 2, 1, 2);

    QWidget *seg = new QWidget(w);
    QGridLayout *sg = new QGridLayout(seg);
    sg->setContentsMargins(0, 0, 0, 0);
    sg->addWidget(new QPushButton(QStringLiteral("Test")), 0, 0);
    sg->addWidget(new QPushButton(QStringLiteral("One")), 0, 1);
    sg->addWidget(new QPushButton(QStringLiteral("Two")), 0, 2);
    g->addWidget(seg, r++, 0, 1, 2);

    QSlider *slider = new QSlider(Qt::Horizontal, w);
    g->addWidget(slider, r++, 0, 1, 4);

    QProgressBar *pb1 = new QProgressBar(w);
    pb1->setRange(0, 100);
    pb1->setValue(60);
    g->addWidget(pb1, r++, 0, 1, 4);

    QProgressBar *pb2 = new QProgressBar(w);
    pb2->setRange(0, 0);
    g->addWidget(pb2, r++, 0, 1, 4);

    return w;
}

// --------------------------------------------------------------------------------
//  Gradient Editor (Visual Table)
// --------------------------------------------------------------------------------
class GradientEditor : public QGroupBox
{
    Q_OBJECT
public:
    explicit GradientEditor(QWidget *parent = nullptr)
        : QGroupBox(QStringLiteral("Gradient"), parent)
    {
        QGridLayout *gl = new QGridLayout(this);
        m_table = new QTableWidget(6, 3, this);
        m_table->setHorizontalHeaderLabels(QStringList{QStringLiteral("Use"), QStringLiteral("Position"), QStringLiteral("Value")});
        m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_table->verticalHeader()->setVisible(false);

        for (int i = 0; i < 6; ++i)
        {
            QCheckBox *chk = new QCheckBox(this);
            m_table->setCellWidget(i, 0, chk);

            QDoubleSpinBox *pos = new QDoubleSpinBox(this);
            pos->setRange(0.0, 1.0);
            pos->setDecimals(2);
            pos->setSingleStep(0.1);
            m_table->setCellWidget(i, 1, pos);

            QSpinBox *val = new QSpinBox(this);
            val->setRange(-100, 100);
            m_table->setCellWidget(i, 2, val);
        }
        gl->addWidget(m_table, 0, 0, 1, 3);
    }

    void setFromString(const QString &s)
    {
        // Clear all first
        for (int i = 0; i < 6; ++i)
            static_cast<QCheckBox *>(m_table->cellWidget(i, 0))->setChecked(false);

        // Format: "0.0:5, 1.0:-5"
        const QStringList pairs = s.split(QLatin1Char(','), Qt::SkipEmptyParts);
        for (int i = 0; i < pairs.size() && i < 6; ++i)
        {
            QStringList pp = pairs.at(i).trimmed().split(QLatin1Char(':'));
            if (pp.size() != 2)
                continue;

            bool ok1 = false, ok2 = false;
            double p = pp[0].toDouble(&ok1);
            int v = pp[1].toInt(&ok2);
            if (!ok1 || !ok2)
                continue;

            static_cast<QCheckBox *>(m_table->cellWidget(i, 0))->setChecked(true);
            static_cast<QDoubleSpinBox *>(m_table->cellWidget(i, 1))->setValue(p);
            static_cast<QSpinBox *>(m_table->cellWidget(i, 2))->setValue(v);
        }
    }

    QString toString() const
    {
        QStringList out;
        for (int i = 0; i < 6; ++i)
        {
            if (!static_cast<QCheckBox *>(m_table->cellWidget(i, 0))->isChecked())
                continue;

            double p = static_cast<QDoubleSpinBox *>(m_table->cellWidget(i, 1))->value();
            int v = static_cast<QSpinBox *>(m_table->cellWidget(i, 2))->value();
            out << QString::number(p, 'f', 2) + QLatin1Char(':') + QString::number(v);
        }
        return out.join(QStringLiteral(", "));
    }

private:
    QTableWidget *m_table;
};

// --------------------------------------------------------------------------------
//  About Dialog (macOS Style)
// --------------------------------------------------------------------------------
class ManagerAboutBox : public QDialog
{
    Q_OBJECT
public:
    explicit ManagerAboutBox(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setupUi();
    }

private:
    void setupUi()
    {
        setWindowTitle(tr("About Atmo Manager"));
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);
        setFixedSize(420, 280);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(8);
        layout->setAlignment(Qt::AlignCenter);

        // Icon
        QLabel *iconLabel = new QLabel(this);
        QPixmap iconPixmap = QIcon::fromTheme(QStringLiteral("preferences-desktop")).pixmap(72, 72);
        if (iconPixmap.isNull())
            iconPixmap = style()->standardIcon(QStyle::SP_DesktopIcon).pixmap(72, 72);
        iconLabel->setPixmap(iconPixmap);
        iconLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(iconLabel);
        layout->addSpacing(8);

        // Title
        QLabel *titleLabel = new QLabel(tr("Atmo Framework Manager"), this);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(18);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        // Subtitle
        QLabel *subtitleLabel = new QLabel(tr("The Syndromatic Desktop Experience."), this);
        subtitleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(subtitleLabel);

        // Version (from applicationVersion)
        QString version = qApp->applicationVersion();
        if (version.isEmpty())
            version = QStringLiteral("1.0");
        QLabel *versionLabel = new QLabel(tr("Version %1").arg(version), this);
        QPalette vPal = versionLabel->palette();
        vPal.setColor(QPalette::WindowText, vPal.color(QPalette::WindowText).darker(130));
        versionLabel->setPalette(vPal);
        versionLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(versionLabel);

        layout->addStretch();

        // Copyright
        QLabel *copyrightLabel = new QLabel(
            tr("™ and © 2025 Syndromatic Ltd.\nAll rights reserved.\nDesigned by Kavish Krishnakumar in Manchester."),
            this);
        copyrightLabel->setAlignment(Qt::AlignCenter);
        QPalette cPal = copyrightLabel->palette();
        cPal.setColor(QPalette::WindowText, cPal.color(QPalette::WindowText).darker(130));
        copyrightLabel->setPalette(cPal);
        layout->addWidget(copyrightLabel);

        // OK button
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        layout->addWidget(buttonBox);
    }
};

// --------------------------------------------------------------------------------
//  Variable Editor Struct
// --------------------------------------------------------------------------------
struct VarEditor
{
    Settings::Key key;
    QWidget *widget;
    QString type; // "bool", "int", "float", "string", "gradient", "stringlist"
};

// --------------------------------------------------------------------------------
//  Manager Window (macOS Preferences Style)
// --------------------------------------------------------------------------------
class ManagerWindow : public QMainWindow
{
    Q_OBJECT
public:
    ManagerWindow()
    {
        Settings::read();

        setWindowTitle(QStringLiteral("Atmo Framework Manager"));
        setMinimumSize(900, 680);
        resize(980, 720);

        // ---------- Menu Bar (macOS style) ----------
        QMenu *fileMenu = menuBar()->addMenu(tr("File"));
        fileMenu->addAction(tr("Close"), QKeySequence::Close, this, &QWidget::close);

        QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
        helpMenu->addAction(tr("About Atmo Manager"), this, &ManagerWindow::showAbout);

        // ---------- Central Widget ----------
        QWidget *central = new QWidget(this);
        setCentralWidget(central);

        // Main vertical layout (content + buttons at bottom)
        QVBoxLayout *outerLayout = new QVBoxLayout(central);
        outerLayout->setContentsMargins(0, 0, 0, 0);
        outerLayout->setSpacing(0);

        // Horizontal layout for sidebar + content
        QWidget *mainContent = new QWidget(central);
        QHBoxLayout *mainLayout = new QHBoxLayout(mainContent);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // ---------- Sidebar (macOS Preferences Style) ----------
        m_sidebar = new QListWidget(this);
        m_sidebar->setIconSize(QSize(24, 24));
        m_sidebar->setFixedWidth(200);
        m_sidebar->setFrameShape(QFrame::NoFrame);
        m_sidebar->setSpacing(2);
        mainLayout->addWidget(m_sidebar);

        // ---------- Content Area ----------
        QWidget *contentArea = new QWidget(this);
        QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
        contentLayout->setContentsMargins(16, 16, 16, 16);
        contentLayout->setSpacing(12);

        // Tabs: Customise / Defaults
        m_tabs = new QTabWidget(contentArea);
        QWidget *customiseTab = new QWidget(m_tabs);
        QWidget *defaultsTab = new QWidget(m_tabs);
        m_tabs->addTab(customiseTab, tr("Customise"));
        m_tabs->addTab(defaultsTab, tr("Defaults"));
        contentLayout->addWidget(m_tabs, 1);

        // ---------- Customise Tab Layout ----------
        QVBoxLayout *customiseLay = new QVBoxLayout(customiseTab);
        customiseLay->setContentsMargins(0, 0, 0, 0);
        customiseLay->setSpacing(12);

        // Stacked widget for category pages
        m_stack = new QStackedWidget(customiseTab);

        // Empty state label
        QLabel *emptyState = new QLabel(
            tr("Choose an option to customise your Syndromatic Desktop Experience."),
            m_stack);
        emptyState->setAlignment(Qt::AlignCenter);
        emptyState->setWordWrap(true);
        QPalette ePal = emptyState->palette();
        ePal.setColor(QPalette::WindowText, ePal.color(QPalette::WindowText).darker(140));
        emptyState->setPalette(ePal);
        m_stack->addWidget(emptyState);

        // Scroll area for stacked content
        QScrollArea *scroll = new QScrollArea(customiseTab);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setWidget(m_stack);
        customiseLay->addWidget(scroll, 1);

        // Description label
        m_desc = new QLabel(
            tr("Choose an option to customise your Syndromatic Desktop Experience."),
            customiseTab);
        m_desc->setWordWrap(true);
        customiseLay->addWidget(m_desc);

        // ---------- Colours GroupBox ----------
        QGroupBox *colourBox = new QGroupBox(tr("Colours"), customiseTab);
        QGridLayout *colourLay = new QGridLayout(colourBox);

        QLabel *colourNote = new QLabel(
            tr("Colour palette follows your system accent. Use Apply to commit changes, "
               "or open the palette file for manual tweaks."),
            colourBox);
        colourNote->setWordWrap(true);
        colourLay->addWidget(colourNote, 0, 0, 1, 4);

        const QList<QPair<QPalette::ColorRole, QString>> roles = {
            {QPalette::Window, tr("Window")},
            {QPalette::WindowText, tr("Window Text")},
            {QPalette::Base, tr("Base")},
            {QPalette::Text, tr("Text")},
            {QPalette::Button, tr("Button")},
            {QPalette::ButtonText, tr("Button Text")},
            {QPalette::Highlight, tr("Highlight")}
        };

        int row = 1;
        int col = 0;
        for (const auto &pr : roles)
        {
            QPushButton *picker = new QPushButton(colourBox);
            picker->setAutoFillBackground(true);
            picker->setProperty("role", pr.first);
            picker->setFixedHeight(28);
            picker->setMinimumWidth(100);
            picker->setText(pr.second);
            updateColourButton(picker, QApplication::palette().color(pr.first));
            m_colourPickers.append(picker);

            colourLay->addWidget(picker, row, col);

            connect(picker, &QPushButton::clicked, this, [this, picker]()
            {
                const QColor current = picker->palette().color(QPalette::Button);
                QColor chosen = QColorDialog::getColor(current, this, picker->text());
                if (chosen.isValid())
                    updateColourButton(picker, chosen);
            });

            col++;
            if (col >= 4)
            {
                col = 0;
                row++;
            }
        }

        QPushButton *openPalette = new QPushButton(tr("Open User Palette"), colourBox);
        connect(openPalette, &QPushButton::clicked, this, &ManagerWindow::openPaletteFile);
        colourLay->addWidget(openPalette, row + 1, 0, 1, 2, Qt::AlignLeft);

        customiseLay->addWidget(colourBox);

        // ---------- Build Editors ----------
        buildEditors();

        // ---------- Defaults Tab ----------
        buildDefaultsPage(defaultsTab);

        mainLayout->addWidget(contentArea, 1);
        outerLayout->addWidget(mainContent, 1);

        // ---------- Bottom Buttons ----------
        QWidget *buttonBar = new QWidget(central);
        QHBoxLayout *btnLay = new QHBoxLayout(buttonBar);
        btnLay->setContentsMargins(16, 12, 16, 16);
        btnLay->setSpacing(8);

        QPushButton *btnDemo = new QPushButton(tr("Demo"), buttonBar);
        QPushButton *btnApply = new QPushButton(tr("Apply"), buttonBar);
        QPushButton *btnRevert = new QPushButton(tr("Revert to Defaults"), buttonBar);
        QPushButton *btnClose = new QPushButton(tr("Close"), buttonBar);

        // Set minimum sizes for proper button appearance
        btnDemo->setMinimumWidth(80);
        btnApply->setMinimumWidth(80);
        btnRevert->setMinimumWidth(140);
        btnClose->setMinimumWidth(80);

        btnLay->addStretch();
        btnLay->addWidget(btnDemo);
        btnLay->addWidget(btnApply);
        btnLay->addWidget(btnRevert);
        btnLay->addWidget(btnClose);

        outerLayout->addWidget(buttonBar);

        // ---------- Connections ----------
        connect(m_sidebar, &QListWidget::currentItemChanged, this, &ManagerWindow::showCategory);
        connect(btnDemo, &QPushButton::clicked, this, &ManagerWindow::showDemo);
        connect(btnApply, &QPushButton::clicked, this, &ManagerWindow::writeConfig);
        connect(btnRevert, &QPushButton::clicked, this, &ManagerWindow::revertDefaults);
        connect(btnClose, &QPushButton::clicked, this, &QWidget::close);

        // Initial state
        m_sidebar->clearSelection();
        m_stack->setCurrentIndex(0);
    }

private:
    QMap<Settings::Key, VarEditor> m_editors;
    QListWidget *m_sidebar = nullptr;
    QStackedWidget *m_stack = nullptr;
    QTabWidget *m_tabs = nullptr;
    QLabel *m_desc = nullptr;
    QDialog *m_demoDialog = nullptr;
    QWidget *m_demoPreview = nullptr;
    ManagerAboutBox *m_aboutBox = nullptr;
    QList<QPushButton *> m_colourPickers;

    // ---------- Category Helpers ----------
    static QString categoryOf(const QString &key)
    {
        if (!key.contains(QLatin1Char('.')))
            return QStringLiteral("General");
        return key.left(key.indexOf(QLatin1Char('.')));
    }

    static QString categoryDescription(const QString &c)
    {
        static QMap<QString, QString> descriptions{
            {QStringLiteral("General"), QStringLiteral("Global behaviour, menus, animation, icons, and application-wide Atmo defaults.")},
            {QStringLiteral("deco"), QStringLiteral("Window decoration shape, titlebar controls, frame depth, and legacy embedding preferences.")},
            {QStringLiteral("pushbtn"), QStringLiteral("Default push button roundness, depth, gradients, and tinting.")},
            {QStringLiteral("toolbtn"), QStringLiteral("Toolbar button material, active states, icon handling, and mask behaviour.")},
            {QStringLiteral("input"), QStringLiteral("Text fields, spin boxes, and editable controls, including UNO-specific roundness.")},
            {QStringLiteral("tabs"), QStringLiteral("Tabbed views, document tabs, Safari-style tab blending, and close button behaviour.")},
            {QStringLiteral("uno"), QStringLiteral("Unified titlebar and toolbar appearance: gradients, noise, tint, and content blending.")},
            {QStringLiteral("menues"), QStringLiteral("Menu icons, menu gradients, menu-item highlights, and item shadows.")},
            {QStringLiteral("sliders"), QStringLiteral("Slider handle material, groove fill, metallic controls, and progress-style feedback.")},
            {QStringLiteral("scrollers"), QStringLiteral("Scrollbar thickness, thumb material, groove styling, and separators.")},
            {QStringLiteral("views"), QStringLiteral("Tree/list/table view rows, headers, opacity, roundness, and traditional view styling.")},
            {QStringLiteral("progressbars"), QStringLiteral("Progress bar depth, text placement, gradients, stripes, and visual style.")},
            {QStringLiteral("windows"), QStringLiteral("Non-UNO window background gradients, noise textures, and orientation.")},
            {QStringLiteral("shadows"), QStringLiteral("Shared widget shadow opacity, text beveling, illumination, and raised-edge treatment.")}
        };
        return descriptions.value(c, QStringLiteral("Advanced Atmo settings for this category."));
    }

    static QIcon categoryIcon(const QString &c)
    {
        static QMap<QString, QString> icons{
            {QStringLiteral("General"), QStringLiteral("preferences-system")},
            {QStringLiteral("deco"), QStringLiteral("preferences-system-windows")},
            {QStringLiteral("pushbtn"), QStringLiteral("dialog-ok")},
            {QStringLiteral("toolbtn"), QStringLiteral("configure-toolbars")},
            {QStringLiteral("input"), QStringLiteral("edit-rename")},
            {QStringLiteral("tabs"), QStringLiteral("tab-new")},
            {QStringLiteral("uno"), QStringLiteral("preferences-desktop-theme")},
            {QStringLiteral("menues"), QStringLiteral("view-list-details")},
            {QStringLiteral("sliders"), QStringLiteral("audio-volume-high")},
            {QStringLiteral("scrollers"), QStringLiteral("view-scroll")},
            {QStringLiteral("views"), QStringLiteral("view-grid")},
            {QStringLiteral("progressbars"), QStringLiteral("chronometer")},
            {QStringLiteral("windows"), QStringLiteral("preferences-system-windows")},
            {QStringLiteral("shadows"), QStringLiteral("preferences-desktop-effects")}
        };
        const QIcon icon = QIcon::fromTheme(icons.value(c, QStringLiteral("preferences-system")));
        return icon.isNull() ? QIcon::fromTheme(QStringLiteral("preferences-system")) : icon;
    }

    static QString prettyCategory(const QString &c)
    {
        static QMap<QString, QString> prettyNames{
            {QStringLiteral("General"), QStringLiteral("General")},
            {QStringLiteral("deco"), QStringLiteral("Decoration")},
            {QStringLiteral("pushbtn"), QStringLiteral("Push Buttons")},
            {QStringLiteral("toolbtn"), QStringLiteral("Toolbar Buttons")},
            {QStringLiteral("input"), QStringLiteral("Input Fields")},
            {QStringLiteral("tabs"), QStringLiteral("Tabs")},
            {QStringLiteral("uno"), QStringLiteral("Unified Titlebar (UNO)")},
            {QStringLiteral("menues"), QStringLiteral("Menus")},
            {QStringLiteral("sliders"), QStringLiteral("Sliders")},
            {QStringLiteral("scrollers"), QStringLiteral("Scroll Bars")},
            {QStringLiteral("views"), QStringLiteral("Views")},
            {QStringLiteral("progressbars"), QStringLiteral("Progress Bars")},
            {QStringLiteral("windows"), QStringLiteral("Windows")},
            {QStringLiteral("shadows"), QStringLiteral("Shadows")}
        };
        QString name = prettyNames.value(c, c);
        if (name == c)
            name = name.left(1).toUpper() + name.mid(1);
        return name;
    }

    static QString prettyLabelFor(Settings::Key k)
    {
        QString desc = QString::fromLatin1(Settings::description(k));
        // British English substitutions
        desc.replace(QStringLiteral("color"), QStringLiteral("colour"), Qt::CaseInsensitive);
        desc.replace(QStringLiteral("Color"), QStringLiteral("Colour"));
        desc.replace(QStringLiteral("customize"), QStringLiteral("customise"), Qt::CaseInsensitive);
        desc.replace(QStringLiteral("Customize"), QStringLiteral("Customise"));
        return desc;
    }

    // ---------- Build Editors ----------
    void buildEditors()
    {
        // Determine categories present
        QMap<QString, QList<Settings::Key>> cats;
        for (int i = 0; i < Settings::Keycount; ++i)
        {
            Settings::Key k = static_cast<Settings::Key>(i);
            cats[categoryOf(QString::fromLatin1(Settings::key(k)))].append(k);
        }

        // Define category order (correct order, not reversed)
        const QStringList order = {
            QStringLiteral("General"), QStringLiteral("deco"), QStringLiteral("pushbtn"), QStringLiteral("toolbtn"), QStringLiteral("input"), QStringLiteral("tabs"),
            QStringLiteral("uno"), QStringLiteral("menues"), QStringLiteral("sliders"), QStringLiteral("scrollers"), QStringLiteral("views"),
            QStringLiteral("progressbars"), QStringLiteral("windows"), QStringLiteral("shadows")
        };

        // Build ordered list
        QStringList orderedKeys;
        for (const QString &o : order)
        {
            if (cats.contains(o))
                orderedKeys << o;
        }
        // Add any remaining categories not in predefined order
        for (const QString &k : cats.keys())
        {
            if (!orderedKeys.contains(k))
                orderedKeys << k;
        }

        // Create pages
        m_sidebar->clear();
        int pageIdx = 1; // 0 is empty state

        for (const QString &cat : orderedKeys)
        {
            QWidget *page = createPage(cat);
            m_stack->addWidget(page);

            QListWidgetItem *it = new QListWidgetItem(
                categoryIcon(cat),
                prettyCategory(cat));
            it->setToolTip(categoryDescription(cat));
            it->setData(Qt::UserRole, pageIdx++);
            m_sidebar->addItem(it);
        }
    }

    QWidget *createPage(const QString &cat)
    {
        QWidget *page = new QWidget;
        QVBoxLayout *pageLay = new QVBoxLayout(page);
        pageLay->setContentsMargins(18, 18, 18, 18);
        pageLay->setSpacing(14);

        QLabel *title = new QLabel(prettyCategory(cat), page);
        QFont titleFont = title->font();
        titleFont.setPointSize(titleFont.pointSize() + 5);
        titleFont.setBold(true);
        title->setFont(titleFont);
        pageLay->addWidget(title);

        QLabel *summary = new QLabel(categoryDescription(cat), page);
        summary->setWordWrap(true);
        QPalette summaryPal = summary->palette();
        summaryPal.setColor(QPalette::WindowText, summaryPal.color(QPalette::WindowText).darker(130));
        summary->setPalette(summaryPal);
        pageLay->addWidget(summary);

        QFrame *rule = new QFrame(page);
        rule->setFrameShape(QFrame::HLine);
        rule->setFrameShadow(QFrame::Sunken);
        pageLay->addWidget(rule);

        QGroupBox *settingsBox = new QGroupBox(tr("Settings"), page);
        QFormLayout *lay = new QFormLayout(settingsBox);
        lay->setLabelAlignment(Qt::AlignRight);
        lay->setRowWrapPolicy(QFormLayout::WrapLongRows);
        lay->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        lay->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
        lay->setContentsMargins(14, 16, 14, 14);
        lay->setHorizontalSpacing(18);
        lay->setVerticalSpacing(10);

        for (int i = 0; i < Settings::Keycount; ++i)
        {
            Settings::Key k = static_cast<Settings::Key>(i);
            QString name = QString::fromLatin1(Settings::key(k));
            if (categoryOf(name) != cat)
                continue;

            QVariant def = Settings::defaultValue(k);
            QVariant cur = Settings::readVal(k);
            QWidget *ed = nullptr;
            QString type;

            const int typeId = def.typeId();
            if (typeId == QMetaType::Bool)
            {
                QCheckBox *cb = new QCheckBox;
                cb->setChecked(cur.toBool());
                ed = cb;
                type = QStringLiteral("bool");
            }
            else if (typeId == QMetaType::Int)
            {
                QSpinBox *sp = new QSpinBox;
                sp->setRange(-9999, 9999);
                sp->setValue(cur.toInt());
                ed = sp;
                type = QStringLiteral("int");
            }
            else if (typeId == QMetaType::Double || typeId == QMetaType::Float)
            {
                QDoubleSpinBox *ds = new QDoubleSpinBox;
                ds->setRange(-1e6, 1e6);
                ds->setDecimals(2);
                ds->setValue(cur.toDouble());
                ed = ds;
                type = QStringLiteral("float");
            }
            else if (typeId == QMetaType::QStringList)
            {
                QLineEdit *le = new QLineEdit;
                le->setText(cur.toStringList().join(QStringLiteral(", ")));
                ed = le;
                type = QStringLiteral("stringlist");
            }
            else
            {
                // QString: detect gradient-like keys
                QString lname = name.toLower();
                if (lname.endsWith(QStringLiteral("gradient")) || lname.contains(QStringLiteral(".gradient")))
                {
                    GradientEditor *ge = new GradientEditor;
                    ge->setFromString(cur.toString());
                    ed = ge;
                    type = QStringLiteral("gradient");
                }
                else
                {
                    QLineEdit *le = new QLineEdit;
                    le->setText(cur.toString());
                    ed = le;
                    type = QStringLiteral("string");
                }
            }

            QString labelText = prettyLabelFor(k);
            QLabel *lab = new QLabel(labelText);
            lab->setWordWrap(true);
            lab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            lab->setToolTip(QStringLiteral("Key: %1\nDefault: %2")
                .arg(QString::fromLatin1(Settings::key(k)))
                .arg(Settings::defaultValue(k).toString()));

            ed->setToolTip(QString::fromLatin1(Settings::description(k)));
            ed->setProperty("desc", QString::fromLatin1(Settings::description(k)));
            ed->installEventFilter(this);

            lay->addRow(lab, ed);
            m_editors.insert(k, {k, ed, type});
        }

        // Synthetic control for progressbars.style
        if (cat == QStringLiteral("progressbars"))
        {
            QComboBox *pbStyleBox = new QComboBox(page);
            pbStyleBox->addItem(QStringLiteral("Modern (flat)"), 0);
            pbStyleBox->addItem(QStringLiteral("Traditional"), 1);
            pbStyleBox->addItem(QStringLiteral("Tinted"), 2);
            pbStyleBox->addItem(QStringLiteral("Aqua (skeuomorphic)"), 3);
            pbStyleBox->setCurrentIndex(dConf.progressbars.style);

            QLabel *lab = new QLabel(QStringLiteral("Progress Bars Style"));
            lab->setToolTip(QStringLiteral("0=Modern flat, 1=Traditional, 2=Tinted, 3=Aqua skeuomorphic"));
            pbStyleBox->setToolTip(lab->toolTip());

            lay->addRow(lab, pbStyleBox);
            page->setProperty("pbStyleBox", QVariant::fromValue(static_cast<void *>(pbStyleBox)));
        }

        pageLay->addWidget(settingsBox);
        pageLay->addStretch();
        return page;
    }

    void buildDefaultsPage(QWidget *page)
    {
        QVBoxLayout *v = new QVBoxLayout(page);
        v->setContentsMargins(16, 16, 16, 16);

        QLabel *note = new QLabel(
            tr("Defaults are sourced from the installed template NSE.conf. "
               "Use Revert to restore."),
            page);
        note->setWordWrap(true);
        v->addWidget(note);

        QTreeWidget *tw = new QTreeWidget(page);
        tw->setColumnCount(3);
        tw->setHeaderLabels(QStringList{QStringLiteral("Setting"), QStringLiteral("Default"), QStringLiteral("Description")});

        for (int i = 0; i < Settings::Keycount; ++i)
        {
            Settings::Key k = static_cast<Settings::Key>(i);
            QTreeWidgetItem *it = new QTreeWidgetItem({
                QString::fromLatin1(Settings::key(k)),
                Settings::defaultValue(k).toString(),
                QString::fromLatin1(Settings::description(k))
            });
            tw->addTopLevelItem(it);
        }

        tw->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tw->header()->setStretchLastSection(true);
        v->addWidget(tw, 1);
    }

    // ---------- Event Filter (for description updates) ----------
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

private Q_SLOTS:
    void showCategory(QListWidgetItem *current, QListWidgetItem * /*previous*/)
    {
        if (!current)
        {
            m_stack->setCurrentIndex(0);
            return;
        }
        int idx = current->data(Qt::UserRole).toInt();
        m_stack->setCurrentIndex(idx);
        if (m_desc)
            m_desc->setText(current->toolTip());
    }

    void showAbout()
    {
        if (!m_aboutBox)
            m_aboutBox = new ManagerAboutBox(this);
        m_aboutBox->show();
        m_aboutBox->raise();
        m_aboutBox->activateWindow();
    }

    void showDemo()
    {
        // Save and re-read config
        writeConfig();
        Settings::read();

        if (!m_demoDialog)
        {
            m_demoDialog = new QDialog(this);
            m_demoDialog->setWindowTitle(tr("Atmo Demo Preview"));
            m_demoDialog->resize(720, 500);

            QVBoxLayout *v = new QVBoxLayout(m_demoDialog);
            m_demoPreview = makePreview(m_demoDialog);
            v->addWidget(m_demoPreview);

            QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Close, m_demoDialog);
            connect(box, &QDialogButtonBox::rejected, m_demoDialog, &QDialog::close);
            v->addWidget(box);
        }

        // Apply Atmo style to the demo dialog
        if (QStyle *atmo = QStyleFactory::create(QStringLiteral("Atmo")))
            m_demoDialog->setStyle(atmo);

        repolishWidget(m_demoPreview);
        m_demoDialog->show();
        m_demoDialog->raise();
    }

    void writeConfig()
    {
        for (auto it = m_editors.begin(); it != m_editors.end(); ++it)
        {
            const VarEditor &ve = it.value();
            QVariant setVal;

            if (ve.type == QStringLiteral("bool"))
                setVal = static_cast<QCheckBox *>(ve.widget)->isChecked();
            else if (ve.type == QStringLiteral("int"))
                setVal = static_cast<QSpinBox *>(ve.widget)->value();
            else if (ve.type == QStringLiteral("float"))
                setVal = static_cast<QDoubleSpinBox *>(ve.widget)->value();
            else if (ve.type == QStringLiteral("stringlist"))
                setVal = static_cast<QLineEdit *>(ve.widget)->text().split(QLatin1Char(','), Qt::SkipEmptyParts);
            else if (ve.type == QStringLiteral("gradient"))
                setVal = static_cast<GradientEditor *>(ve.widget)->toString();
            else
                setVal = static_cast<QLineEdit *>(ve.widget)->text();

            Settings::writeVal(ve.key, setVal);
        }

        // Save synthetic progressbars.style
        QList<QWidget *> pages = this->findChildren<QWidget *>();
        for (QWidget *page : pages)
        {
            QVariant v = page->property("pbStyleBox");
            if (v.isValid())
            {
                QComboBox *box = static_cast<QComboBox *>(v.value<void *>());
                if (box && NSE::Settings::settings())
                    NSE::Settings::settings()->setValue(QStringLiteral("progressbars.style"), box->currentIndex());
            }
        }

        Settings::read();
        QMessageBox::information(this, QStringLiteral("Atmo"),
            tr("Settings applied. Some apps may need restart to reflect changes."));
    }

    void revertDefaults()
    {
        if (QMessageBox::question(this, tr("Revert to Defaults"),
                tr("Are you sure you want to reset all settings to defaults?"),
                QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;

        // Try to copy from system template
        const QString userDir = QDir::homePath() + QStringLiteral("/.config/NSE");
        QDir().mkpath(userDir);
        const QString userFile = userDir + QStringLiteral("/NSE.conf");
        const QString tmpl = QStandardPaths::locate(
            QStandardPaths::GenericDataLocation,
            QStringLiteral("atmo/NSE.conf"));

        if (!tmpl.isEmpty())
        {
            QFile::remove(userFile);
            if (!QFile::copy(tmpl, userFile))
            {
                QMessageBox::warning(this, QStringLiteral("Atmo"),
                    tr("Could not copy template file. Falling back to internal defaults."));
                Settings::writeDefaults();
            }
        }
        else
        {
            QMessageBox::warning(this, QStringLiteral("Atmo"),
                tr("Could not locate default template. Falling back to internal defaults."));
            Settings::writeDefaults();
        }

        // Re-read and refresh UI
        Settings::read();
        refreshEditorsFromSettings();

        QMessageBox::information(this, QStringLiteral("Atmo"), tr("Defaults restored."));
    }

    void refreshEditorsFromSettings()
    {
        for (auto it = m_editors.begin(); it != m_editors.end(); ++it)
        {
            const VarEditor &ve = it.value();
            QVariant cur = Settings::readVal(ve.key);

            if (ve.type == QStringLiteral("bool"))
                static_cast<QCheckBox *>(ve.widget)->setChecked(cur.toBool());
            else if (ve.type == QStringLiteral("int"))
                static_cast<QSpinBox *>(ve.widget)->setValue(cur.toInt());
            else if (ve.type == QStringLiteral("float"))
                static_cast<QDoubleSpinBox *>(ve.widget)->setValue(cur.toDouble());
            else if (ve.type == QStringLiteral("stringlist"))
                static_cast<QLineEdit *>(ve.widget)->setText(cur.toStringList().join(QStringLiteral(", ")));
            else if (ve.type == QStringLiteral("gradient"))
                static_cast<GradientEditor *>(ve.widget)->setFromString(cur.toString());
            else
                static_cast<QLineEdit *>(ve.widget)->setText(cur.toString());
        }

        // Refresh colour pickers
        for (QPushButton *picker : m_colourPickers)
        {
            QPalette::ColorRole role = static_cast<QPalette::ColorRole>(
                picker->property("role").toInt());
            updateColourButton(picker, QApplication::palette().color(role));
        }
    }

    void openPaletteFile()
    {
        const QString userPalette = QDir::homePath() + QStringLiteral("/.config/NSE/NSE.conf");
        const QString path = QFileInfo(userPalette).absoluteFilePath();

        // Try docsurf first (SynOS file viewer), fall back to xdg-open
        if (!QProcess::startDetached(QStringLiteral("docsurf"), QStringList() << path))
            QProcess::startDetached(QStringLiteral("xdg-open"), QStringList() << path);
    }

    // ---------- Helper Functions ----------
    static void updateColourButton(QPushButton *btn, const QColor &c)
    {
        QPalette pal = btn->palette();
        pal.setColor(QPalette::Button, c);
        pal.setColor(QPalette::ButtonText, Color::lum(c) > 140 ? Qt::black : Qt::white);
        btn->setPalette(pal);
        btn->update();
    }

    static void repolishWidget(QWidget *root)
    {
        if (!root)
            return;
        QStyle *st = root->style();
        QList<QWidget *> ws = root->findChildren<QWidget *>();
        ws << root;
        for (QWidget *w : ws)
        {
            st->unpolish(w);
            st->polish(w);
            w->update();
        }
    }
};

// --------------------------------------------------------------------------------
//  Main Entry Point
// --------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Atmo Manager"));
    app.setApplicationVersion(QStringLiteral("1.0"));
    app.setOrganizationName(QStringLiteral("Syndromatic"));
    app.setOrganizationDomain(QStringLiteral("syndromatic.com"));

    // Apply Atmo style if available
    if (QStyle *s = QStyleFactory::create(QStringLiteral("Atmo")))
        QApplication::setStyle(s);

    ManagerWindow win;
    win.show();
    return app.exec();
}

#include "main.moc"
