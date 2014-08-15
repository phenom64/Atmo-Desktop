#ifndef ANIMHANDLER_H
#define ANIMHANDLER_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QTabBar>
#include <QStyle>

namespace Anim
{

#define STEPS 16

Q_DECL_EXPORT void setStyle(QStyle *style);

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

class Q_DECL_EXPORT Tabs : public QObject
{
    Q_OBJECT
public:
    typedef int Tab, Level; //for readability only...
    explicit Tabs(QObject *parent = 0);
    static Tabs *instance();
    static void manage(QTabBar *tb);
    static int level(const QTabBar *tb, Tab tab) { return instance()->hoverLevel(tb, tab); }

protected:
    bool eventFilter(QObject *, QEvent *);
    int hoverLevel(const QTabBar *tb, Tab tab);

protected slots:
    void removeSender();
    void animate();

private:
    static Tabs *s_instance;
    QMap<QTabBar *, QMap<Tab, Level> > m_vals;
    QTimer *m_timer;
};

}

#endif //ANIMHANDLER_H
