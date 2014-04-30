#ifndef WIDGETS_H
#define WIDGETS_H

#include "../kwin/kwinclient.h"
#include <QSizeGrip>

class Q_DECL_EXPORT SizeGrip : public QWidget
{
    Q_OBJECT
public:
    SizeGrip(KwinClient *parent = 0);

protected:
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

    void restack();
    void repos();

private:
    KwinClient *m_client;
};

#endif // WIDGETS_H
