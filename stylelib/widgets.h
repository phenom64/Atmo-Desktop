#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>
#include <QSplitterHandle>
#include <QMap>

namespace DSP
{
class Q_DECL_EXPORT ButtonBase
{
public:
    enum Style { NoStyle = -1, Yosemite = 0, Lion, Sunken, Carved, FollowToolBtn, MagPie, StyleCount };
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

    ButtonBase(Type type);
    virtual ~ButtonBase();

    virtual const QRect buttonRect() const = 0;
    virtual const bool isHovered() const { return m_hasMouse; }
    void paint(QPainter &p);
    bool processMouseEvent(QMouseEvent *e);
    void unhover() { m_hasMouse = false; m_hoverLock = false; hoverChanged(); }
    void hover();
    const Type type() const { return m_type; }

    inline void setButtonStyle(const ButtonStyle s) { m_buttonStyle = s; m_bgPix.clear(); }
    inline const ButtonStyle buttonStyle() const { return m_buttonStyle; }

    inline void setShadowOpacity(const int o) { m_shadowOpacity = o; m_bgPix.clear(); }
    inline const int shadowOpacity() const { return m_shadowOpacity; }

protected:
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

    virtual const QColor color(const ColorRole &c = Fg) const = 0;
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
    int m_shadowOpacity;
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
