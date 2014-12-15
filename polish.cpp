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

#include "styleproject.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/shadowhandler.h"
#include "stylelib/progresshandler.h"
#include "stylelib/animhandler.h"
#include "stylelib/handlers.h"
#include "stylelib/xhandler.h"
#include "stylelib/settings.h"
#include "stylelib/color.h"

/* Thomas gave me his blessing to use
 * his macmenu! yeah! so now we get
 * macmenues in styleproject!
 */
#if !defined(QT_NO_DBUS)
#include "macmenu.h"
#endif


/* a non-const environment that is called
 * for for every widget directly after its
 * creation, one can manipulate them in
 * pretty much any way here.
 */

#define installFilter(_VAR_) { _VAR_->removeEventFilter(this); _VAR_->installEventFilter(this); }

void
StyleProject::polish(QWidget *widget)
{
    if (!widget)
        return;

#if 0
    if (widget->parentWidget())
        qDebug() << widget << widget->parentWidget();

    installFilter(widget);
#endif

    if (dConf.splitterExt && (widget->objectName() == "qt_qmainwindow_extended_splitter" || qobject_cast<QSplitterHandle *>(widget)))
        SplitterExt::manage(widget);

    if (qobject_cast<QPushButton *>(widget) ||
            qobject_cast<QCheckBox *>(widget) ||
            qobject_cast<QRadioButton *>(widget) ||
            qobject_cast<QSlider *>(widget) ||
            qobject_cast<QScrollBar *>(widget))
        Anim::Basic::manage(widget);
    if (/*dConf.uno.enabled &&*/
            (qobject_cast<QMainWindow *>(widget) ||
            widget->findChild<QToolBar *>() ||
            widget->findChild<QTabBar *>()))
        Handlers::Window::manage(widget);

    if (dConf.uno.contAware && qobject_cast<QMainWindow *>(widget->window()) && qobject_cast<QAbstractScrollArea *>(widget))
        Handlers::ScrollWatcher::watch(static_cast<QAbstractScrollArea *>(widget));

    if (Handlers::Drag::canDrag(widget))
        Handlers::Drag::manage(widget);

    if (dConf.app == Settings::Konversation && qobject_cast<QMainWindow *>(widget->window()))
    {
//        if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget))
//        {
//            edit->setFrameStyle(QFrame::StyledPanel|QFrame::Sunken);
//            edit->viewport()->setAutoFillBackground(true);
//        }
        if (qobject_cast<QHBoxLayout *>(widget->layout())||qobject_cast<QVBoxLayout *>(widget->layout()))
            if (QBoxLayout *l = static_cast<QBoxLayout *>(widget->layout()))
                l->setSpacing(0);
    }

    if (widget->isWindow()) //this segment needs serious cleaning still...
    {
        if (dConf.compactMenu && widget->findChild<QMenuBar *>())
            Handlers::Window::addCompactMenu(widget);

        const bool mainWin(qobject_cast<QMainWindow *>(widget));
        if (mainWin && !dConf.uno.enabled)
            widget->setContentsMargins(4, 0, 4, 4);
        const bool hasTitle(!(widget->windowFlags() & Qt::FramelessWindowHint)||widget->property("DSP_hasbuttons").toBool());
        bool needTrans(hasTitle&&(dConf.uno.enabled?mainWin:true));
        if (needTrans)
            needTrans = !(widget->windowType() == Qt::Desktop
                    || widget->testAttribute(Qt::WA_X11NetWmWindowTypeDesktop)
                    || widget->testAttribute(Qt::WA_TranslucentBackground)
                    || widget->testAttribute(Qt::WA_NoSystemBackground)
                    || widget->testAttribute(Qt::WA_OpaquePaintEvent));
        if (XHandler::opacity() < 1.0f && needTrans)
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
            QTimer::singleShot(0, widget, SLOT(update()));
            unsigned int d(0);
            XHandler::setXProperty<unsigned int>(widget->winId(), XHandler::KwinBlur, XHandler::Long, &d);
        }
        bool needShadows(false);

        if (widget->windowType() == Qt::ToolTip && widget->inherits("QTipLabel"))
        {
            if (!widget->testAttribute(Qt::WA_TranslucentBackground))
                widget->setAttribute(Qt::WA_TranslucentBackground);
            needShadows = true;
        }

        if (dConf.hackDialogs && qobject_cast<QDialog *>(widget))
        {
            Handlers::Window::manage(widget);
            needShadows = true;
        }

        if (qobject_cast<QMenu *>(widget)
                || qobject_cast<QDockWidget *>(widget)
                || qobject_cast<QToolBar *>(widget)
                || hasTitle)
            needShadows = true;

        if (needShadows)
            ShadowHandler::manage(widget);
    }

    //main if segment for all widgets
    if (QToolBar *bar = qobject_cast<QToolBar *>(widget))
    {
        Handlers::ToolBar::manage(bar);
        bar->setForegroundRole(QPalette::WindowText);
        bar->setBackgroundRole(QPalette::Window);
    }
    else if (QToolButton *btn = qobject_cast<QToolButton *>(widget))
    {
        Anim::ToolBtns::manage(btn);
        Handlers::ToolBar::manage(btn);
        if (!qobject_cast<QToolBar *>(btn->parentWidget()))
        {
            btn->setBackgroundRole(QPalette::Window);
            btn->setForegroundRole(QPalette::WindowText);
        }
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
    else if (QAbstractItemView *view = qobject_cast<QAbstractItemView *>(widget))
    {
        view->viewport()->setAttribute(Qt::WA_Hover);
        view->setAttribute(Qt::WA_MouseTracking);
        installFilter(view);

        //some views paint w/ button background on some items w/o setting text color (ktelepathy contacts list for example, idiot gay....)
        QPalette pal(widget->palette());
        const bool dark(Color::luminosity(pal.color(QPalette::Base)) < Color::luminosity(pal.color(QPalette::Text)));
        QColor (QColor::*function)(int) const = dark?&QColor::lighter:&QColor::darker;
        pal.setColor(QPalette::Button, (pal.color(QPalette::Base).*function)(120));
        pal.setColor(QPalette::ButtonText, pal.color(QPalette::Text));
        widget->setPalette(pal);
    }
    else if (qobject_cast<QStatusBar *>(widget))
    {
        widget->setForegroundRole(QPalette::WindowText);
        widget->setBackgroundRole(QPalette::Window);
    }
    else if (qobject_cast<QMenu *>(widget)||qobject_cast<QMenuBar *>(widget))
    {
        widget->setMouseTracking(true);
        widget->setAttribute(Qt::WA_Hover);
        if (qobject_cast<QMenu *>(widget))
        {
            if (!widget->testAttribute(Qt::WA_TranslucentBackground))
                widget->setAttribute(Qt::WA_TranslucentBackground);
            widget->setForegroundRole(QPalette::Text);
            widget->setBackgroundRole(QPalette::Base);
        }
        else
        {
            widget->setForegroundRole(QPalette::WindowText);
            widget->setBackgroundRole(QPalette::Window);
        }
        /**
         * Apparently some menues fight back and dont
         * want mousetracking, we filter and set mousetracking
         * when the menu is shown.
         */
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

        if (!qApp->applicationName().compare("konsole", Qt::CaseInsensitive))
        {
            if (safari) //hmmm
            {
                QWidget *p(tabBar->parentWidget());
                if (p)
                {
                    p->setObjectName("konsole_tabbar_parent");
                    installFilter(p);
                }
            }
        }
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
        if (QFrame *frame = qobject_cast<QFrame *>(widget->parentWidget()))
        {
            frame->setFrameShadow(QFrame::Sunken);
            frame->setFrameShape(QFrame::StyledPanel);
            OverLay::manage(frame, 255.0f*dConf.shadows.opacity);
            installFilter(frame);
            frame->setContentsMargins(0, 0, 0, 0);
        }
    }

    if (dConf.uno.enabled)
    if (QFrame *frame = qobject_cast<QFrame *>(widget))
    {
        if (frame->frameStyle() == QFrame::Sunken|QFrame::StyledPanel
                && qobject_cast<QMainWindow *>(frame->window()))
        {
            QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(frame);
            if (!area || area->viewport()->autoFillBackground())
                OverLay::manage(frame, dConf.shadows.opacity*255.0f);
        }
    }

#if !defined(QT_NO_DBUS)
    if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(widget))
        Bespin::MacMenu::manage(menuBar);
#endif
    QCommonStyle::polish(widget);
}

void
StyleProject::unpolish(QWidget *widget)
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
    else if (QFrame *f = qobject_cast<QFrame *>(widget))
        OverLay::release(f);
    else if (QToolButton *tb = qobject_cast<QToolButton *>(widget))
        Anim::ToolBtns::release(tb);
#if !defined(QT_NO_DBUS)
    else if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(widget))
        Bespin::MacMenu::release(menuBar);
#endif
    QCommonStyle::unpolish(widget);
}

void
StyleProject::hackLayout(QWidget *w)
{
    if (qobject_cast<QTabWidget *>(w) || qobject_cast<QSplitter *>(w))
        return;
    QBoxLayout *l(static_cast<QBoxLayout *>(w->layout()));

    const QList<QWidget *> kids(w->findChildren<QWidget *>());
    QList<const QWidget *> clickable;
    for (int i = 0; i < kids.count(); ++i)
    {
        const QWidget *kid = kids.at(i);
        if (kid->parentWidget() != w && !kid->isVisibleTo(w))
            continue;
        const bool isClickable = qobject_cast<const QAbstractButton *>(kid) ||
                qobject_cast<const QComboBox *>(kid) ||
                qobject_cast<const QAbstractSlider *>(kid) ||
//                qobject_cast<const QGroupBox *>(kid) ||
                qobject_cast<const QLineEdit *>(kid) ||
                qobject_cast<const QProgressBar *>(kid) ||
//                qobject_cast<const QLabel *>(kid) ||
//                qobject_cast<const QTabWidget *>(kid) ||
                kid->inherits("KTitleWidget")
                ; //widget w/ possible clickables...
        if (isClickable)
            clickable << kid;
//        if (hasClickables) //one is enough
//            break;
    }
    if (!clickable.isEmpty())
    {
//        qDebug() << "adjusting layout" << l << l->spacing() << l->contentsMargins() << w->contentsMargins();
        const int m(pixelMetric(PM_DefaultLayoutSpacing));
        l->setSpacing(m);
        l->setContentsMargins(m, m, m, m);
    }
}
