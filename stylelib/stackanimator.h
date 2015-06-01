#ifndef STACKANIMATOR_H
#define STACKANIMATOR_H

#include <QObject>
#include <QPixmap>

class QStackedLayout;
class QTimer;
class QWidget;
class StackAnimator : public QObject
{
    Q_OBJECT
public:
    enum Info { Steps = 20 };
    StackAnimator(QObject *parent = 0);
    static void manage(QStackedLayout *l);

protected:
    bool eventFilter(QObject *o, QEvent *e);

protected slots:
    void currentChanged(int i);
    void animate();

private:
    QTimer *m_timer;
    QStackedLayout *m_stack;
    QPixmap m_prevPix, m_activePix, m_pix;
    QWidget *m_widget, *m_prevWidget;
    int m_step, m_prevIndex;
};

#endif //STACKANIMATOR_H
