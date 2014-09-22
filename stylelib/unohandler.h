#ifndef UNOHANDLER_H
#define UNOHANDLER_H

#include <QWidget>
#include "render.h"
#include "widgets.h"

class DButton : public Button
{
public:
    DButton(const Type &t, QWidget *parent = 0) : Button(t, parent){}
protected:
    bool isActive() { return window()->isActiveWindow(); }
    void onClick(QMouseEvent *e, const Type &t);
};

class Buttons : public QWidget
{
    Q_OBJECT
public:
    explicit Buttons(QWidget *parent = 0);
protected:
    bool eventFilter(QObject *, QEvent *);
};

class TitleWidget : public QWidget
{
    Q_OBJECT
public:
    enum TitlePos { Left = 0, Center, Right };
    explicit TitleWidget(QWidget *parent = 0):QWidget(parent){}
protected:
    void paintEvent(QPaintEvent *);
};

class QToolBar;
class QMainWindow;
class Q_DECL_EXPORT UNOHandler : public QObject
{
    Q_OBJECT
public:
    ~UNOHandler(){}
    static UNOHandler *instance();
    static void manage(QWidget *mw);
    static void release(QWidget *mw);
    static void fixWindowTitleBar(QWidget *win);
    static void updateWindow(WId window);
    static void updateToolBar(QToolBar *toolBar);
    static void fixTitleLater(QWidget *win);
    static bool drawUnoPart(QPainter *p, QRect r, const QWidget *w, int offset = 0, float opacity = 1.0f);
    static void setupNoTitleBarWindow(QToolBar *toolBar);

public slots:
    void fixTitle();

protected:
    UNOHandler(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);

private:
    static UNOHandler s_instance;
    static QMap<int, QPixmap> s_pix;
};

class Q_DECL_EXPORT WinHandler : public QObject
{
    Q_OBJECT
public:
    ~WinHandler(){}
    static WinHandler *instance();
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static bool canDrag(QWidget *w);
    static void addCompactMenu(QWidget *w);

protected:
    WinHandler(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);

protected slots:
    void menuShow();

private:
    static WinHandler s_instance;
    QList<QWidget *> m_menuWins;
};

class QAbstractScrollArea;
class Q_DECL_EXPORT ScrollWatcher : public QObject
{
    Q_OBJECT
public:
    ~ScrollWatcher(){}
    static void watch(QAbstractScrollArea *area);
    static inline ScrollWatcher *instance() { return &s_instance; }
    static void regenBg(QMainWindow *win);
    static QPixmap bg(qlonglong win);
    static QRegion paintRegion(QMainWindow *win);

protected:
    ScrollWatcher(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);

protected slots:
    void updateWin(QMainWindow *win);
    void updateLater();
    void removeFromQueue();

private:
    static ScrollWatcher s_instance;
    static QMap<qlonglong, QPixmap> s_winBg;
    QTimer *m_timer;
    QList<QMainWindow *> m_wins;
};

#endif //UNOHANDLER_H
