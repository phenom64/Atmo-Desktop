#include "dsp.h"
#include "stylelib/gfx.h"
#include "stylelib/ops.h"
#include "stylelib/xhandler.h"
#include "stylelib/color.h"
#include "stylelib/animhandler.h"
#include "stylelib/shadowhandler.h"
#include "stylelib/progresshandler.h"
#include "stylelib/animhandler.h"
#include "stylelib/handlers.h"
#include "config/settings.h"

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
#include <QDialog>
#include <QPainter>
#include <QToolBox>
#include <QStyleFactory>

using namespace DSP;

QStringList
ProjectStylePlugin::keys() const
{
    return QStringList() << "StyleProject";
}

QStyle
*ProjectStylePlugin::create(const QString &key)
{
    if (key.toLower() == "styleproject")
        return new Style();
    return 0;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(StyleProject, ProjectStylePlugin)
#endif

#if QT_VERSION >= 0x050000
class TranslucencyWatcher : public QObject
{
protected:
    bool eventFilter(QObject *o, QEvent *e)
    {
        if (!o->isWidgetType())
            return false;
        const bool titleEvent((e->type() == QEvent::WindowTitleChange) || (e->type() == QEvent::WindowIconChange) || (e->type() == QEvent::ChildAdded));
        QWidget *w = static_cast<QWidget *>(o);
//        if (w->isWindow() &&  (qobject_cast<QMainWindow *>(w) || qobject_cast<QDialog *>(w)))
//            qDebug() << e->type() << w << w->internalWinId() << w->testAttribute(Qt::WA_WState_Created) << w->testAttribute(Qt::WA_TranslucentBackground) << w->parentWidget();
        if (!w->isWindow() || !titleEvent || w->parentWidget())
            return false;

        if (XHandler::opacity() < 1.0f
                && !w->testAttribute(Qt::WA_TranslucentBackground)
                && !w->testAttribute(Qt::WA_WState_Created)
                && !w->internalWinId()
                && (qobject_cast<QMainWindow *>(w) || qobject_cast<QDialog *>(w)))
            Style::applyTranslucency(w);
        return false;
    }
};
static TranslucencyWatcher t;
#endif

Style::Style() : QCommonStyle()
{
    DSP::Settings::read();
    XHandler::init();
    init();
    assignMethods();
    GFX::generateData();
#if QT_VERSION >= 0x050000
    if (XHandler::opacity() < 1.0f)
        qApp->installEventFilter(&t);
#endif
}

Style::~Style()
{
    DSP::Settings::writePalette();
//    ShadowHandler::deleteInstance();
//    ProgressHandler::deleteInstance();
//    Anim::Basic::deleteInstance();
//    Anim::Tabs::deleteInstance();
//    Render::deleteInstance();
//    Ops::deleteInstance();
}

void
Style::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_ce[element] && (this->*m_ce[element])(option, painter, widget)))
    {
#if DEBUG
        qDebug() << "drawControl: unsupported element" << element << option << widget;
#endif
        QCommonStyle::drawControl(element, option, painter, widget);
    }
}

void
Style::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_cc[control] && (this->*m_cc[control])(option, painter, widget)))
    {
#if DEBUG
        qDebug() << "drawComplexControl: unsupported control" << control << option << widget;
#endif
        QCommonStyle::drawComplexControl(control, option, painter, widget);
    }
}

void
Style::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_pe[element] && (this->*m_pe[element])(option, painter, widget)))
    {
#if DEBUG
        qDebug() << "drawPrimitive: unsupported element" << element << option << widget;
#endif
        QCommonStyle::drawPrimitive(element, option, painter, widget);
    }
}

void
Style::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    if (text.isEmpty())
        return;

    // we need to add either hide/show mnemonic, otherwise
    // we are rendering text w/ '&' characters.
    flags |= Qt::TextHideMnemonic; // Qt::TextHideMnemonicTextShowMnemonic

    const QPen pen(painter->pen());
    const QPalette::ColorRole bgRole(Ops::opposingRole(textRole));
    if (enabled
            && dConf.shadows.onTextOpacity
            && pal.brush(bgRole).style() == Qt::SolidPattern
            && !pal.brush(bgRole).gradient()
            && pal.color(textRole).alpha() == 0xff
            && textRole != QPalette::NoRole
            && bgRole != QPalette::NoRole)
    {
        const bool isDark(Color::lum(pal.color(textRole)) > Color::lum(pal.color(bgRole)));
        const int rgb(isDark?0:255);
        const QColor bevel(rgb, rgb, rgb, dConf.shadows.onTextOpacity);
        painter->setPen(bevel);
        painter->drawText(rect.translated(0, 1), flags, text);
    }
    painter->setPen(pal.color(enabled ? QPalette::Active : QPalette::Disabled, textRole));
    painter->drawText(rect, flags, text);
    painter->setPen(pen);
}

void
Style::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    painter->drawPixmap(itemPixmapRect(rect, alignment, pixmap).intersected(rect), pixmap);
//    QStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

QPixmap
Style::standardPixmap(StandardPixmap sp, const QStyleOption *opt, const QWidget *widget) const
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
    default:
    {
        p.end();
        pix = QCommonStyle::standardPixmap(sp, opt, widget);
        break;
    }
    }
    return pix;
}

QRect
Style::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const
{
    return GFX::subRect(r, flags, pixmap.rect());
}

bool
Style::inUno(QToolBar *bar, bool *activeWindow)
{
    const QMainWindow *win(bar?qobject_cast<const QMainWindow *>(bar->window()):0);
    const bool inUno = win && bar && win->toolBarArea(bar) == Qt::TopToolBarArea;
    if (activeWindow)
        *activeWindow = Handlers::Window::isActiveWindow(win);
    return inUno;
}

void
Style::installFilter(QWidget *w)
{
    w->removeEventFilter(this);
    w->installEventFilter(this);
}

void
Style::drawText(const QRect &r,
                QPainter *p,
                QString text,
                const QStyleOption *opt,
                int flags,
                const QPalette::ColorRole textRole,
                const Qt::TextElideMode elide,
                const bool bold,
                const bool forceStretch) const
{
    const QFont font = p->font();
    QFont f = p->font();
    text = QFontMetrics(f).elidedText(text, Qt::ElideNone, QWIDGETSIZE_MAX, Qt::TextShowMnemonic);
    int regularW(QFontMetrics(f).width(text));
    f.setBold(bold);
    int boldW(QFontMetrics(f).width(text));
    if (bold && (forceStretch || boldW > r.width()))
        f.setStretch(qCeil((double)regularW * 100.0f / (double)boldW));
    if (elide != Qt::ElideNone)
        text = QFontMetrics(f).elidedText(text, elide, r.width(), Qt::TextShowMnemonic);
    if (bold)
        p->setFont(f);
    drawItemText(p, r, flags, opt->palette, isEnabled(opt), text, textRole);
    p->setFont(font);
}
