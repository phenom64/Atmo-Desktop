#include <kwindowinfo.h>
#include <QPainter>
#include <QHBoxLayout>

#include "kwinclient.h"
#include "button.h"

KwinClient::KwinClient(KDecorationBridge *bridge, Factory *factory)
    : KDecoration(bridge, factory)
    , m_titleLayout(0)
{
    setParent(factory);
}

void
KwinClient::init()
{
    createMainWidget();
    widget()->installEventFilter(this);
    KWindowInfo info(windowId(), NET::WMWindowType, NET::WM2WindowClass);
    m_mainLayout = new QVBoxLayout(widget());
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_titleLayout = new QHBoxLayout();
    m_mainLayout->addLayout(m_titleLayout);
    m_mainLayout->addStretch();
    populate();
    m_titleLayout->addStretch();
    widget()->setLayout(m_mainLayout);
}

void
KwinClient::populate()
{
    const QString &buttons(options()->titleButtonsLeft());
    for (int i = 0; i < buttons.size(); ++i)
    {
        Button::Type t;
        bool isSeparator(false);
        bool supported(true);
        switch (buttons.at(i).toLatin1())
        {
        /*
        * @li 'N' application menu button
        * @li 'M' window menu button
        * @li 'S' on_all_desktops button
        * @li 'H' quickhelp button
        * @li 'I' minimize ( iconify ) button
        * @li 'A' maximize button
        * @li 'X' close button
        * @li 'F' keep_above_others button
        * @li 'B' keep_below_others button
        * @li 'L' shade button
        * @li 'R' resize button
        * @li '_' spacer
        */
        case 'X': t = Button::Close; break;
        case 'I': t = Button::Min; break;
        case 'A': t = Button::Max; break;
        case '-': isSeparator = true;
        default: supported = false; break;
        }
        if (!supported)
            continue;
        if (isSeparator)
            m_titleLayout->addSpacing(4);
        else
            m_titleLayout->addWidget(new Button(t, this, widget()));
    }
}

void
KwinClient::resize(const QSize &s)
{
    widget()->resize(s);
}

void
KwinClient::borders(int &left, int &right, int &top, int &bottom) const
{
    left = 0;
    right = 0;
    top = 16;
    bottom = 0;
}

void
KwinClient::captionChange()
{
    widget()->update();
}

bool
KwinClient::eventFilter(QObject *o, QEvent *e)
{
    if (o != widget())
        return false;
    switch (e->type())
    {
    case QEvent::Paint:
    {
        QPainter p(widget());
        p.setPen(options()->color(ColorFont, isActive()));
        paint(p);
        p.end();
        return true;
    }
    case QEvent::MouseButtonDblClick:
        titlebarDblClickOperation();
        return true;
    case QEvent::MouseButtonPress:
        processMousePressEvent(static_cast<QMouseEvent *>(e));
        return true;
    case QEvent::Wheel:
        titlebarMouseWheelOperation(static_cast<QWheelEvent *>(e)->delta());
        return true;
    default: break;
    }
    return KDecoration::eventFilter(o, e);
}

void
KwinClient::paint(QPainter &p)
{
    QRect r(0, 0, width(), 16);
    p.drawText(r, Qt::AlignCenter, caption());
}

QSize
KwinClient::minimumSize() const
{
    return QSize(256, 128);
}
