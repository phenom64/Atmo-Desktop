#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>
#include <QSplitterHandle>
#include <QMap>
#include "namespace.h"
#include "config/settings.h"

namespace DSP
{
class ButtonGroupBase;
class Q_DECL_EXPORT ButtonBase
{
public:
    enum Style { NoStyle = -1, Yosemite = 0, Lion = 1, Sunken = 2, Carved = 3, FollowToolBtn = 4, MagPie = 5, NeoDesk = 6, Anton = 7, StyleCount };
    typedef int ButtonStyle;
    enum ColorRole { Fg = 0, Bg, Mid, Highlight };
    enum Type
    {
        Menu,
        ApplicationMenu,
        OnAllDesktops,
        Minimize,
        Maximize,
        Close,
        ContextHelp,
        Shade,
        KeepBelow,
        KeepAbove,
        Custom
    };

    ButtonBase(const Type buttonType, ButtonGroupBase *group = 0);
    virtual ~ButtonBase();
    const ButtonStyle buttonStyle() const;

    virtual const QRect buttonRect() const = 0;
    virtual const QSize buttonSize() const { return QSize(16, 16); }
    virtual const bool isHovered() const { return m_hasMouse; }
    void paint(QPainter &p);
    bool processMouseEvent(QMouseEvent *e);
    void unhover() { m_hasMouse = false; m_hoverLock = false; hoverChanged(); }
    void hover();
    inline const Type buttonType() const { return m_type; }
    ButtonGroupBase *group() { return m_group; }

protected:
    quint64 state() const;
    void clearBgPix() { m_bgPix.clear(); }
    void drawBase(QColor c, QPainter &p, QRect &r) const;
    void paintCloseButton(QPainter &p);
    void paintMinButton(QPainter &p);
    void paintMaxButton(QPainter &p);
    void paintOnAllDesktopsButton(QPainter &p);
    void paintWindowMenuButton(QPainter &p);
    void paintKeepAboveButton(QPainter &p);
    void paintKeepBelowButton(QPainter &p);
    void paintShadeButton(QPainter &p);
    void paintResizeButton(QPainter &p){}
    void paintQuickHelpButton(QPainter &p);
    void paintApplicationMenuButton(QPainter &p);

    void drawX(QPainter &p, const QRect &r, const QBrush &brush);
    void drawArrow(QPainter &p, const QRect &r, const QBrush &brush, const bool up = false);

    const QColor color(const ColorRole c = Fg) const;
    virtual const bool isDark() const = 0;
    virtual const bool isMaximized() const = 0;
    virtual const bool isActive() const = 0;

    virtual const bool onAllDesktops() const { return false; }
    virtual const bool keepAbove() const { return false; }
    virtual const bool keepBelow() const {return false; }
    virtual const bool shade() const { return false; }

    virtual void hoverChanged() {}
    virtual void onClick(const Qt::MouseButton &button) = 0;

private:
    typedef void (ButtonBase::*PaintEvent)(QPainter &);
    PaintEvent m_paintMethod[Custom];
    Type m_type;
    bool m_hasPress, m_hasMouse, m_hoverLock;
    ButtonStyle m_buttonStyle;
    QMap<quint64, QPixmap> m_bgPix;
    int m_shadowOpacity, m_shadowIllumination;
    ShadowStyle m_shadowStyle;
    QColor m_color, m_bg, m_fg;
    Gradient m_gradient;
    ButtonGroupBase *m_group;
    friend class ButtonGroupBase;
};

class Q_DECL_EXPORT ButtonGroupBase
{
public:
    typedef QList<ButtonBase *> Buttons;
    static ButtonGroupBase *buttonGroup(const quint64 window);
    static void cleanUp(const quint64 window);
    inline Buttons &buttons() { return m_buttons; }
    void clearCache();
    void setColors(const QColor &bg, const QColor &fg, const QColor &min = QColor(), const QColor &max = QColor(), const QColor &close = QColor());
    void configure(const int shadowOpacity, const int shadowIllumination, const ButtonBase::ButtonStyle buttonStyle, const ShadowStyle shadowStyle, const Gradient &grad);
    void addButton(ButtonBase *button) { buttons() << button; }
    void removeButton(ButtonBase *button) { buttons().removeOne(button); }

protected:
    ButtonGroupBase(const quint64 window);

private:
    Buttons m_buttons;
    quint64 m_window;
};

class Q_DECL_EXPORT WidgetButton : public ButtonBase, public QWidget
{
public:
    WidgetButton(Type type, QWidget *parent = 0);
    ~WidgetButton() {}

    const QRect buttonRect() const { return rect(); }
    const bool isHovered() const { return underMouse(); }

protected:
    void mousePressEvent(QMouseEvent *e) { processMouseEvent(e); }
    void mouseReleaseEvent(QMouseEvent *e) { processMouseEvent(e); }

    void onClick(const Qt::MouseButton &button);
    void paintEvent(QPaintEvent *);

    const QColor color(const ColorRole &c = Fg) const;
    const bool isDark() const;
    const bool isMaximized() const { return window()->isMaximized(); }
    const bool isActive() const { return window()->isActiveWindow(); }
};

class Q_DECL_EXPORT SplitterExt : public QWidget
{
    Q_OBJECT
public:
    SplitterExt();
    static void manage(QWidget *sh);
    static bool isActive();

protected:
    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);

private:
    QWidget *m_splitter;
    QPoint m_enterPoint;
    bool m_hasPress;
};
} //namespace

#endif //WIDGETS_H
