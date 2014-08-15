#ifndef ANIMHANDLER_H
#define ANIMHANDLER_H

#include <QObject>
#include <QMap>
#include <QTimer>

namespace Anim
{

#define STEPS 16

class Q_DECL_EXPORT Basic : public QObject
{
    Q_OBJECT
public:
    explicit Basic(QObject *parent = 0);
    static Basic *instance();
    static void manage(QWidget *w);
    static int level(const QWidget *widget) { return instance()->hoverLevel(widget); }

protected:
    bool eventFilter(QObject *, QEvent *);
    int hoverLevel(const QWidget *widget);

protected slots:
    void removeSender();
    void animate();

private:
    static Basic *s_instance;
    QMap<QWidget *, int> m_vals;
    QTimer *m_timer;
};

}

#endif //ANIMHANDLER_H
