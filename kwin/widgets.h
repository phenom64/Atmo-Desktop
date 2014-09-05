#ifndef WIDGETS_H
#define WIDGETS_H

#include "kwinclient.h"

class SizeGrip : public QWidget
{
    Q_OBJECT
public:
    SizeGrip(KwinClient *parent = 0);

protected:
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void repos();

protected slots:
    void restack();
    void checkSize();

private:
    KwinClient *m_client;
    QTimer *m_timer;
    QSize m_size;
};

#endif // WIDGETS_H
