#ifndef XCBATOMS_H
#define XCBATOMS_H

#include <QDebug>
#include <QScopedPointer>
#include <QString>
#include <xcb/xcb_atom.h>
#include <xcb/xproto.h>
#include <QX11Info>
#include <qwindowdefs.h>

namespace Xcb
{

class Atom
{
public:
    Atom(const char *atomName)
    {
        xcb_intern_atom_cookie_t c = xcb_intern_atom(QX11Info::connection(), false, strlen(atomName), qPrintable(atomName));
        m_reply.reset(xcb_intern_atom_reply(QX11Info::connection(), c, 0));
    }
    operator xcb_atom_t() const { return m_reply ? m_reply.data()->atom: 0; }

private:
    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> m_reply;
};

class Property
{
public:
    Property(const char *atomName, const WId window)
    {
        Atom atom(atomName);
        if (atom)
        {
            xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(QX11Info::connection(), false, window, atom, XCB_ATOM_ANY, 0, 0xffffffff);
            m_reply.reset(xcb_get_property_reply(QX11Info::connection(), cookie, 0));
        }
    }
    operator bool () const { return length(); }
    int length() const { return m_reply ? xcb_get_property_value_length(m_reply.data()) : 0; }
    QString toString() const { return value() ? QString::fromLocal8Bit(static_cast<char *>(value()), length()) : QString(); }

protected:
    void *value() const {return m_reply ? xcb_get_property_value(m_reply.data()) : 0; }

private:
    QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter> m_reply;
};

} //namespace Xcb
#endif // XCBATOMS_H
