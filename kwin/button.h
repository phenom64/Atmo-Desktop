#ifndef BUTTON_H
#define BUTTON_H

#include <QWidget>
#include "kwinclient.h"

class Button : public QWidget
{
    Q_OBJECT
public:
    enum Type { Close, Min, Max, TypeCount };
    Button(Type type, KwinClient *client, QWidget *parent = 0);
    ~Button();

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    bool paintClose();
    bool paintMin();
    bool paintMax();

    void drawBase(QColor c, QPainter &p, QRect &r) const;

    typedef bool (Button::*PaintEvent)();

private:
    PaintEvent m_paintEvent[TypeCount];
    Type m_type;
    bool m_hasPress;
    KwinClient *m_client;
};

#endif //BUTTON_H
