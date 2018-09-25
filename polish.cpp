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
#include "stylelib/titlewidget.h"

#include <QStackedWidget>
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
#include <QCalendarWidget>
#include <QTableView>

#include <KWindowInfo>
#include <KWindowSystem>
//#include <QWebView>

/* Thomas gave me his blessing to use
 * his macmenu! yeah! so now we get
 * macmenues in styleproject!
 */
#include "defines.h"
#if HASDBUS
#include "stylelib/macmenu.h"
#endif

#if QT_VERSION >= 0x050000
#include <QWindow>
#endif
using namespace DSP;

static void applyBlur(QWidget *widget)
{
    if (dConf.app == Settings::Konsole
            || dConf.app == Settings::Yakuake)
        return;
    unsigned int d(0);
    XHandler::setXProperty<unsigned int>(widget->winId(), XHandler::_KDE_NET_WM_BLUR_BEHIND_REGION, XHandler::Long, &d);
}

void
Style::applyTranslucency(QWidget *widget)
{
    if (!widget->isWindow()
            || widget->testAttribute(Qt::WA_WState_Created)
            || dConf.app == Settings::Konsole
            || dConf.app == Settings::Yakuake)
        return;

    const QIcon icn = widget->windowIcon();
    const bool wasVisible= widget->isVisible();
    const bool wasMoved = widget->testAttribute(Qt::WA_Moved);
    if (wasVisible)
        widget->hide();
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setAttribute(Qt::WA_NoSystemBackground);
//#if QT_VERSION >= 0x050000
//    if (widget->testAttribute(Qt::WA_WState_Created))
//    if (QWindow *win = widget->windowHandle())
//    {
//        QSurfaceFormat format = win->format();
//        if (format.alphaBufferSize() != 8)
//        {
//            qDebug() << "DSP: Warning! applying translucency to already created window" << widget;
//            format.setAlphaBufferSize(8);
//            win->destroy(); //apparently if we destroy and then set format we actually get a 32 bit window... this however messes w/ QWidget::winId()
//            win->setFormat(format);
//            win->create();
//            QEvent winIdChange(QEvent::WinIdChange);
//            QCoreApplication::sendEvent(widget, &winIdChange);
//            QCoreApplication::processEvents();
//        }
//    }
//#endif
    widget->setWindowIcon(icn);
    widget->setAttribute(Qt::WA_Moved, wasMoved); // https://bugreports.qt-project.org/browse/QTBUG-34108
    widget->setVisible(wasVisible);
    applyBlur(widget);
}

void
Style::polish(QWidget *widget)
{
    if (!widget)
        return;

    QCommonStyle::polish(widget);

#if DEBUG
//    if (widget->parentWidget())
//        qDebug() << "polishing widget:" << widget << "with parentWidget:" << widget->parentWidget();

    installFilter(widget);
#endif
    if (qobject_cast<Handlers::Balloon *>(widget)
            || qobject_cast<SplitterExt *>(widget)
            || qobject_cast<TitleWidget *>(widget))
        return;
    if (widget->inherits("QWebView")
            || widget->inherits("QWebPage")
            || widget->inherits("QWebFrame"))
        return;

    //some special handling
    if (dConf.splitterExt
            && (widget->objectName() == "qt_qmainwindow_extended_splitter"
                || qobject_cast<QSplitterHandle *>(widget)))
        SplitterExt::manage(widget);

    if (dConf.deco.embed && dConf.uno.enabled
            && qobject_cast<QToolBar *>(widget)
            && qobject_cast<QMainWindow *>(widget->parentWidget())
            && !widget->parentWidget()->parentWidget()
            && widget->styleSheet().isEmpty())
    {
        QToolBar *tb = static_cast<QToolBar *>(widget);
        tb->setMovable(true);
        if (tb->isMovable())
            TitleWidget::manage(static_cast<QToolBar *>(widget));
    }

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
//#if 0
//#if QT_VERSION < 0x050000
//            if (XHandler::opacity() < 0xff /*&& !widget->testAttribute(Qt::WA_TranslucentBackground)*/)
//                applyTranslucency(widget);
//#endif
//#endif
            if (dConf.lockDocks)
                Handlers::Dock::manage(widget);
            Handlers::Window::manage(widget);
            if (dConf.removeTitleBars)
                ShadowHandler::manage(widget);

#if 0
            if (!dConf.uno.enabled)
            {
                QMainWindow *win = static_cast<QMainWindow *>(widget);
                QWidget *center = win->centralWidget();
                if (center && !qobject_cast<QTabWidget *>(center) && center->layout())
                {
                    center->setProperty("DSP_center", true);
                    installFilter(center);
                    installFilter(win);
                    QPalette pal(center->palette());

                    const QColor win(pal.color(QPalette::Window));
                    const QColor wintxt(pal.color(QPalette::WindowText));
                    pal.setColor(QPalette::Window, wintxt);
                    pal.setColor(QPalette::WindowText, win);

                    const QColor base(pal.color(QPalette::Base));
                    const QColor text(pal.color(QPalette::Text));
                    pal.setColor(QPalette::Base, text);
                    pal.setColor(QPalette::AlternateBase, text);
                    pal.setColor(QPalette::Text, base);

                    center->setPalette(pal);
                    static const int m(8);
                    center->setContentsMargins(m,m,m,m);
                    QList<QAbstractScrollArea *> areas = center->findChildren<QAbstractScrollArea *>();
                    for (int i = 0; i < areas.size(); ++i)
                    {
                        QAbstractScrollArea *area = areas.at(i);
                        area->viewport()->setAutoFillBackground(true);
                        area->viewport()->setBackgroundRole(QPalette::Base);
                        area->viewport()->setForegroundRole(QPalette::Text);
                    }
                }
            }
#endif
        }
        else if (qobject_cast<QDialog *>(widget))
        {
//#if 0
//#if QT_VERSION < 0x050000
//            if (XHandler::opacity() < 0xff)
//            {
//                if (!dConf.uno.enabled)
//                    applyTranslucency(widget);
//                applyBlur(widget);
//            }
//#endif
//#endif
            Handlers::Window::manage(widget);
        }
        const bool isFrameLessMainWindow(((widget->windowFlags() & Qt::FramelessWindowHint)
                                          && widget->windowRole().startsWith("MainWindow")));
        if (widget->windowType() == Qt::Popup
                || widget->windowType() == Qt::ToolTip
                || widget->windowType() == Qt::Dialog
                || widget->windowType() == Qt::Tool
                || widget->inherits("BE::Panel")
                || isFrameLessMainWindow)
            ShadowHandler::manage(widget);
        if (widget->testAttribute(Qt::WA_TranslucentBackground)
                || (qobject_cast<QDialog *>(widget) && !(widget->windowFlags() & Qt::FramelessWindowHint)))
            applyBlur(widget);
        if (dConf.app == Settings::Konversation
                && qobject_cast<QMenu *>(widget)
                && !(widget->windowFlags() & Qt::FramelessWindowHint))
            widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
    }
    //main if segment for all widgets
    if (QToolBar *bar = qobject_cast<QToolBar *>(widget))
    {
        bar->setForegroundRole(QPalette::WindowText);
        bar->setBackgroundRole(QPalette::Window);
//        if (!TitleWidget::isManaging(bar))
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
        cBox->setMaximumHeight(dConf.baseSize+GFX::shadowMargin(dConf.input.shadow)*2);
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

//        if (area->viewport()->autoFillBackground())
//        {
//            const quint8 mg(pixelMetric(PM_DefaultFrameWidth, 0, widget));
//            const QMargins m(mg, mg, mg, mg);

//            area->setContentsMargins(m);
//            area->setAutoFillBackground(false); //area is autofilling? why?
//            area->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);

//        }
        if (area->viewport()->autoFillBackground() && dConf.views.opacity != 0xff)
        {
            QPalette pal = area->palette();
            QColor base = pal.color(QPalette::Base);
            QColor altbase = pal.color(QPalette::AlternateBase);
            base.setAlpha(dConf.views.opacity);
            altbase.setAlpha(0);
            pal.setColor(QPalette::Base, base);
            pal.setColor(QPalette::AlternateBase, altbase);
            area->setPalette(pal);
            if (qobject_cast<QAbstractItemView *>(area))
                static_cast<QAbstractItemView *>(area)->setAlternatingRowColors(false);
        }

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
            const bool dark(Color::lum(pal.color(QPalette::Base)) < Color::lum(pal.color(QPalette::Text)));
            QColor (QColor::*function)(int) const = dark?&QColor::lighter:&QColor::darker;
            pal.setColor(QPalette::Button, (pal.color(QPalette::Base).*function)(120));
            pal.setColor(QPalette::ButtonText, pal.color(QPalette::Text));
            widget->setPalette(pal);

            if (view->inherits("KFilePlacesView"))
            {
                view->viewport()->setBackgroundRole(QPalette::Window);
                view->viewport()->setForegroundRole(QPalette::WindowText);
                view->viewport()->setAutoFillBackground(false);
            }
        }
        if (dConf.views.traditional)
        {
            QList<QHeaderView *> headers = area->findChildren<QHeaderView *>();
            for (int i = 0; i < headers.count(); ++i)
            {
                QHeaderView *header = headers.at(i);
                QPalette palette = header->palette();
                palette.setColor(QPalette::Window, palette.color(QPalette::Base));
                palette.setColor(QPalette::WindowText, palette.color(QPalette::Text));
                palette.setColor(QPalette::Button, palette.color(QPalette::Base));
                palette.setColor(QPalette::ButtonText, palette.color(QPalette::Text));
                header->setPalette(palette);
                header->setAutoFillBackground(true);
                header->viewport()->setAutoFillBackground(true);
                header->viewport()->setPalette(palette); //could we maybe believe at this point that we want base colored headers...
            }
        }
    }
    else if (qobject_cast<QStatusBar *>(widget))
    {
        widget->setForegroundRole(QPalette::WindowText);
        widget->setBackgroundRole(QPalette::Window);
    }
    else if (qobject_cast<QMenu *>(widget))
    {
//        qDebug() << widget->windowType();
//        if (!widget->parentWidget())
//        {
//            if (!widget->testAttribute(Qt::WA_TranslucentBackground))
//                applyTranslucency(widget);
//        }
        widget->setMouseTracking(true);
        widget->setAttribute(Qt::WA_Hover);
        widget->setForegroundRole(QPalette::Text);
        widget->setBackgroundRole(QPalette::Base);
        installFilter(widget);
        ShadowHandler::manage(widget);
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
        polishLater(tabBar);
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
    else if (QTabWidget *qtw = qobject_cast<QTabWidget *>(widget))
    {
        installFilter(qtw);
    }
    else if (QFrame *frame = qobject_cast<QFrame *>(widget))
    {
//        if (qobject_cast<QStackedWidget *>(frame))
//        {
//            frame->setFrameShadow(QFrame::Sunken);
//            frame->setFrameShape(QFrame::StyledPanel);
//        }
        if (frame->inherits("KMultiTabBarInternal"))
        {
            frame->setContentsMargins(4,4,4,4);
            installFilter(frame);

            frame->setFrameShadow(QFrame::Sunken);
            frame->setFrameShape(QFrame::StyledPanel);
            frame->setAutoFillBackground(true);
            QPalette pal = frame->palette();
            QColor macBg = Color::mid(pal.color(QPalette::Window), pal.color(QPalette::Highlight), 10, 1);
            macBg = Color::mid(macBg, pal.color(QPalette::WindowText), 10, 1);
            pal.setColor(QPalette::Window, macBg);
            frame->setPalette(pal);
//            frame->setBackgroundRole(QPalette::WindowText);
//            frame->setForegroundRole(QPalette::Window);
            Overlay::manage(frame, dConf.shadows.opacity);
        }
        else if (widget->inherits("QTipLabel")) //tooltip
        {
            if (!widget->testAttribute(Qt::WA_TranslucentBackground))
                applyTranslucency(widget);

            ShadowHandler::manage(widget);
            if (dConf.balloonTips)
                Handlers::BalloonHelper::manage(widget);
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
    else if (widget->inherits("KTextEditor::ViewPrivate"))
    {
        if (dConf.uno.enabled && dConf.uno.overlay)
            Overlay::manage(widget, dConf.shadows.opacity);
    }
    else if (widget->inherits("NavigationBar"))
    {
        widget->setContentsMargins(2,2,2,2);
    }
    else if (widget->inherits("KateTabButton")) //KateTabButton(0x1dcea80) KateTabBar(0x1ccee10)
    {
//        widget->setMaximumWidth(200); //cant, kate uses its own layouting, not a QLayout...
        installFilter(widget);
    }
    else if (widget->inherits("KateTabBar"))
    {
        widget->setFixedHeight(dConf.baseSize+TabBarBottomSize);
        installFilter(widget);
    }
    //this needs to be here at the end cause I might alter the frames before in the main if segment
    if (dConf.uno.enabled && dConf.uno.overlay && qobject_cast<QFrame *>(widget))
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
    if (app && dConf.palette)
        app->setPalette(*dConf.palette);
    else if (app)
    {
        QPalette p = app->palette();
        for (int i = 0; i <= QPalette::Background; ++i)
            p.setColor(QPalette::Inactive, QPalette::ColorRole(i), p.color(QPalette::Active, QPalette::ColorRole(i)));
        qApp->setPalette(p);
    }
    GFX::generateData();
}

void
Style::polishSlot(qulonglong w)
{
    if (QTabBar *tabBar = Ops::getChild<QTabBar *>(w))
    {
        bool docMode = tabBar->documentMode();
        QTabWidget *tw(0);
        if (tw = qobject_cast<QTabWidget *>(tabBar->parentWidget()))
            docMode = tw->documentMode();
        if (docMode)
        {
            tabBar->setDrawBase(true);
            if (dConf.tabs.invDoc && !dConf.uno.enabled && tabBar->shape() == QTabBar::RoundedNorth || tabBar->shape() == QTabBar::TriangularNorth)
                tabBar->setShape(QTabBar::RoundedSouth);
        }
        const bool safari = Ops::isSafariTabBar(tabBar);
        if (!safari && tabBar->expanding())
            tabBar->setExpanding(false);
        if (safari || docMode)
        {
            QPalette pal(tabBar->palette());
            const QColor fg(pal.color(QPalette::WindowText));
            const QColor bg(pal.color(QPalette::Window));
            pal.setColor(QPalette::Button, bg);
            pal.setColor(QPalette::ButtonText, fg);
            pal.setColor(tabBar->backgroundRole(), bg);
            pal.setColor(tabBar->foregroundRole(), fg);
            tabBar->setPalette(pal);
        }
        else
        {
            QPalette pal(tabBar->palette());
            pal.setColor(QPalette::Window, pal.color(QPalette::Button));
            pal.setColor(QPalette::WindowText, pal.color(QPalette::ButtonText));
            tabBar->setPalette(pal);
        }
        Anim::Tabs::manage(tabBar);
        tabBar->setAttribute(Qt::WA_Hover);
        tabBar->setAttribute(Qt::WA_MouseTracking);
        if (dConf.uno.enabled) //we need to ajust UNO height data on hide/show...
            installFilter(tabBar);

        if (tabBar->expanding() && !dConf.uno.enabled)
            tabBar->setExpanding(false);

        if (dConf.app == DSP::Settings::Konsole && tabBar->parentWidget() && tabBar->documentMode())
        {
            tabBar->parentWidget()->setProperty("DSP_konsoleTabBarParent", true);
            installFilter(tabBar->parentWidget());
            disconnect(Handlers::Window::instance(), SIGNAL(windowDataChanged(QWidget*)), tabBar->parentWidget(), SLOT(update()));
            connect(Handlers::Window::instance(), SIGNAL(windowDataChanged(QWidget*)), tabBar->parentWidget(), SLOT(update()));
        }
    }
}
