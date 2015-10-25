#include "decobutton.h"
#include "kwinclient2.h"
#include "../stylelib/color.h"
#include "../stylelib/xhandler.h"

#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationButton>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QCoreApplication>

#if HASXCB
#include <QX11Info>
#endif

///-------------------------------------------------------------------------------------------------

namespace DSP
{

Button::Button(QObject *parent, const QVariantList &args)
    : KDecoration2::DecorationButton(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<KDecoration2::Decoration*>(), parent)
    , ButtonBase((ButtonBase::Type)args.at(0).value<KDecoration2::DecorationButtonType>())
{
    setGeometry(QRectF(0, 0, 16, 16));
}

Button::Button(KDecoration2::DecorationButtonType type, Deco *decoration, QObject *parent)
    : KDecoration2::DecorationButton(type, decoration, parent)
    , ButtonBase((ButtonBase::Type)type)
{
    setGeometry(QRectF(0, 0, 16, 16));
}

bool
Button::isSupported(KDecoration2::DecorationButtonType t, Deco *d)
{
    bool supported(true);
    switch (t)
    {
    case KDecoration2::DecorationButtonType::Close: supported = d->client().data()->isCloseable(); break;
    case KDecoration2::DecorationButtonType::Maximize: supported = d->client().data()->isMaximizeable(); break;
    case KDecoration2::DecorationButtonType::Minimize: supported = d->client().data()->isMinimizeable(); break;
    case KDecoration2::DecorationButtonType::Shade: supported = d->client().data()->isShadeable(); break;
    default: break;
    }
    return supported;
}

Button
*Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    if (Deco *d = qobject_cast<Deco *>(decoration))
        if (isSupported(type, d))
            return new Button(type, d, parent);
    return 0;
}

void
Button::paint(QPainter *painter, const QRect &repaintArea)
{
    ButtonBase::paint(*painter);
}

const bool
Button::isActive() const
{
    return decoration()->client().data()->isActive();
}

const bool
Button::isMaximized() const
{
    return decoration()->client().data()->isMaximized();
}

const bool
Button::keepBelow() const
{
    return decoration()->client().data()->isKeepBelow();
}

const bool
Button::keepAbove() const
{
    return decoration()->client().data()->isKeepAbove();
}

const bool
Button::onAllDesktops() const
{
    return decoration()->client().data()->isOnAllDesktops();
}

const bool
Button::shade() const
{
    return decoration()->client().data()->isShaded();
}

const bool
Button::isDark() const
{
    return Color::luminosity(color(ButtonBase::Fg)) > Color::luminosity(color(ButtonBase::Bg));
}

const QColor
Button::color(const ColorRole &c) const
{
    if (c == Mid)
    {
        const QColor &fg = static_cast<Deco *>(decoration().data())->fgColor();
        const QColor &bg = static_cast<Deco *>(decoration().data())->bgColor();
        const bool dark(Color::luminosity(fg) > Color::luminosity(bg));
        return Color::mid(fg, bg, 1, (!dark*4)+(!isActive()*(dark?2:8)));
    }
    if (c == ButtonBase::Highlight)
        return decoration()->client().data()->palette().color(QPalette::Highlight);
    if (c == ButtonBase::Fg)
        return static_cast<Deco *>(decoration().data())->fgColor();
    if (c == ButtonBase::Bg)
        return static_cast<Deco *>(decoration().data())->bgColor();
    return QColor();
}

void
Button::hoverEnterEvent(QHoverEvent *event)
{
    KDecoration2::DecorationButton::hoverEnterEvent(event);
//    hover();
}

void
Button::hoverLeaveEvent(QHoverEvent *event)
{
    KDecoration2::DecorationButton::hoverLeaveEvent(event);
//    unhover();
}

void
Button::mouseReleaseEvent(QMouseEvent *event)
{
    KDecoration2::DecorationButton::mouseReleaseEvent(event);
//    update();
}

void
Button::hoverChanged()
{
    update();
}

//--------------------------------------------------------------------------------------------------------

EmbeddedWidget::EmbeddedWidget(Deco *d, const Side s)
    : QWidget(0)
    , m_deco(d)
    , m_side(s)
    , m_press(false)
{
    bool isX11(false);
#if HASXCB
    isX11 = QX11Info::isPlatformX11();
#endif
    if (!isX11 || !m_deco)
    {
        hide();
        return;
    }

    setFocusPolicy(Qt::NoFocus);
//    setFixedSize(48, 16);
//    KDecoration2::DecoratedClient *c = m_deco->client().data();

    QHBoxLayout *l = new QHBoxLayout(this);
    KDecoration2::DecorationButtonGroup *group = s == Left ? m_deco->m_leftButtons : m_deco->m_rightButtons;
    QVector< QPointer<KDecoration2::DecorationButton > > buttons = group->buttons();
    for (int i = 0; i < buttons.count(); ++i)
        l->addWidget(new EmbeddedButton(this, (ButtonBase::Type)buttons.at(i).data()->type()));
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(4);
    setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    restack();
    updatePosition();

    if (s == Right)
        connect(d->client().data(), &KDecoration2::DecoratedClient::widthChanged, this, &EmbeddedWidget::updatePosition);

    show();
}

void
EmbeddedWidget::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    if (WindowData *d = m_deco->m_wd)
    {
        d->setValue<uint>(m_side == Left ? WindowData::LeftEmbedSize : WindowData::RightEmbedSize, width()+8);
        AdaptorManager::instance()->dataChanged(m_deco->client().data()->windowId());
    }
}

const QPoint
EmbeddedWidget::topLeft() const
{
    return QPoint(m_side==Left?8:m_deco->titleBar().width()-width()-8, 4);
}

void
EmbeddedWidget::updatePosition()
{
    XHandler::move(winId(), topLeft());
}

void
EmbeddedWidget::restack()
{
    if (XHandler::XWindow windowId = m_deco->client().data()->windowId())
        XHandler::restack(winId(), windowId);
    else
        hide();
}

void
EmbeddedWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    WindowData *d = m_deco->m_wd;
    if (d && d->lock())
    {
        p.setBrushOrigin(-(topLeft()+QPoint(0, m_deco->titleHeight())));
        const QImage img = d->image();
        if (!img.isNull())
            p.fillRect(rect(), img);
        d->unlock();
    }
    p.end();
}

void
EmbeddedWidget::mousePressEvent(QMouseEvent *e)
{
    m_press = true;
    QWidget::mousePressEvent(e);
    XHandler::pressEvent(e->globalPos(), m_deco->client().data()->decorationId(), e->button());
}

void
EmbeddedWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_press = false;
    QWidget::mouseReleaseEvent(e);
    XHandler::releaseEvent(e->globalPos(), m_deco->client().data()->decorationId(), e->button());
}

void
EmbeddedWidget::mouseMoveEvent(QMouseEvent *e)
{
    QWidget::mouseMoveEvent(e);
    if (m_press)
        XHandler::mwRes(e->pos(), e->globalPos(), m_deco->client().data()->windowId());
}

void
EmbeddedWidget::wheelEvent(QWheelEvent *e)
{
    XHandler::wheelEvent(m_deco->client().data()->decorationId(), e->delta()>0);
    e->accept();
}

void
EmbeddedWidget::setButtonShadowOpacity(const int o)
{
    QList<EmbeddedButton *> buttons(findChildren<EmbeddedButton *>());
    for (int i = 0; i < buttons.count(); ++i)
        buttons.at(i)->setShadowOpacity(o);
}

void
EmbeddedWidget::setButtonStyle(ButtonBase::ButtonStyle style)
{
    QList<EmbeddedButton *> buttons(findChildren<EmbeddedButton *>());
    for (int i = 0; i < buttons.count(); ++i)
        buttons.at(i)->setButtonStyle(style);
}

//--------------------------------------------------------------------------------------------------------

EmbeddedButton::EmbeddedButton(QWidget *parent, const Type &t)
    : QWidget(parent)
    , ButtonBase(t)
{
    setAttribute(Qt::WA_Hover);
    setFixedSize(16, 16);
    setForegroundRole(QPalette::ButtonText);
    setBackgroundRole(QPalette::Button);
}

Deco
*EmbeddedButton::decoration() const
{
    return embeddedWidget()->m_deco;
}

void
EmbeddedButton::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    ButtonBase::paint(painter);
    painter.end();
}

const bool
EmbeddedButton::isActive() const
{
    return decoration()->client().data()->isActive();
}

const bool
EmbeddedButton::isMaximized() const
{
    return decoration()->client().data()->isMaximized();
}

const bool
EmbeddedButton::keepBelow() const
{
    return decoration()->client().data()->isKeepBelow();
}

const bool
EmbeddedButton::keepAbove() const
{
    return decoration()->client().data()->isKeepAbove();
}

const bool
EmbeddedButton::onAllDesktops() const
{
    return decoration()->client().data()->isOnAllDesktops();
}

const bool
EmbeddedButton::shade() const
{
    return decoration()->client().data()->isShaded();
}

const bool
EmbeddedButton::isDark() const
{
    return Color::luminosity(color(ButtonBase::Fg)) > Color::luminosity(color(ButtonBase::Bg));
}

const QColor
EmbeddedButton::color(const ColorRole &c) const
{
    const Deco *d = const_cast<const Deco *>(const_cast<EmbeddedButton *>(this)->decoration());
    if (c == Mid)
    {
        const QColor &fg = d->fgColor();
        const QColor &bg = d->bgColor();
        const bool dark(Color::luminosity(fg) > Color::luminosity(bg));
        return Color::mid(fg, bg, 1, (!dark*4)+(!isActive()*(dark?2:8)));
    }
    if (c == ButtonBase::Highlight)
        return d->client().data()->palette().color(QPalette::Highlight);
    if (c == ButtonBase::Fg)
        return d->fgColor();
    if (c == ButtonBase::Bg)
        return d->bgColor();
    return QColor();
}

void
EmbeddedButton::hoverChanged()
{
    update();
}

void
EmbeddedButton::onClick(const Qt::MouseButton &button)
{
//    if (button == Qt::LeftButton)
//    {
        switch (type())
        {
        case Close: decoration()->requestClose(); break;
        case Maximize: decoration()->requestToggleMaximization(button); break;
        case Minimize: decoration()->requestMinimize(); break;
        case ContextHelp: decoration()->requestContextHelp(); break;
        case OnAllDesktops: decoration()->requestToggleOnAllDesktops(); break;
        case Shade: decoration()->requestToggleShade(); break;
        case KeepAbove: decoration()->requestToggleKeepAbove(); break;
        case KeepBelow: decoration()->requestToggleKeepBelow(); break;
        case Menu: decoration()->requestShowWindowMenu(); break;
        default: break;
        }
//    }
}

} //DSP

