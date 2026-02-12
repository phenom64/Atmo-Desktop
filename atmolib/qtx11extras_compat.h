#ifndef ATMO_QTX11EXTRAS_COMPAT_H
#define ATMO_QTX11EXTRAS_COMPAT_H

#include "../defines.h"

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#else

#include <QDateTime>
#include <QGuiApplication>
#include <QtGui/qguiapplication_platform.h>
#include <QScreen>

#if HASXCB
#include <xcb/xcb.h>
#endif

#if HASX11
#include <X11/Xlib.h>
#endif

class QX11Info
{
public:
    static bool isPlatformX11()
    {
        return QGuiApplication::platformName().contains(QStringLiteral("xcb"), Qt::CaseInsensitive);
    }

#if HASXCB
    static xcb_connection_t *connection()
    {
        if (!qGuiApp || !isPlatformX11())
            return nullptr;
        if (auto *native = qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
            return native->connection();
        return nullptr;
    }
#else
    static void *connection() { return nullptr; }
#endif

#if HASX11
    static Display *display()
    {
        if (!qGuiApp || !isPlatformX11())
            return nullptr;
        if (auto *native = qGuiApp->nativeInterface<QNativeInterface::QX11Application>())
            return native->display();
        return nullptr;
    }
#else
    static void *display() { return nullptr; }
#endif

    static unsigned long appRootWindow()
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

    static unsigned long appTime()
    {
        return static_cast<unsigned long>(QDateTime::currentMSecsSinceEpoch() & 0xffffffffu);
    }

    static int appScreen()
    {
        const QList<QScreen *> screens = QGuiApplication::screens();
        const QScreen *primary = QGuiApplication::primaryScreen();
        const int idx = screens.indexOf(const_cast<QScreen *>(primary));
        return idx >= 0 ? idx : 0;
    }
};

#endif

#endif // ATMO_QTX11EXTRAS_COMPAT_H
