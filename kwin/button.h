#ifndef BUTTON_H
#define BUTTON_H

#include <QWidget>
#include "kwinclient.h"

class Button : public QWidget
{
    Q_OBJECT
public:
    enum Type { Close, Min, Max };
    Button(Type type, KwinClient *client, QWidget *parent = 0);
    ~Button();

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    Type m_type;
    bool m_hasPress;
    KwinClient *m_client;
};

#endif //BUTTON_H
