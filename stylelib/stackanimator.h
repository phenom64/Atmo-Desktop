#ifndef STACKANIMATOR_H
#define STACKANIMATOR_H

#include <QObject>
#include <QPixmap>

class QStackedLayout;
class QTimer;
class QWidget;
class Q_DECL_EXPORT StackAnimator : public QObject
{
    Q_OBJECT
public:
    enum Info { Steps = 10 };
    StackAnimator(QObject *parent = 0);
    static void manage(QStackedLayout *l);

protected:
    bool eventFilter(QObject *o, QEvent *e);

protected slots:
    void currentChanged(int i);
    void animate();
    void activate();

private:
    QTimer *m_timer;
    QStackedLayout *m_stack;
    QPixmap m_prevPix, m_activePix, m_pix;
    QWidget *m_widget;
    int m_step, m_prevIndex;
    bool m_isActivated;
};

#endif //STACKANIMATOR_H
