#include <QStylePlugin>
#include <QWidget>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QStyleOptionComboBox>
#include <QDebug>
#include <qmath.h>
#include <QEvent>
#include <QToolBar>
#include <QAbstractItemView>
#include <QMainWindow>
#include <QTimer>
#include <QApplication>
#include <QLayout>
#include <QProgressBar>
#include <QRect>

#include "styleproject.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/xhandler.h"
#include "stylelib/color.h"
#include "stylelib/animhandler.h"
#include "stylelib/shadowhandler.h"
#include "stylelib/progresshandler.h"
#include "stylelib/animhandler.h"
#include "stylelib/handlers.h"
#include "stylelib/settings.h"

class ProjectStylePlugin : public QStylePlugin
{
public:
    QStringList keys() const { return QStringList() << "StyleProject"; }
    QStyle *create(const QString &key)
    {
        if (!key.compare("styleproject", Qt::CaseInsensitive))
            return new StyleProject();
        return 0;
    }
};

Q_EXPORT_PLUGIN2(StyleProject, ProjectStylePlugin)

StyleProject::StyleProject() : QCommonStyle()
{
//    Anim::setStyle(this);
    init();
    assignMethods();
    Settings::read();
    Render::generateData();
}

StyleProject::~StyleProject()
{
//    ShadowHandler::deleteInstance();
//    ProgressHandler::deleteInstance();
//    Anim::Basic::deleteInstance();
//    Anim::Tabs::deleteInstance();
//    Render::deleteInstance();
//    Ops::deleteInstance();
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
//    QCommonStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
//    return;
    if (text.isEmpty())
        return;
    painter->save();
    // we need to add either hide/show mnemonic, otherwise
    // we are rendering text w/ '&' characters.
    flags |= Qt::TextHideMnemonic; // Qt::TextHideMnemonicTextShowMnemonic
//    if (painter->fontMetrics().boundingRect(text).width() > rect.width()) //if we have more text then space its pointless to render the center of the text...
//        flags &= ~Qt::AlignHCenter;
    const QPalette::ColorRole bgRole(Ops::opposingRole(textRole));
    if (pal.brush(bgRole).style() == Qt::SolidPattern && !pal.brush(bgRole).gradient())
    if (pal.color(textRole).alpha() == 0xff /*&& pal.color(bgRole).alpha()*/)
    if (textRole != QPalette::NoRole && bgRole != QPalette::NoRole && enabled)
    {
        const bool isDark(Color::luminosity(pal.color(textRole)) > Color::luminosity(pal.color(bgRole)));
        const int rgb(isDark?0:255);
        const QColor bevel(rgb, rgb, rgb, 127);
        painter->setPen(bevel);
        painter->drawText(rect.translated(0, 1), flags, text);
    }

    painter->setPen(pal.color(enabled ? QPalette::Active : QPalette::Disabled, textRole));
    painter->drawText(rect, flags, text);
    painter->restore();
}

void
StyleProject::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
//    QRect r(itemPixmapRect(rect, alignment, pixmap));
//    painter->drawTiledPixmap(r, pixmap);
    QCommonStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

QPixmap
StyleProject::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
{
    return QCommonStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

QPixmap
StyleProject::standardPixmap(StandardPixmap sp, const QStyleOption *opt, const QWidget *widget) const
{
    const int size(16);
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::NoPen);
    const QRect r(pix.rect());
    const QColor fg(widget&&opt?opt->palette.color(widget->foregroundRole()):Qt::black);
    const QColor bg(widget&&opt?opt->palette.color(widget->backgroundRole()):Qt::white);
    const int _2_(size/2),_4_(size/4), _8_(size/8), _16_(size/16);
    switch (sp)
    {
    case SP_DockWidgetCloseButton:
    case SP_TitleBarCloseButton:
    {
        const QRect line(_2_-_16_, _4_, _8_, size-(_4_*2));
        p.setBrush(fg);
        p.setRenderHint(QPainter::Antialiasing);
        QRect l(r.adjusted(_16_, _16_, -_16_, -_16_));
        p.drawEllipse(l);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        const int rot[2] = { 45, 90 };
        for (int i = 0; i < 2; ++i)
        {
            p.translate(_2_, _2_);
            p.rotate(rot[i]);
            p.translate(-_2_, -_2_);
            p.drawRect(line);
        }
        p.end();
        break;
    }
    case SP_TitleBarMaxButton:
    case SP_TitleBarNormalButton:
    {
        p.setBrush(fg);
        p.setRenderHint(QPainter::Antialiasing);
        QRect l(r.adjusted(_16_, _16_, -_16_, -_16_));
        p.drawEllipse(l);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        QRect pl(r.adjusted(_4_+_16_, _4_+_16_, -(_2_-_8_), -(_2_-_8_)));
        QPolygon pol;
        pol << pl.topLeft() << pl.topRight() << pl.bottomLeft();
        p.drawPolygon(pol);
        p.translate(_2_, _2_);
        p.rotate(180);
        p.translate(-_2_, -_2_);
        p.drawPolygon(pol);
        p.end();
        break;
    }
    default: p.end(); pix = QCommonStyle::standardPixmap(sp, opt, widget); break;
    }
    return pix;
}

QRect
StyleProject::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const
{
    QRect ret(pixmap.rect());
    if (flags & (Qt::AlignHCenter|Qt::AlignVCenter))
        ret.moveCenter(r.center());
    if (flags & Qt::AlignLeft)
        ret.moveLeft(r.left());
    else if (flags & Qt::AlignRight)
        ret.moveRight(r.right());
    if (flags & Qt::AlignTop)
        ret.moveTop(r.top());
    else if (flags & Qt::AlignBottom)
        ret.moveBottom(r.bottom());
    return ret;
}
