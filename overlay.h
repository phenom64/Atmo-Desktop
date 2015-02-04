#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>

class QFrame;
class QScrollBar;
class OverLay;

class OverLayHandler : public QObject
{
    Q_OBJECT
public:
    static OverLayHandler *instance() { return &s_instance; }
    static void manage(OverLay *o);

protected:
    bool eventFilter(QObject *, QEvent *);

protected slots:
    void overLayDeleted();

private:
    static OverLayHandler s_instance;
};

class OverLay : public QWidget
{
    Q_OBJECT
public:
    enum Side { Left = 0x1, Top = 0x2, Right = 0x4, Bottom = 0x8, All = 0xf };
    enum Position { West = 0, North = 1, East = 2, South = 3, PosCount = 4 };
    typedef uint Sides;
    ~OverLay();
    static bool manage(QFrame *frame, int opacity);
    static bool release(QFrame *frame);
    static bool hasOverLay(const QFrame *frame);
    inline Sides lines() { return m_lines; }

protected:
    OverLay(QFrame *parent = 0, int opacity = 0);
    void paintEvent(QPaintEvent *);
    QRegion mask() const;
    bool eventFilter(QObject *o, QEvent *e);
    void parentChanged();
    static QRect mappedRect(const QWidget *widget);
    bool frameIsInteresting(const QFrame *frame, const Position pos) const;
    QFrame *getFrameForWidget(QWidget *w, const Position pos) const;

private slots:
    void updateOverlay();

private:
    int m_alpha;
    bool m_hasFocus;
    Sides m_lines;
    QFrame *m_frame;
    QWidget *m_window;
    QPoint m_position[PosCount];
};

#endif //OVERLAY_H
