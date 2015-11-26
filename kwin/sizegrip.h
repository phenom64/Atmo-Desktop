#ifndef SIZEGRIP_H
#define SIZEGRIP_H

#include "kwinclient.h"

namespace DSP
{
class KwinClient;
class SizeGrip : public QWidget
{
    Q_OBJECT
public:
    SizeGrip(KwinClient *client = 0);

protected:
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void repos();
    int xPos() const;
    int yPos() const;
    QPoint thePos() const;

protected slots:
    void restack();

private:
    KwinClient *m_client;
};
}

#endif // SIZEGRIP_H
