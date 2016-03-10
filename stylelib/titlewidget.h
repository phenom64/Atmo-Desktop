#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>
#include "defines.h"
class QToolBar;
#if HASDBUS
#include <QDBusMessage>
#endif
namespace DSP
{
class Q_DECL_EXPORT TitleWidget : public QWidget
{
    Q_OBJECT
public:
    enum TitlePos { Left = 0, Center, Right };
    static void manage(QToolBar *toolBar);
    static bool isManaging(const QToolBar *toolBar);

protected:
    explicit TitleWidget(QToolBar *parent = 0);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void unembedLater();
    void embedLater();
    int visibleKids();
    QWidget *getCenterWidget();
    bool shouldEmbed();

    bool eventFilter(QObject *o, QEvent *e);

protected slots:
    void toolBarMovableChanged(const bool movable);
    void toolBarFloatingChagned(const bool floating);
    void toolBarOrientationChagned(const Qt::Orientation o);
    void toolBarVisibilityChanged(const bool visible);
    void embed();
    void unembed();
    void logicalToggle();
#if HASDBUS
    void macMenuChanged();
    void dataChanged(QDBusMessage msg);
#endif

private:
    qulonglong m_time;
    QToolBar *m_toolBar;
    QAction *m_action;
    bool m_unembedQueued, m_embedQueued, m_logicalQueued;
    int m_visibleCount;
};
}

#endif // TITLEWIDGET_H
