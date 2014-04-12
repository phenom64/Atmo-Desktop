
#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QResizeEvent>
#include <QTabBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <qmath.h>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QMap>
#include <QStackedWidget>
#include <QMainWindow>

class OverLay : public QWidget
{
    Q_OBJECT
public:
    enum Side { Left = 0x1, Top = 0x2, Right = 0x4, Bottom = 0x8, All = 0xf };
    enum Position { West = 0, North = 1, East = 2, South = 3, PosCount = 4 };
    typedef uint Sides;
    OverLay(QFrame *parent = 0, int opacity = 0);
    static bool manage(QFrame *frame, int opacity);
    inline Sides lines() { return m_lines; }

protected:
    void paintEvent(QPaintEvent *);
    QRegion mask() const;
    bool eventFilter(QObject *o, QEvent *e);
    void parentChanged();
    static QRect mappedRect(const QWidget *widget);
    bool frameIsInteresting(const QFrame *frame, const Position &pos) const;
    QFrame *getFrameForWidget(QWidget *w, const Position &pos) const;

private slots:
    void updateOverlay();

private:
    int m_alpha;
    bool m_hasFocus;
    Sides m_lines;
    QFrame *m_frame;
    QTimer *m_timer;
    QWidget *m_window;
    QPoint m_position[PosCount];
};

#endif //OVERLAY_H
