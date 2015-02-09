#ifndef HANDLERS_H
#define HANDLERS_H

#include <QWidget>
#include <QSharedMemory>
#include <QMap>
#include <QLabel>
#include "render.h"
#include "widgets.h"

class QToolBar;
class QMainWindow;
class QAbstractScrollArea;
class QTabBar;
class QToolButton;

#define HASSTRETCH  "DSP_hasstretch"
#define CSDBUTTONS  "DSP_hasbuttons"
#define TOOLPADDING "DSP_toolpadder"
#define TOOLTIMER   "DSP_toolbartimer"
#define TIMERNAME   "DSP_windowupdater"
#define MENUPRESS   "DSP_menupress"

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
    static bool isArrowPressed(const QToolButton *tb);
    static void setupNoTitleBarWindowLater(QToolBar *toolBar);
    static void adjustMargins(QToolBar *toolBar);
    static Render::Sides sides(const QToolButton *btn);
    static void processToolBar(QToolBar *bar, bool forceSizeUpdate = false);
    static bool isDirty(QToolBar *bar);

protected:
    ToolBar(QObject *parent = 0):QObject(parent){}
    void checkForArrowPress(QToolButton *tb, const QPoint pos);
    bool eventFilter(QObject *, QEvent *);

protected slots:
    void adjustMarginsSlot();
    void toolBarDeleted(QObject *toolBar);
    void toolBtnDeleted(QObject *toolBtn);
    void setupNoTitleBarWindow(qulonglong bar);
    void fixSpacer(QWidget *toolbar);

private:
    static ToolBar s_instance;
    static QMap<QToolButton *, Render::Sides> s_sides;
    static QMap<QToolBar *, bool> s_dirty;
    static QMap<QToolBar *, QAction *> s_spacers;
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
    static QMap<QWidget *, QPixmap> s_bgPix;
    ~Window(){}
    static Window *instance();
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static void addCompactMenu(QWidget *w);
    static bool drawUnoPart(QPainter *p, QRect r, const QWidget *w, QPoint offset = QPoint(), float opacity = 1.0f);
    static void updateDeco(WId window, unsigned int changed = 63);
    static void updateWindowDataLater(QWidget *win);

public slots:
    void updateWindowData(qulonglong window);

signals:
    void windowDataChanged(QWidget *win);

protected:
    Window(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);
    static unsigned int getHeadHeight(QWidget *win, unsigned int &needSeparator);

protected slots:
    void menuShow();

private:
    static Window s_instance;
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
    void updateWin(QWidget *mainWin);
    void vpDeleted(QObject *vp);

private:
    static ScrollWatcher s_instance;
    static QMap<qlonglong, QImage> s_winBg;
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

}

#endif //HANDLERS_H
