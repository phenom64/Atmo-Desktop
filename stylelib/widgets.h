#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>
class Q_DECL_EXPORT Button : public QWidget
{
public:
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
    Button(Type type, QWidget *parent = 0);
    ~Button();

    inline void setBgPix(const QPixmap &bg) {m_bgPix = bg;}

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    virtual bool paintCloseButton(QPainter &p);
    virtual bool paintMinButton(QPainter &p);
    virtual bool paintMaxButton(QPainter &p);
    virtual bool paintOnAllDesktopsButton(QPainter &p) {return false;}
    virtual bool paintWindowMenuButton(QPainter &p) {return false;}
    virtual bool paintKeepAboveButton(QPainter &p) {return false;}
    virtual bool paintKeepBelowButton(QPainter &p) {return false;}
    virtual bool paintShadeButton(QPainter &p) {return false;}
    virtual bool paintResizeButton(QPainter &p) {return false;}
    virtual bool paintQuickHelpButton(QPainter &p) {return false;}
    virtual bool paintApplicationMenuButton(QPainter &p) {return false;}

    void drawBase(QColor c, QPainter &p, QRect &r) const;

    typedef bool (Button::*PaintEvent)(QPainter &);
    virtual bool isActive() = 0;
    virtual void onClick(QMouseEvent *e, const Type &t) = 0;
    static QPixmap sunkenized(const QRect &r, const QPixmap &source);

    PaintEvent m_paintEvent[TypeCount];
    Type m_type;
    bool m_hasPress;
    QPixmap m_bgPix;
};

#endif //WIDGETS_H
