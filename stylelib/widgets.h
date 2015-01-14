#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>
#include <QSplitterHandle>

class Q_DECL_EXPORT ButtonBase
{
public:
    enum Style { Yosemite = 0, Lion, Sunken, Carved, StyleCount };
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

    virtual QRect buttonRect() = 0;
    virtual bool isHovered() { return m_hasMouse; }
    void paint(QPainter &p);
    void processMouseEvent(QMouseEvent *e);
    void unhover() { m_hasMouse = false; }

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    virtual void paintCloseButton(QPainter &p);
    virtual void paintMinButton(QPainter &p);
    virtual void paintMaxButton(QPainter &p);
    virtual void paintOnAllDesktopsButton(QPainter &p) {}
    virtual void paintWindowMenuButton(QPainter &p) {}
    virtual void paintKeepAboveButton(QPainter &p) {}
    virtual void paintKeepBelowButton(QPainter &p) {}
    virtual void paintShadeButton(QPainter &p) {}
    virtual void paintResizeButton(QPainter &p) {}
    virtual void paintQuickHelpButton(QPainter &p) {}
    virtual void paintApplicationMenuButton(QPainter &p) {}

    virtual const QColor color(const ColorRole &c = Fg) const = 0;
    virtual const bool isDark() const = 0;
    virtual bool isMaximized() = 0;
    virtual const ButtonStyle buttonStyle() const;
    virtual void hoverChanged() {}

    void drawBase(QColor c, QPainter &p, QRect &r) const;

    typedef void (ButtonBase::*PaintEvent)(QPainter &);
    virtual bool isActive() const = 0;
    virtual void onClick(QMouseEvent *e, const Type &t) = 0;

    PaintEvent m_paintEvent[TypeCount];
    Type m_type;
    bool m_hasPress, m_hasMouse;
    QPixmap m_bgPix[2];
};

class Q_DECL_EXPORT Button : public ButtonBase, public QWidget
{
public:
    Button(Type type, QWidget *parent = 0);
    ~Button() {}

    virtual QRect buttonRect() { return rect(); }
    virtual bool isHovered() { return underMouse(); }

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    const QColor color(const ColorRole &c = Fg) const;
    const bool isDark() const;
    bool isMaximized() { return window()->isMaximized(); }

    virtual bool isActive() const = 0;
    virtual void onClick(QMouseEvent *e, const Type &t) = 0;
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
