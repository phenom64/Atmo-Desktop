#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>
#include <QTimer>
#include "namespace.h"

class QFrame;
class QScrollBar;

namespace DSP
{
class Overlay;
class OverlayHandler : public QObject
{
    Q_OBJECT
public:
    static OverlayHandler *instance();
    static void manage(Overlay *o);

protected:
    bool eventFilter(QObject *, QEvent *);

protected slots:
    void overlayDeleted();

private:
    static OverlayHandler *s_instance;
};

class Overlay : public QWidget
{
    Q_OBJECT
public:
    Overlay(QWidget *parent = 0, int opacity = 0);
    ~Overlay();
    static bool manage(QWidget *frame, int opacity);
    static bool release(QWidget *frame);
    static Overlay *overlay(const QWidget *frame, const bool recursive = false);
    static bool isSupported(const QWidget *frame);
    inline Sides &sides() { return m_sides; }

protected:
    void paintEvent(QPaintEvent *);
    QRegion mask() const;
    bool eventFilter(QObject *o, QEvent *e);
    void removeSide(const Side s);
    void addSide(const Side s);
    bool isSplitter(QWidget *w, const Position p);

private slots:
    void updateOverlay();

private:
    int m_alpha;
    bool m_hasFocus, m_shown;
    Sides m_sides;
    QWidget *m_frame;
    QWidget *m_window;
    QPoint m_position[PosCount];
};

class Restorer : public QTimer
{
    Q_OBJECT
public:
    Restorer(qulonglong widget);
public slots:
    void restore();
private:
    qulonglong m_widget;

};

}

#endif //OVERLAY_H
