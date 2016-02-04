#include "dsp.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/shadowhandler.h"
#include "stylelib/progresshandler.h"
#include "stylelib/animhandler.h"
#include "stylelib/handlers.h"
#include "stylelib/xhandler.h"
#include "config/settings.h"
#include "stylelib/color.h"
#include "stylelib/stackanimator.h"
#include "defines.h"

#include <QStackedLayout>
#include <QWidget>
#include <QToolBar>
#include <QFrame>
#include <QMainWindow>
#include <QLayout>
#include <QMenuBar>
#include <QToolButton>
#include <QAbstractItemView>
#include <QGroupBox>
#include <QCheckBox>
#include <QDockWidget>
#include <QDesktopWidget>
#include <QLabel>
#include <QProgressBar>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QHeaderView>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QScrollBar>
#include <QTextEdit>
#include <QDialog>
#include <QStatusBar>
#include <QToolBox>
#include <QDebug>
//#include <QWebView>

/* Thomas gave me his blessing to use
 * his macmenu! yeah! so now we get
 * macmenues in styleproject!
 */
#include "defines.h"
#if HASDBUS
#include "stylelib/macmenu.h"
#endif

using namespace DSP;

static void applyBlur(QWidget *widget)
{
    unsigned int d(0);
    XHandler::setXProperty<unsigned int>(widget->winId(), XHandler::_KDE_NET_WM_BLUR_BEHIND_REGION, XHandler::Long, &d);
}

void
Style::applyTranslucency(QWidget *widget)
{
    const QIcon icn = widget->windowIcon();
    const bool wasVisible= widget->isVisible();
    const bool wasMoved = widget->testAttribute(Qt::WA_Moved);
    if (wasVisible)
        widget->hide();
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setWindowIcon(icn);
    widget->setAttribute(Qt::WA_Moved, wasMoved); // https://bugreports.qt-project.org/browse/QTBUG-34108
    widget->setVisible(wasVisible);
}

void
Style::installFilter(QWidget *w)
{
    w->removeEventFilter(this);
    w->installEventFilter(this);
}

void
Style::polish(QWidget *widget)
{
    if (!widget)
        return;

    QCommonStyle::polish(widget);
#if DEBUG
    if (widget->parentWidget())
        qDebug() << "polishing widget:" << widget << "with parentWidget:" << widget->parentWidget();

    installFilter(widget);
#endif
    if (qobject_cast<Handlers::Balloon *>(widget)
            || qobject_cast<SplitterExt *>(widget)
            || qobject_cast<Buttons *>(widget)
            || qobject_cast<TitleWidget *>(widget))
        return;
//    if (qobject_cast<QWebView *>(widget))
    if (widget->inherits("QWebView")
            || widget->inherits("QWebPage")
            || widget->inherits("QWebFrame"))
        return;

    //some special handling
    if (dConf.splitterExt
            && (widget->objectName() == "qt_qmainwindow_extended_splitter"
                || qobject_cast<QSplitterHandle *>(widget)))
        SplitterExt::manage(widget);

    if (dConf.uno.enabled
            && dConf.app == Settings::Konversation
            && qobject_cast<QMainWindow *>(widget->window())
            && (qobject_cast<QHBoxLayout *>(widget->layout())
                || qobject_cast<QVBoxLayout *>(widget->layout())))
        static_cast<QBoxLayout *>(widget->layout())->setSpacing(0);

    if (dConf.balloonTips
            && (qobject_cast<QAbstractButton *>(widget)
                || qobject_cast<QSlider *>(widget)
                || qobject_cast<QLabel *>(widget)
                || widget->inherits("BE::Clock")))
        Handlers::BalloonHelper::manage(widget);

    if (dConf.animateScroll
            && qobject_cast<QAbstractScrollArea *>(widget))
        Anim::ScrollAnimator::manage(static_cast<QAbstractScrollArea *>(widget));

    if (qobject_cast<QPushButton *>(widget)
            || qobject_cast<QCheckBox *>(widget)
            || qobject_cast<QRadioButton *>(widget)
            || qobject_cast<QSlider *>(widget)
            || qobject_cast<QScrollBar *>(widget))
        Anim::Basic::manage(widget);

    if (Handlers::Drag::canDrag(widget))
        Handlers::Drag::manage(widget);

    if (dConf.animateStack)
    {
        const QList<QStackedLayout *> layouts = widget->findChildren<QStackedLayout *>();
        for (int i = 0; i < layouts.count(); ++i)
            StackAnimator::manage(layouts.at(i));
    }
    if (widget->isWindow())
    {
        if (qobject_cast<QMainWindow *>(widget))
        {
            if (dConf.compactMenu && widget->findChild<QMenuBar *>())
                Handlers::Window::addCompactMenu(widget);
#if QT_VERSION < 0x050000
            if (XHandler::opacity() < 1.0f && !widget->testAttribute(Qt::WA_TranslucentBackground))
                applyTranslucency(widget);
#endif
            if (dConf.lockDocks)
                Handlers::Dock::manage(widget);
            Handlers::Window::manage(widget);
            if (dConf.removeTitleBars)
                ShadowHandler::manage(widget);
        }
        else if (qobject_cast<QDialog *>(widget))
        {
#if QT_VERSION < 0x050000
            if (!dConf.uno.enabled
                    && XHandler::opacity() < 1.0f
                    && !widget->testAttribute(Qt::WA_TranslucentBackground))
                applyTranslucency(widget);
#endif
            Handlers::Window::manage(widget);
        }
        const bool isFrameLessMainWindow(((widget->windowFlags() & Qt::FramelessWindowHint)
                                          && widget->windowRole().startsWith("MainWindow")));
        if (widget->windowType() == Qt::Popup
                || widget->windowType() == Qt::ToolTip
                || widget->windowType() == Qt::Dialog
                || isFrameLessMainWindow)
            ShadowHandler::manage(widget);
        if (widget->testAttribute(Qt::WA_TranslucentBackground)
                || (qobject_cast<QDialog *>(widget) && !(widget->windowFlags() & Qt::FramelessWindowHint)))
            applyBlur(widget);
    }

    //main if segment for all widgets
    if (QToolBar *bar = qobject_cast<QToolBar *>(widget))
    {
        bar->setForegroundRole(QPalette::WindowText);
        bar->setBackgroundRole(QPalette::Window);
        Handlers::ToolBar::manageToolBar(bar);
    }
    else if (QToolButton *btn = qobject_cast<QToolButton *>(widget))
    {
        if (!qobject_cast<QToolBar *>(btn->parentWidget()))
        {
            btn->setBackgroundRole(QPalette::Window);
            btn->setForegroundRole(QPalette::WindowText);
        }
        Anim::ToolBtns::manage(btn);
        Handlers::ToolBar::manage(btn);
    }
    else if (QHeaderView *hw = qobject_cast<QHeaderView *>(widget))
    {
        hw->viewport()->setBackgroundRole(QPalette::Base);
        hw->viewport()->setForegroundRole(QPalette::Text);
    }
    else if (QProgressBar *pBar = qobject_cast<QProgressBar *>(widget))
    {
        pBar->setBackgroundRole(QPalette::Base);
        pBar->setForegroundRole(QPalette::Text);
        QFont f(pBar->font());
        f.setBold(true);
        pBar->setFont(f);
        ProgressHandler::manage(pBar);
    }
    else if (QComboBox *cBox = qobject_cast<QComboBox *>(widget))
    {
        if (!cBox->isEditable())
        {
            Anim::Basic::manage(cBox);
            cBox->setForegroundRole(QPalette::ButtonText);
            cBox->setBackgroundRole(QPalette::Button);
        }
        else if (QLineEdit *l = cBox->findChild<QLineEdit *>())
            connect(l, SIGNAL(textChanged(QString)), cBox, SLOT(update()));
    }
    else if (qobject_cast<QDockWidget *>(widget))
    {
        widget->setAutoFillBackground(false);
    }
    else if (qobject_cast<QScrollBar *>(widget))
    {
        widget->setAttribute(Qt::WA_Hover);
    }
    else if (QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(widget))
    {
        if (dConf.uno.enabled
                && dConf.uno.contAware
                && qobject_cast<QMainWindow *>(widget->window()))
            Handlers::ScrollWatcher::watch(area);

#if QT_VERSION >= 0x050000
        if (pixelMetric(PM_ScrollView_ScrollBarOverlap, 0, area))
            installFilter(area->viewport());
#endif

        if (QAbstractItemView *view = qobject_cast<QAbstractItemView *>(area))
        {
            view->viewport()->setAttribute(Qt::WA_Hover);
            view->setAttribute(Qt::WA_MouseTracking);
            installFilter(view);

            //some views paint w/ button background on some items w/o setting text color (ktelepathy contacts list for example...)
            QPalette pal(widget->palette());
            const bool dark(Color::luminosity(pal.color(QPalette::Base)) < Color::luminosity(pal.color(QPalette::Text)));
            QColor (QColor::*function)(int) const = dark?&QColor::lighter:&QColor::darker;
            pal.setColor(QPalette::Button, (pal.color(QPalette::Base).*function)(120));
            pal.setColor(QPalette::ButtonText, pal.color(QPalette::Text));
            widget->setPalette(pal);
        }
    }
    else if (qobject_cast<QStatusBar *>(widget))
    {
        widget->setForegroundRole(QPalette::WindowText);
        widget->setBackgroundRole(QPalette::Window);
    }
    else if (qobject_cast<QMenu *>(widget))
    {
        if (!widget->testAttribute(Qt::WA_TranslucentBackground))
            applyTranslucency(widget);
        widget->setMouseTracking(true);
        widget->setAttribute(Qt::WA_Hover);
        widget->setForegroundRole(QPalette::Text);
        widget->setBackgroundRole(QPalette::Base);
        installFilter(widget);
        ShadowHandler::manage(widget);
        if (XHandler::opacity() < 1.0f)
        {
            unsigned int d(0);
            XHandler::setXProperty<unsigned int>(widget->winId(), XHandler::_KDE_NET_WM_BLUR_BEHIND_REGION, XHandler::Long, &d);
        }
    }
    else if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(widget))
    {
#if HASDBUS
        BE::MacMenu::manage(menuBar);
#endif
        widget->setMouseTracking(true);
        widget->setAttribute(Qt::WA_Hover);
        widget->setForegroundRole(QPalette::WindowText);
        widget->setBackgroundRole(QPalette::Window);
        installFilter(widget);
    }
    else if (qobject_cast<QToolBox *>(widget))
    {
        widget->setBackgroundRole(QPalette::Window);
        widget->setForegroundRole(QPalette::WindowText);
        if (QLayout *l = widget->layout())
        {
            l->setMargin(0);
            l->setSpacing(0);
        }
    }
    else if (QTabBar *tabBar = qobject_cast<QTabBar *>(widget))
    {
        const bool safari(Ops::isSafariTabBar(tabBar)); //hmmm
        if (safari || tabBar->documentMode())
        {
            tabBar->setBackgroundRole(QPalette::Window);
            tabBar->setForegroundRole(QPalette::WindowText);
        }
        else
        {
            tabBar->setBackgroundRole(QPalette::Button);
            tabBar->setForegroundRole(QPalette::ButtonText);
        }
        Anim::Tabs::manage(tabBar);
        tabBar->setAttribute(Qt::WA_Hover);
        tabBar->setAttribute(Qt::WA_MouseTracking);
        installFilter(tabBar);

        if (tabBar->expanding() && !dConf.uno.enabled)
            tabBar->setExpanding(false);

        if (dConf.app == DSP::Settings::Konsole && tabBar->parentWidget() && tabBar->documentMode())
        {
            tabBar->parentWidget()->setObjectName("konsole_tabbar_parent");
            installFilter(tabBar->parentWidget());
            disconnect(Handlers::Window::instance(), SIGNAL(windowDataChanged(QWidget*)), tabBar->parentWidget(), SLOT(update()));
            connect(Handlers::Window::instance(), SIGNAL(windowDataChanged(QWidget*)), tabBar->parentWidget(), SLOT(update()));
        }
    }
    else if (QGroupBox *gb = qobject_cast<QGroupBox *>(widget))
    {
        const quint8 mg(pixelMetric(PM_DefaultFrameWidth, 0, widget));
        const QMargins m(mg, mg, mg, mg);
        if (QLayout *l = gb->layout())
        {
            if (l->contentsMargins() != m)
                l->setContentsMargins(m);
        }
        else if (gb->contentsMargins() != m)
            gb->setContentsMargins(m);
    }
    else if (widget->inherits("KTitleWidget"))
    {
        installFilter(widget);
        QList<QFrame *> children = widget->findChildren<QFrame *>();
        for (int i = 0; i < children.size(); ++i)
        {
            QFrame *f(children.at(i));
            if (f->autoFillBackground())
            {
                f->setAutoFillBackground(false);
                f->setFrameStyle(0);
            }
            if (QLabel *l = qobject_cast<QLabel *>(f))
            {
                l->setAlignment(Qt::AlignCenter);
                installFilter(l);
            }
        }
    }
    else if (widget->inherits("KUrlNavigator"))
    {
        if (widget->parentWidget() && widget->parentWidget()->size() == widget->size())
            widget->parentWidget()->setAutoFillBackground(false); //gwenview kurlnavigator parent seems to be an idiot... oh well
    }
    else if (widget->inherits("KTabWidget"))
    {
        installFilter(widget);
    }
    else if (widget->inherits("KMultiTabBarTab"))
    {
        installFilter(widget);
    }
    else if (widget->inherits("KMultiTabBarInternal"))
    {
        if (QFrame *frame = qobject_cast<QFrame *>(widget))
        {
            frame->setFrameShadow(QFrame::Sunken);
            frame->setFrameShape(QFrame::StyledPanel);
            installFilter(frame);
            frame->setContentsMargins(0, 0, 0, 0);
        }
    }
    else if (widget->inherits("QTipLabel")) //tooltip
    {
        if (!widget->testAttribute(Qt::WA_TranslucentBackground))
            applyTranslucency(widget);

        ShadowHandler::manage(widget);
        if (dConf.balloonTips)
            Handlers::BalloonHelper::manage(widget);
    }

    //this needs to be here at the end cause I might alter the frames before in the main if segment
    if (dConf.uno.enabled && qobject_cast<QFrame *>(widget) && Overlay::isSupported(static_cast<QFrame *>(widget)))
        Overlay::manage(static_cast<QFrame *>(widget), dConf.shadows.opacity);
}

void
Style::unpolish(QWidget *widget)
{
    if (!widget)
        return;
    widget->disconnect(this);
    widget->removeEventFilter(this);
    ShadowHandler::release(widget);
    Anim::Basic::release(widget);
    Handlers::Window::release(widget);
    if (QProgressBar *pb = qobject_cast<QProgressBar *>(widget))
        ProgressHandler::release(pb);
    else if (QTabBar *tb = qobject_cast<QTabBar *>(widget))
        Anim::Tabs::release(tb);
    else if (QToolButton *tb = qobject_cast<QToolButton *>(widget))
        Anim::ToolBtns::release(tb);
    else if (QFrame *f = qobject_cast<QFrame *>(widget))
        Overlay::release(f);
#if HASDBUS
    else if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(widget))
        BE::MacMenu::release(menuBar);
#endif
    QCommonStyle::unpolish(widget);
}

void
Style::polish(QPalette &p)
{
    QCommonStyle::polish(p);
    if (dConf.palette)
        p = *dConf.palette;
    GFX::generateData();
}

void
Style::polish(QApplication *app)
{
    QCommonStyle::polish(app);
//    if (app && dConf.palette)
//        app->setPalette(*dConf.palette);
}
