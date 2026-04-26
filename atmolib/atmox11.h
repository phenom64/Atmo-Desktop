#ifndef ATMOX11_H
#define ATMOX11_H

#include "../defines.h"

#include <QDateTime>
#include <QGuiApplication>
#include <QScreen>
#include <QString>
#include <QtGlobal>

#if HASXCB
#include <QtGui/qguiapplication_platform.h>
#include <xcb/xcb.h>
#endif

#if HASX11
#include <QtGui/qguiapplication_platform.h>
#include <X11/Xlib.h>
#endif

namespace NSE
{
namespace AtmoX11
{

inline bool isAvailable()
{
    return qGuiApp
        && QGuiApplication::platformName().contains(QStringLiteral("xcb"), Qt::CaseInsensitive);
}

#if HASXCB
inline xcb_connection_t *connection()
{
    if (!isAvailable())
        return nullptr;
    if (auto *native = qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
        return native->connection();
    return nullptr;
}
#else
inline void *connection()
{
    return nullptr;
}
#endif

#if HASX11
inline Display *display()
{
    if (!isAvailable())
        return nullptr;
    if (auto *native = qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
        return native->display();
    return nullptr;
}
#else
inline void *display()
{
    return nullptr;
}
#endif

inline unsigned long rootWindow()
{
#if HASXCB
    if (xcb_connection_t *c = connection())
    {
        const xcb_setup_t *setup = xcb_get_setup(c);
        xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
        if (it.data)
            return it.data->root;
    }
#endif
#if HASX11
    if (Display *dpy = display())
        return XDefaultRootWindow(dpy);
#endif
    return 0;
}

inline unsigned long appTime()
{
    return static_cast<unsigned long>(QDateTime::currentMSecsSinceEpoch() & 0xffffffffu);
}

inline int appScreen()
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    const QScreen *primary = QGuiApplication::primaryScreen();
    const int idx = screens.indexOf(const_cast<QScreen *>(primary));
    return idx >= 0 ? idx : 0;
}

}
}

#endif // ATMOX11_H
