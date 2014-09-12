#ifndef WIDGETS_H
#define WIDGETS_H

#include <QWidget>

class Q_DECL_EXPORT Button : public QWidget
{
public:
    enum Type { Close, Min, Max, TypeCount };
    Button(Type type, QWidget *parent = 0);
    ~Button();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    bool paintClose(QPainter &p);
    bool paintMin(QPainter &p);
    bool paintMax(QPainter &p);

    void drawBase(QColor c, QPainter &p, QRect &r) const;

    typedef bool (Button::*PaintEvent)(QPainter &);
    virtual bool isActive() = 0;
    virtual void onClick(QMouseEvent *e, const Type &t) = 0;

private:
    PaintEvent m_paintEvent[TypeCount];
    Type m_type;
    bool m_hasPress;
};

#endif //WIDGETS_H
