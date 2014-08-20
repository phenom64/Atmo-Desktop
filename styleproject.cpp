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

Settings Q_DECL_EXPORT StyleProject::m_s;

StyleProject::StyleProject()
    : QCommonStyle()
{
//    Anim::setStyle(this);
    init();
    assignMethods();
    readSettings();
    Render::generateData();
    QPalette p(qApp->palette());
    QColor c = p.color(QPalette::Window);
    Color::titleBarColors[0] = Color::mid(c, Qt::white, 10, 1);
    Color::titleBarColors[1] = Color::mid(c, Qt::black, 10, 1);
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
    if (text.isEmpty())
        return;
    painter->save();
    // we need to add either hide/show mnemonic, otherwise
    // we are rendering text w/ '&' characters.
    flags |= Qt::TextHideMnemonic; // Qt::TextHideMnemonicTextShowMnemonic
//    if (painter->fontMetrics().boundingRect(text).width() > rect.width()) //if we have more text then space its pointless to render the center of the text...
//        flags &= ~Qt::AlignHCenter;
    const QPalette::ColorRole bgRole(Ops::opposingRole(textRole));
    if (pal.color(textRole).isValid() && pal.color(bgRole).isValid())
    if (textRole != QPalette::NoRole)
    {
        if (bgRole != QPalette::NoRole && enabled)
        {
            const bool isDark(Color::luminosity(pal.color(textRole)) > Color::luminosity(pal.color(bgRole)));
            const int rgb(isDark?0:255);
            const QColor bevel(rgb, rgb, rgb, 127);
            painter->setPen(bevel);
            painter->drawText(rect.translated(0, 1), flags, text);
        }
        painter->setPen(pal.color(enabled ? QPalette::Active : QPalette::Disabled, textRole));
    }

    painter->drawText(rect, flags, text);
//    QCommonStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
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
    QImage img(size, size, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::NoPen);
    const QRect r(img.rect());
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
        return QPixmap::fromImage(img);
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
        return QPixmap::fromImage(img);
    }
    default: p.end(); return QCommonStyle::standardPixmap(sp, opt, widget);
    }

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

void
StyleProject::fixTitleLater(QWidget *win)
{
    QTimer *t = new QTimer(win);
    connect(t, SIGNAL(timeout()), this, SLOT(fixTitle()));
    t->start(250);
}

void
StyleProject::fixTitle()
{
    if (castObj(QTimer *, t, sender()))
    {
        if (castObj(QWidget *, w, t->parent()))
            Ops::fixWindowTitleBar(w);
        t->stop();
        t->deleteLater();
    }
}

void
StyleProject::updateToolBar(QToolBar *toolBar)
{
    QMainWindow *win = static_cast<QMainWindow *>(toolBar->parentWidget());
    if (toolBar->isWindow() && toolBar->isFloating())
    {
        toolBar->setContentsMargins(0, 0, 0, 0);
        if (toolBar->layout())
            toolBar->layout()->setContentsMargins(4, 4, 4, 4);
    }
    else if (win->toolBarArea(toolBar) == Qt::TopToolBarArea && toolBar->layout())
    {
        if (toolBar->geometry().top() <= win->rect().top() && !toolBar->isWindow())
        {
            toolBar->setMovable(true);
            toolBar->layout()->setContentsMargins(0, 0, 0, 0);
            toolBar->setContentsMargins(0, 0, pixelMetric(PM_ToolBarHandleExtent), 6);
        }
        else if (toolBar->findChild<QTabBar *>()) //sick, put a tabbar in a toolbar... eiskaltdcpp does this :)
        {
            toolBar->layout()->setContentsMargins(0, 0, 0, 0);
            toolBar->setMovable(false);
        }
        else
            toolBar->layout()->setContentsMargins(2, 2, 2, 2);
    }
}

void
StyleProject::fixMainWindowToolbar()
{
    updateToolBar(static_cast<QToolBar *>(sender()));
}

