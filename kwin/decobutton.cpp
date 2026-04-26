/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**************************************************************************
*   Based on styleproject, Copyright (C) 2013 by Robert Metsaranta        *
*   therealestrob@gmail.com                                              *
***************************************************************************/
#include "decobutton.h"
#include "kwinclient2.h"
#include "../atmolib/color.h"
#include "../atmolib/ops.h"
#include "../atmolib/xhandler.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QCoreApplication>

#if HASXCB || HASX11
#include "../atmolib/atmox11.h"
#endif

namespace NSE
{

///-------------------------------------------------------------------------------------------------

Button::Button(KDecoration3::DecorationButtonType type, Deco *decoration, QObject *parent)
    : KDecoration3::DecorationButton(type, decoration, parent)
    , ButtonBase((ButtonBase::Type)type, decoration->m_buttonManager)
{
    m_decoration = decoration;
    setGeometry(AtmoDecoRect(AtmoDecoPoint(0, 0), buttonSize()));
}

Button::~Button()
{
}

bool
Button::isSupported(KDecoration3::DecorationButtonType t, Deco *d)
{
    bool supported(true);
    switch (t)
    {
    case KDecoration3::DecorationButtonType::Close: supported = d->window()->isCloseable(); break;
    case KDecoration3::DecorationButtonType::Maximize: supported = d->window()->isMaximizeable(); break;
    case KDecoration3::DecorationButtonType::Minimize: supported = d->window()->isMinimizeable(); break;
    case KDecoration3::DecorationButtonType::Shade: supported = d->window()->isShadeable(); break;
    default: break;
    }
    return supported;
}

Button
*Button::create(KDecoration3::DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent)
{
    if (Deco *d = qobject_cast<Deco *>(decoration))
        if (isSupported(type, d))
            return new Button(type, d, parent);
    return 0;
}

const QSize
Button::buttonSize() const
{
//    if (buttonType() == ApplicationMenu)
//    {
//        return QSize();
//    }
    const int small = Ops::dpiScaled(nullptr, 15);
    const int regular = Ops::dpiScaled(nullptr, 16);
    const int antonClose = Ops::dpiScaled(nullptr, 19);
    const int anton = Ops::dpiScaled(nullptr, 17);
    if (buttonStyle() == ButtonBase::Anton)
    {
        if (buttonType() == ButtonBase::Close)
            return QSize(antonClose, small);
        return QSize(anton, small);
    }
    return QSize(regular, regular);
}

void
Button::paint(QPainter *painter, const AtmoDecoRect &repaintArea)
{
//    if (buttonType() == ApplicationMenu)
//    {
//        if (!geometry().isValid())
//        {
//            qreal w = QFontMetrics(painter->font()).boundingRect("Menu").width()+12;
//            setGeometry(QRectF(0.0, 0.0, w, m_decoration->titleHeight()));
//        }
//        const QPen &saved = painter->pen();
//        const bool aa = painter->testRenderHint(QPainter::Antialiasing);
//        painter->setPen(m_decoration->m_textFg);
//        painter->setRenderHint(QPainter::Antialiasing);
//        painter->drawText(geometry(), Qt::AlignCenter, "Menu");
//        painter->setPen(saved);
//        painter->setRenderHint(QPainter::Antialiasing, aa);
//        return;
//    }
    ButtonBase::paint(*painter);
}

const bool
Button::isActive() const
{
    return decoration()->window()->isActive();
}

const bool
Button::isMaximized() const
{
    return decoration()->window()->isMaximized();
}

const bool
Button::keepBelow() const
{
    return decoration()->window()->isKeepBelow();
}

const bool
Button::keepAbove() const
{
    return decoration()->window()->isKeepAbove();
}

const bool
Button::onAllDesktops() const
{
    return decoration()->window()->isOnAllDesktops();
}

const bool
Button::shade() const
{
    return decoration()->window()->isShaded();
}

const bool
Button::isDark() const
{
    return Color::lum(color(ButtonBase::Fg)) > Color::lum(color(ButtonBase::Bg));
}

void
Button::hoverEnterEvent(QHoverEvent *event)
{
    KDecoration3::DecorationButton::hoverEnterEvent(event);
//    hover();
}

void
Button::hoverLeaveEvent(QHoverEvent *event)
{
    KDecoration3::DecorationButton::hoverLeaveEvent(event);
//    unhover();
}

void
Button::mouseReleaseEvent(QMouseEvent *event)
{
    KDecoration3::DecorationButton::mouseReleaseEvent(event);
//    update();
    processMouseEvent(event);
    update();
}

void
Button::mousePressEvent(QMouseEvent *event)
{
    KDecoration3::DecorationButton::mousePressEvent(event);
    processMouseEvent(event);
    update();
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
    , m_hasBeenShown(false)
{
    bool isX11(false);
#if HASXCB || HASX11
    isX11 = AtmoX11::isAvailable();
#endif
    if (!isX11 || !m_deco || !m_deco->winId())
    {
        hide();
        return;
    }
    setFocusPolicy(Qt::NoFocus);
//    setFixedSize(48, 16);
//    KDecoration3::DecoratedClient *c = m_deco->window();

    KDecoration3::DecorationButtonGroup *group = s == Left ? m_deco->m_leftButtons : m_deco->m_rightButtons;
    const auto buttons = group->buttons();
    QHBoxLayout *l = new QHBoxLayout(this);
    for (const auto &button : buttons)
    {
        if (!button)
            continue;
        l->addWidget(new EmbeddedButton(this, static_cast<ButtonBase::Type>(button->type())));
    }
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(Ops::dpiScaled(nullptr, 4));
    setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    restack();
    updatePosition();

    if (s == Right)
        connect(d->window(), &AtmoDecoratedWindow::widthChanged, this, &EmbeddedWidget::updatePosition);

    connect(m_deco, SIGNAL(dataChanged()), this, SLOT(update()));
    show();
}

void
EmbeddedWidget::cleanUp()
{
    if (!m_hasBeenShown)
        return;

    WindowData d = WindowData::memory(m_deco->winId(), m_deco);
    if (d && d.lock())
    {
        if (m_side == Left)
            d->leftEmbedSize = 0;
        if (m_side == Right)
            d->rightEmbedSize = 0;
        d.unlock();
    }
    AdaptorManager::instance()->dataChanged(m_deco->winId());
}

void
EmbeddedWidget::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    if (m_hasBeenShown)
        return;

    WindowData d = WindowData::memory(m_deco->winId(), m_deco);
    if (d && d.lock())
    {
        if (m_side == Left)
            d->leftEmbedSize = width() + Ops::dpiScaled(nullptr, 8);
        if (m_side == Right)
            d->rightEmbedSize = width() + Ops::dpiScaled(nullptr, 8);
        d.unlock();
    }
    AdaptorManager::instance()->dataChanged(m_deco->winId());
    m_hasBeenShown = true;
}

const QPoint
EmbeddedWidget::topLeft() const
{
    const int sidePad = Ops::dpiScaled(nullptr, 8);
    const int topPad = Ops::dpiScaled(nullptr, 4);
    return QPoint(m_side == Left ? sidePad : m_deco->titleBar().width() - width() - sidePad, topPad);
}

void
EmbeddedWidget::updatePosition()
{
    XHandler::move(winId(), topLeft());
}

void
EmbeddedWidget::restack()
{
    if (XHandler::XWindow windowId = m_deco->winId())
        XHandler::restack(winId(), windowId);
    else
        hide();
}

void
EmbeddedWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setBrushOrigin(-(topLeft()+QPoint(0, m_deco->titleHeight())));
    p.fillRect(rect(), m_deco->m_bgPix);
    p.end();
}

void
EmbeddedWidget::mousePressEvent(QMouseEvent *e)
{
    m_press = true;
    QWidget::mousePressEvent(e);
    if (m_deco->winId())
    {
        XHandler::pressEvent(e->globalPosition().toPoint(), m_deco->winId(), e->button());
    }
}

void
EmbeddedWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_press = false;
    QWidget::mouseReleaseEvent(e);
    if (m_deco->winId())
    {
        XHandler::releaseEvent(e->globalPosition().toPoint(), m_deco->winId(), e->button());
    }
}

void
EmbeddedWidget::mouseMoveEvent(QMouseEvent *e)
{
    QWidget::mouseMoveEvent(e);
    if (m_press && m_deco->winId())
    {
        XHandler::mwRes(e->position().toPoint(), e->globalPosition().toPoint(), m_deco->winId());
    }
}

void
EmbeddedWidget::wheelEvent(QWheelEvent *e)
{
    if (m_deco->winId())
        XHandler::wheelEvent(m_deco->winId(), e->angleDelta().y() > 0);
    e->accept();
}

//--------------------------------------------------------------------------------------------------------

EmbedHandler::EmbedHandler(Deco *d) : m_deco(d)
{
    for (int i = 0; i < 2; ++i)
    {
        m_embedded[i] = new EmbeddedWidget(m_deco, (EmbeddedWidget::Side)i);
        if (!m_embedded[i]->findChild<EmbeddedButton *>())
        {
            m_embedded[i]->cleanUp();
            m_embedded[i]->deleteLater();
            m_embedded[i] = 0;
        }
    }

}

#define EW m_embedded[i]
#define RUN(...) \
for (int i = 0; i < 2; ++i) \
    if (EW) { __VA_ARGS__ }

EmbedHandler::~EmbedHandler()
{
    RUN(EW->cleanUp(); EW->deleteLater(); EW=0;)
}

void
EmbedHandler::repaint()
{
    RUN(EW->repaint();)
}

void
EmbedHandler::setUpdatesEnabled(bool enabled)
{
    RUN(EW->setUpdatesEnabled(enabled);)
}


//--------------------------------------------------------------------------------------------------------

EmbeddedButton::EmbeddedButton(QWidget *parent, const Type &t)
    : QWidget(parent)
    , ButtonBase(t)
{
    setAttribute(Qt::WA_Hover);
    const int buttonExtent = Ops::dpiScaled(nullptr, 16);
    setFixedSize(buttonExtent, buttonExtent);
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
    return decoration()->window()->isActive();
}

const bool
EmbeddedButton::isMaximized() const
{
    return decoration()->window()->isMaximized();
}

const bool
EmbeddedButton::keepBelow() const
{
    return decoration()->window()->isKeepBelow();
}

const bool
EmbeddedButton::keepAbove() const
{
    return decoration()->window()->isKeepAbove();
}

const bool
EmbeddedButton::onAllDesktops() const
{
    return decoration()->window()->isOnAllDesktops();
}

const bool
EmbeddedButton::shade() const
{
    return decoration()->window()->isShaded();
}

const bool
EmbeddedButton::isDark() const
{
    return Color::lum(color(ButtonBase::Fg)) > Color::lum(color(ButtonBase::Bg));
}

//const QColor
//EmbeddedButton::color(const ColorRole &c) const
//{
//    const Deco *d = const_cast<const Deco *>(const_cast<EmbeddedButton *>(this)->decoration());
//    if (c == Mid)
//    {
//        const QColor &fg = d->fgColor();
//        const QColor &bg = d->bgColor();
//        const bool dark(Color::lum(fg) > Color::lum(bg));
//        return Color::mid(fg, bg, 1, (!dark*4)+(!isActive()*(dark?2:8)));
//    }
//    if (c == ButtonBase::Highlight)
//        return d->window()->palette().color(QPalette::Highlight);
//    if (c == ButtonBase::Fg)
//        return d->fgColor();
//    if (c == ButtonBase::Bg)
//        return d->bgColor();
//    return QColor();
//}

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
        switch (buttonType())
        {
        case Close: decoration()->requestClose(); break;
        case Maximize: decoration()->requestToggleMaximization(button); break;
        case Minimize: decoration()->requestMinimize(); break;
        case ContextHelp: decoration()->requestContextHelp(); break;
        case OnAllDesktops: decoration()->requestToggleOnAllDesktops(); break;
        case Shade: decoration()->requestToggleShade(); break;
        case KeepAbove: decoration()->requestToggleKeepAbove(); break;
        case KeepBelow: decoration()->requestToggleKeepBelow(); break;
        case Menu:
#if defined(ATMO_USE_KDECORATION3)
            decoration()->requestShowWindowMenu(buttonRect().translated(mapToGlobal(QPoint(0, 0))));
#else
            decoration()->requestShowWindowMenu();
#endif
            break;
        default: break;
        }
//    }
}

} //NSE
