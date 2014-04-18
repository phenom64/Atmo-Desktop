#include <QStylePlugin>
#include <QWidget>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QDebug>
#include <qmath.h>
#include <QEvent>
#include <QToolBar>
#include <QAbstractItemView>
#include <QMainWindow>

#include "styleproject.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/xhandler.h"

class ProjectStylePlugin : public QStylePlugin
{
public:
    QStringList keys() const { return QStringList() << "StyleProject"; }
    QStyle *create(const QString &key)
    {
        if (key == "styleproject")
            return new StyleProject;
        return 0;
    }
};

Q_EXPORT_PLUGIN2(StyleProject, ProjectStylePlugin)

StyleProject::StyleProject()
    : QCommonStyle()
{
    init();
    assignMethods();
    Render::generateData();
}

void
StyleProject::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_ce[element] && (this->*m_ce[element])(option, painter, widget)))
        QCommonStyle::drawControl(element, option, painter, widget);
}

void
StyleProject::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_cc[control] && (this->*m_cc[control])(option, painter, widget)))
        QCommonStyle::drawComplexControl(control, option, painter, widget);
}

void
StyleProject::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_pe[element] && (this->*m_pe[element])(option, painter, widget)))
        QCommonStyle::drawPrimitive(element, option, painter, widget);
}

void
StyleProject::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    painter->save();
    // we need to add either hide/show mnemonic, otherwise
    // we are rendering text w/ '&' characters.
    flags |= Qt::TextShowMnemonic; // Qt::TextHideMnemonic
    if (textRole != QPalette::NoRole)
        painter->setPen(pal.color(enabled ? QPalette::Active : QPalette::Disabled, textRole));
    painter->drawText(rect, flags, text);
//    QCommonStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
    painter->restore();
}

void
StyleProject::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    QCommonStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

QPixmap
StyleProject::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
{
    return QCommonStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

QRect
StyleProject::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    switch (cc)
    {
    default: break;
    }
    return QCommonStyle::subControlRect(cc, opt, sc, w);
}

QRect
StyleProject::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const
{
    if (flags == Qt::AlignCenter)
        flags = Qt::AlignLeft|Qt::AlignVCenter;
    QRect ret(pixmap.rect());
    if (flags & (Qt::AlignHCenter|Qt::AlignVCenter))
        ret.moveCenter(r.center());
    if (flags & Qt::AlignLeft)
        ret.moveLeft(r.left());
    if (flags & Qt::AlignRight)
        ret.moveRight(r.right());
    if (flags & Qt::AlignTop)
        ret.moveTop(r.top());
    if (flags & Qt::AlignBottom)
        ret.moveBottom(r.bottom());
    return ret;
}
