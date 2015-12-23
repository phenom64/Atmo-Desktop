#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>
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

public slots:
    void manageOverlay(QWidget *f);

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
    static Overlay *overlay(const QWidget *frame);
    static bool isSupported(const QFrame *f);
    inline Sides &sides() { return m_sides; }

protected:
    void paintEvent(QPaintEvent *);
    QRegion mask() const;
    bool eventFilter(QObject *o, QEvent *e);
    static QRect mappedRect(const QWidget *widget);
    bool frameIsInteresting(const QFrame *frame, const Position p) const;
    QFrame *getFrameForWidget(QWidget *w, const Position p) const;
    void removeSide(const Side s);
    void addSide(const Side s);

private slots:
    void updateOverlay();

private:
    int m_alpha;
    bool m_hasFocus;
    Sides m_sides;
    QWidget *m_frame;
    QWidget *m_window;
    QPoint m_position[PosCount];
};

}

#endif //OVERLAY_H
