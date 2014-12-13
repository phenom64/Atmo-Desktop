#ifndef UNOHANDLER_H
#define UNOHANDLER_H

#include <QWidget>
#include <QSharedMemory>
#include <QMap>
#include "render.h"
#include "widgets.h"

class QToolBar;
class QMainWindow;
class QAbstractScrollArea;
class QTabBar;
class QToolButton;

class DButton : public Button
{
public:
    DButton(const Type &t, QWidget *parent = 0) : Button(t, parent){}
protected:
    bool isActive() const { return window()->isActiveWindow(); }
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

namespace Handlers
{

class Q_DECL_EXPORT ToolBar : public QObject
{
    Q_OBJECT
public:
    ~ToolBar(){}
    static ToolBar *instance();
    static void manage(QToolBar *tb);
    static void manage(QToolButton *tb);
    static void updateToolBarLater(QToolBar *bar, const int time = 250);
    static bool isArrowPressed(const QToolButton *tb);
    static void setupNoTitleBarWindow(QToolBar *toolBar);
    static void unoFyToolBar(QToolBar *toolBar);

protected:
    ToolBar(QObject *parent = 0):QObject(parent){}
    void forceButtonSizeReRead(QToolBar *bar);
    void checkForArrowPress(QToolButton *tb, const QPoint pos);
    bool eventFilter(QObject *, QEvent *);

protected slots:
    void unoFyToolBarSlot();
    void updateToolBar();

private:
    static ToolBar s_instance;
};

enum UnoData { ToolBars = 0, ToolBarAndTabBar, TitleBar, All, HeightCount };

class Q_DECL_EXPORT Data
{
public:
    Data(int *heightData = 0, QTabBar *saftb = 0)
        : possibleSafTabBar(saftb)
    {
        if (heightData)
            for (int i = 0; i < HeightCount; ++i)
                height[i]=heightData[i];
    }
    ~Data(){}
    int height[HeightCount];
    QTabBar *possibleSafTabBar;
};

class Q_DECL_EXPORT Window : public QObject
{
    Q_OBJECT
public:
    static QMap<QWidget *, Handlers::Data> s_unoData;
    ~Window(){}
    static Window *instance();
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static void addCompactMenu(QWidget *w);
    static void fixWindowTitleBar(QWidget *win);
    static bool drawUnoPart(QPainter *p, QRect r, const QWidget *w, const QPoint &offset = QPoint(), float opacity = 1.0f);
    static void updateWindow(WId window, unsigned int changed = 63);
    static void fixTitleLater(QWidget *win);

public slots:
    void fixTitle();

protected:
    Window(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);
    static unsigned int getHeadHeight(QWidget *win, unsigned int &needSeparator);

protected slots:
    void menuShow();
    void cleanUp();

private:
    static Window s_instance;
    static QMap<uint, QVector<QPixmap> > s_pix;
    QList<QWidget *> m_menuWins;
};

static int unoHeight(const QWidget *w, const UnoData d)
{
    int i(0);
    if (Window::s_unoData.contains(const_cast<QWidget *>(w)))
        i = Window::s_unoData.value(const_cast<QWidget *>(w)).height[d];
    return i;
}

class Q_DECL_EXPORT Drag : public QObject
{
    Q_OBJECT
public:
    static Drag *instance();
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static bool canDrag(QWidget *w);
    ~Drag(){}
protected:
    Drag(QObject *parent = 0):QObject(parent){}
    bool eventFilter(QObject *, QEvent *);
private:
    static Drag s_instance;
};

class Q_DECL_EXPORT ScrollWatcher : public QObject
{
    Q_OBJECT
public:
    ~ScrollWatcher(){}
    static void watch(QAbstractScrollArea *area);
    static inline ScrollWatcher *instance() { return &s_instance; }
    void regenBg(QMainWindow *win);
    static QImage bg(qlonglong win);
    static void detachMem(QMainWindow *win);
    static bool hasBg(qulonglong win) { return s_winBg.contains(win); }

protected:
    explicit ScrollWatcher(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);

protected slots:
    void updateWin(QMainWindow *win);
    void updateLater();

signals:
    void updateRequest();

private:
    static ScrollWatcher s_instance;
    static QMap<qlonglong, QImage> s_winBg;
    static QMap<QMainWindow *, QSharedMemory *> s_mem;
};

}

#endif //UNOHANDLER_H
