#include "windowhelpers.h"
#include "windowdata.h"
#include "color.h"
#include "ops.h"
#include "macmenu.h"
#include "xhandler.h"
#include "fx.h"
#include <QMainWindow>
#include <QToolBar>
#include <QMenuBar>
#include <QImage>
#include <QPainter>

#if HASDBUS
#include <QDBusInterface>
#endif

using namespace DSP;

WindowHelpers *WindowHelpers::s_instance(0);

WindowHelpers
*WindowHelpers::instance()
{
    if (!s_instance)
        s_instance = new WindowHelpers();
    return s_instance;
}

WindowHelpers::WindowHelpers(QObject *parent)
    : QObject(parent)
{
#if HASDBUS
    static const QString service("org.kde.dsp.kwindeco"),
            interface("org.kde.dsp.deco"),
            path("/DSPDecoAdaptor");
    QDBusInterface *iface = new QDBusInterface(service, path, interface);
    iface->connection().connect(service, path, interface, "dataChanged", this, SLOT(dataChanged(QDBusMessage)));
#endif
}

bool
WindowHelpers::inUno(const QWidget *w)
{
    return w&&w->mapTo(w->window(), QPoint()).y() < unoHeight(w->window());
}

bool
WindowHelpers::scheduleWindow(const qulonglong w)
{
    if (instance()->m_scheduled.contains(w))
        return false;
    instance()->m_scheduled << w;
    return true;
}

void
WindowHelpers::updateWindowDataLater(QWidget *win)
{
    const qulonglong window = (qulonglong)win;
    if (scheduleWindow(window))
        QMetaObject::invokeMethod(instance(), "updateWindowData", Qt::QueuedConnection, Q_ARG(qulonglong, window));
}

void
WindowHelpers::updateWindowData(qulonglong window)
{
    m_scheduled.removeOne(window);
    QWidget *win = Ops::getChild<QWidget *>(window);
    if (!win || !win->isWindow())
        return;

    WindowData *data = WindowData::memory(win->winId(), win, true);
    if (!data)
        return;

    QPalette pal(win->palette());
    if (!Color::contrast(pal.color(win->backgroundRole()), pal.color(win->foregroundRole()))) //im looking at you spotify
        pal = QApplication::palette();
    if (dConf.uno.enabled)
    {
        bool separator(true);
        const unsigned int height = getHeadHeight(win, separator);
        if (dConf.differentInactive)
        {
            if (isActiveWindow(win))
            {
                pal.setColor(QPalette::Active, win->backgroundRole(), Color::mid(pal.color(win->backgroundRole()), Qt::black, 10, 1));
                pal.setColor(QPalette::Active, win->foregroundRole(), Color::mid(pal.color(win->foregroundRole()), Qt::black, 10, 1));
                pal.setCurrentColorGroup(QPalette::Active);
            }
            else
            {
                pal.setColor(QPalette::Inactive, win->backgroundRole(), Color::mid(pal.color(win->backgroundRole()), Qt::white, 20, 1));
                pal.setColor(QPalette::Inactive, win->foregroundRole(), Color::mid(pal.color(win->foregroundRole()), Qt::white, 20, 1));
                pal.setCurrentColorGroup(QPalette::Inactive);
            }
        }
        int width(0);
        if (data->lock())
        {
            unoBg(win, width, height, pal, data->imageData());
            data->setImageSize(width, height);
            data->unlock();
        }
        data->setValue<bool>(WindowData::Separator, separator);
        data->setValue<int>(WindowData::UnoHeight, height);
    }
    else
    {
//        const int th = data->value<int>(WindowData::TitleHeight, 0);
        if (data->lock())
        {
            QImage img(data->imageData(), GFX::noise(true).width(), GFX::noise(true).height(), QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::transparent);
            QPainter p(&img);
            p.drawTiledPixmap(img.rect(), GFX::noise(true));
            p.end();
            data->setImageSize(img.width(), img.height());
            data->unlock();
        }
        data->setWindowGradient(dConf.windows.gradient);
        data->setValue<bool>(WindowData::Separator, false);
    }
    data->setValue<bool>(WindowData::WindowIcon, dConf.deco.icon);
    data->setValue<bool>(WindowData::ContAware, dConf.uno.enabled&&dConf.uno.contAware);
    data->setValue<bool>(WindowData::Uno, dConf.uno.enabled);
    data->setValue<bool>(WindowData::Horizontal, dConf.uno.enabled?dConf.uno.hor:dConf.windows.hor);
    data->setValue<int>(WindowData::Opacity, XHandler::opacity()*255.0f);
    data->setValue<int>(WindowData::Buttons, dConf.deco.buttons);
    data->setValue<int>(WindowData::Frame, dConf.deco.frameSize);
    data->setValue<int>(WindowData::ShadowOpacity, dConf.shadows.opacity);
    data->setValue<int>(WindowData::FollowDecoShadow, dConf.toolbtn.shadow);
    data->setCloseColor(QColor::fromRgba(dConf.deco.close));
    data->setMaxColor(QColor::fromRgba(dConf.deco.max));
    data->setMinColor(QColor::fromRgba(dConf.deco.min));
    data->setFg(pal.color(win->foregroundRole()));
    data->setBg(pal.color(win->backgroundRole()));
    data->setButtonGradient(dConf.toolbtn.gradient);
    win->update();
//    if (TitleWidget *w = win->findChild<TitleWidget *>())
//        w->update();
    emit instance()->windowDataChanged(win);
    data->sync();
}


unsigned int
WindowHelpers::getHeadHeight(QWidget *win, bool &separator)
{
    WindowData *d = WindowData::memory(win->winId(), win); //guaranteed by caller method
    int h = d->value<int>(WindowData::TitleHeight);
    if (!h)
        h = 25;
    if (!h)
    {
        separator = false;
        return 0;
    }
    if (!dConf.uno.enabled)
    {
        separator = false;
        if (h)
            return h;
        return 0;
    }
    int height(0);
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(win))
    {
        if (QMenuBar *menuBar = mw->findChild<QMenuBar *>())
#if HASDBUS
            if (!BE::MacMenu::isActive() && !BE::MacMenu::manages(menuBar))
#endif
        {
            if (menuBar->isVisible())
                height += menuBar->height();
        }

        QList<QToolBar *> tbs(mw->findChildren<QToolBar *>());
        for (int i = 0; i<tbs.count(); ++i)
        {
            QToolBar *tb(tbs.at(i));
            if (!tb->isFloating() && tb->isVisible())
                if (!tb->findChild<QTabBar *>()) //ah eiskalt...
                    if (mw->toolBarArea(tb) == Qt::TopToolBarArea)
                    {
                        if (tb->geometry().bottom() > height)
                            height = tb->geometry().bottom()+1;
                        separator = false;
                    }
        }
    }
    QList<QTabBar *> tabbars = win->findChildren<QTabBar *>();
    for (int i = 0; i < tabbars.count(); ++i)
    {
        const QTabBar *tb(tabbars.at(i));
        if (tb && tb->isVisible() && Ops::isUnoTabBar(tb))
        {
            separator = false;
            const int y(tb->mapTo(win, tb->rect().bottomLeft()).y()+1);
            if (y > height)
                height = y;
        }
    }
    instance()->m_uno.insert(win, height);
    return height+h;
}

int
WindowHelpers::unoHeight(QWidget *win, bool includeClientPart, bool includeTitleBar)
{
    int height(0);
    if (includeClientPart)
        height += instance()->m_uno.value(win, 0);
    if (!includeTitleBar)
        return height;
    WindowData *data = WindowData::memory(win->winId(), win);
    if (data)
        return height + data->value<int>(WindowData::TitleHeight);
    return height;
}

static const char *s_active("DSP_activeWindow");

bool
WindowHelpers::isActiveWindow(const QWidget *w)
{
    if (!w)
        return true;
    QWidget *win = w->window();
    if (!win->property(s_active).isValid())
        return true;
    return (win->property(s_active).toBool());
}

#if HASDBUS
void
WindowHelpers::dataChanged(QDBusMessage msg)
{
    uint winId = msg.arguments().at(0).toUInt();
    QList<QWidget *> widgets = qApp->allWidgets();
    for (int i = 0; i < widgets.count(); ++i)
    {
        QWidget *w(widgets.at(i));
        if (w->isWindow() && w->winId() == winId)
        {
            WindowData *data = WindowData::memory(winId, w);
            w->setProperty(s_active, data->value<bool>(WindowData::IsActiveWindow));
            updateWindowDataLater(w);
            break;
        }
    }
}
#endif

void
WindowHelpers::unoBg(QWidget *win, int &w, int h, const QPalette &pal, uchar *data)
{
    if (!data || !win || !h)
        return;
    const bool hor(dConf.uno.hor);
    QColor bc(pal.color(win->backgroundRole()));
    if (dConf.uno.tint.first.isValid())
        bc = Color::mid(bc, dConf.uno.tint.first, 100-dConf.uno.tint.second, dConf.uno.tint.second);
    QBrush b(bc);
    if (!dConf.uno.gradient.isEmpty())
    {
        QLinearGradient lg(0, 0, hor?win->width():0, hor?0:h);
        if (dConf.differentInactive && !isActiveWindow(win))
            lg.setStops(QGradientStops()
                        << DSP::Settings::pairToStop(DSP::GradientStop(0.0f, 0), bc)
                        << DSP::Settings::pairToStop(DSP::GradientStop(1.0f, 0), bc));
        else
            lg.setStops(DSP::Settings::gradientStops(dConf.uno.gradient, bc));
        b = QBrush(lg);
    }
    const unsigned int n(dConf.uno.noise);
    w = (hor?win->width():(n?GFX::noise().width():1));
    QImage img(data, w, h, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter pt(&img);
    pt.fillRect(img.rect(), b);
    if (n)
    {
        const QPixmap noise = FX::mid(GFX::noise(), b, n, 100-n, img.size());
//        QPixmap noise(GFX::noise().size());
//        noise.fill(Qt::transparent);
//        QPainter ptt(&noise);
//        ptt.drawTiledPixmap(noise.rect(), GFX::noise());
//        ptt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
//        ptt.fillRect(noise.rect(), QColor(0, 0, 0, n*2.55f));
//        ptt.end();
//        pt.setCompositionMode(QPainter::CompositionMode_Overlay);
        pt.drawTiledPixmap(img.rect(), noise);
    }
    pt.end();

    if (XHandler::opacity() < 1.0f)
    {
        QImage copy(img);
        img.fill(Qt::transparent);
        pt.begin(&img);
        pt.drawImage(img.rect(), copy);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pt.fillRect(img.rect(), QColor(0, 0, 0, XHandler::opacity()*255.0f));
        pt.end();
    }
}
