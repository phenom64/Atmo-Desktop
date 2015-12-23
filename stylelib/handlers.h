#ifndef HANDLERS_H
#define HANDLERS_H

#include <QWidget>
#include <QSharedMemory>
#include <QApplication>
#include <QMap>
#include <QLabel>
#include "gfx.h"
#include "widgets.h"

#if HASDBUS
#include <QDBusMessage>
#endif

class QToolBar;
class QMainWindow;
class QAbstractScrollArea;
class QTabBar;
class QToolButton;
class QDockWidget;

#define HASSTRETCH  "DSP_hasstretch"
#define TOOLPADDING "DSP_toolpadder"
#define TOOLTIMER   "DSP_toolbartimer"
#define TIMERNAME   "DSP_windowupdater"
#define MENUPRESS   "DSP_menupress"

namespace DSP
{

class Q_DECL_EXPORT Buttons : public QWidget
{
    Q_OBJECT
public:
    explicit Buttons(QWidget *parent = 0);
protected:
    bool eventFilter(QObject *, QEvent *);
};

class Q_DECL_EXPORT TitleWidget : public QWidget
{
    Q_OBJECT
public:
    enum TitlePos { Left = 0, Center, Right };
    explicit TitleWidget(QWidget *parent = 0);
    static bool supported(const QToolBar *toolBar);
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
};

namespace Handlers
{

template<typename T>
static T getChild(const qulonglong child)
{
    const QList<QWidget *> widgets = qApp->allWidgets();
    for (int i = 0; i < widgets.count(); ++i)
    {
        QWidget *w(widgets.at(i));
        if (reinterpret_cast<qulonglong>(w) == child)
            return static_cast<T>(w);
    }
    return 0;
}

class Q_DECL_EXPORT ToolBar : public QObject
{
    Q_OBJECT
public:
    ~ToolBar(){}
    static ToolBar *instance();
    static void manageToolBar(QToolBar *tb);
    static void manage(QWidget *child);
    static bool isArrowPressed(const QToolButton *tb);
    static void embedTitleWidgetLater(QToolBar *toolBar);
    static void adjustMargins(QToolBar *toolBar);
    static Sides sides(const QToolButton *btn);
    static void queryToolBarLater(QToolBar *bar, bool forceSizeUpdate = false);
    static bool isDirty(QToolBar *bar);
    static void setDirty(QToolBar *bar);
    static void fixSpacerLater(QToolBar *toolbar, int width = 7);

protected:
    ToolBar(QObject *parent = 0):QObject(parent){}
    void checkForArrowPress(QToolButton *tb, const QPoint pos);
    bool eventFilter(QObject *, QEvent *);
    static void unembed(QToolBar *bar);

protected slots:
    void toolBarMovableChanged(const bool movable);
    void toolBarFloatingChagned(const bool floating);
    void toolBarOrientationChagned(const Qt::Orientation o);
    void toolBarVisibilityChanged(const bool visible);
    void toolBarDeleted(QObject *toolBar);
    void toolBtnDeleted(QObject *toolBtn);
    void embedTitleWidget(qulonglong bar);
    void fixSpacer(qulonglong toolbar, int width = 7);
    void queryToolBar(qulonglong toolbar, bool forceSizeUpdate);
    void macMenuChanged();

private:
    static ToolBar *s_instance;
    static QMap<QToolButton *, Sides> s_sides;
    static QMap<QToolBar *, bool> s_dirty;
    static QMap<QToolBar *, QAction *> s_spacers;
};

enum UnoData { ToolBars = 0, ToolBarAndTabBar, TitleBar, Head, HeightCount };

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
    static bool drawUnoPart(QPainter *p, QRect r, const QWidget *w, QPoint offset = QPoint());
    static void updateWindowDataLater(QWidget *win);
    static void unoBg(QWidget *win, int &w, int h, const QPalette &pal, uchar *data);
    static QImage windowBg(const QSize &sz, const QColor &bgColor);
    static bool isActiveWindow(const QWidget *w);

public slots:
    void dataChanged(QDBusMessage msg);
    void updateWindowData(qulonglong window);
    void updateDecoBg(QWidget *w);

signals:
    void windowDataChanged(QWidget *win);

protected:
    Window(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);
    static unsigned int getHeadHeight(QWidget *win, bool &separator);

protected slots:
    void menuShow();

private:
    static Window *s_instance;
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
//    Drag(QObject *parent);
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
    static void drawContBg(QPainter *p, QWidget *w, const QRect r, const QPoint &offset = QPoint());
    static void detachMem(QMainWindow *win);

protected:
    ScrollWatcher(){}
    bool eventFilter(QObject *, QEvent *);
    void regenBg(QMainWindow *win);
    static QSharedMemory *mem(QMainWindow *win);

protected slots:
    void updateWin(QWidget *mainWin);
    void vpDeleted(QObject *vp);

private:
    static ScrollWatcher s_instance;
    static QMap<QMainWindow *, QSharedMemory *> s_mem;
};

class Q_DECL_EXPORT Balloon : public QWidget
{
    Q_OBJECT
public:
    Balloon();
    ~Balloon();
    void setToolTipText(const QString &text);
    inline QString toolTipText() const { return m_label->text(); }

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
    void updateShadow();
    void genPixmaps();

private:
    QLabel *m_label;
};

class Q_DECL_EXPORT BalloonHelper : public QObject
{
    Q_OBJECT
public:
    enum Mode { Left = 0, Top, Right, Bottom };
    static BalloonHelper *instance() { return &s_instance; }
    static void manage(QWidget *w);
    static void release(QWidget *w) { w->removeEventFilter(instance()); }
    static Mode mode();
    static void updateBallon();
    static Balloon *balloon();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    static BalloonHelper s_instance;
};

class Q_DECL_EXPORT Dock : public QObject
{
    Q_OBJECT
public:
    static void manage(QWidget *win);
    static void release(QWidget *win);
    static Dock *instance();

protected:
    void lockDock(QDockWidget *dock);
    void unlockDock(QDockWidget *dock);

protected slots:
    void lockWindowLater(QWidget *win);
    void lockDocks(const bool locked);

private:
    static Dock *s_instance;
};

} //namespace Handlers
} //namespace DSP

#endif //HANDLERS_H
