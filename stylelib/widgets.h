#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>
#include <QSplitterHandle>
#include <QMap>

class Q_DECL_EXPORT ButtonBase
{
public:
    enum Style { NoStyle = -1, Yosemite = 0, Lion, Sunken, Carved, StyleCount };
    typedef int ButtonStyle;
    enum ColorRole { Fg = 0, Bg, Mid, Highlight };
    enum Type { Close,
                Min,
                Max,
                OnAllDesktops,
                WindowMenu,
                KeepAbove,
                KeepBelow,
                Shade,
                Resize,
                QuickHelp,
                AppMenu,
                TypeCount };
    ButtonBase(Type type);
    ~ButtonBase();

    virtual const QRect buttonRect() const = 0;
    virtual const bool isHovered() const { return m_hasMouse; }
    void paint(QPainter &p);
    bool processMouseEvent(QMouseEvent *e);
    void unhover() { m_hasMouse = false; }
    const Type type() const { return m_type; }

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

    virtual const ButtonStyle buttonStyle() const;
    virtual const bool onAllDesktops() const { return false; }
    virtual const bool keepAbove() const { return false; }
    virtual const bool keepBelow() const {return false; }
    virtual const bool shade() const { return false; }

    virtual void hoverChanged() {}
    virtual void onClick(const Qt::MouseButton &button) = 0;

private:
    typedef void (ButtonBase::*PaintEvent)(QPainter &);
    PaintEvent m_paintEvent[TypeCount];
    Type m_type;
    bool m_hasPress, m_hasMouse;
    QMap<quint64, QPixmap> m_bgPix;
};

class Q_DECL_EXPORT Button : public ButtonBase, public QWidget
{
public:
    Button(Type type, QWidget *parent = 0);
    ~Button() {}

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

protected:
    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);

private:
    QWidget *m_splitter;
    QPoint m_enterPoint;
    bool m_hasPress;
};

#endif //WIDGETS_H
