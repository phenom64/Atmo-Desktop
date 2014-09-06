#ifndef UNOHANDLER_H
#define UNOHANDLER_H

#include <QWidget>
#include "render.h"

class Button : public QWidget
{
    Q_OBJECT
public:
    enum Type { Close, Min, Max, TypeCount };
    Button(Type type, QWidget *parent = 0);
    ~Button();

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    bool paintClose(QPainter &p);
    bool paintMin(QPainter &p);
    bool paintMax(QPainter &p);

    void drawBase(QColor c, QPainter &p, QRect &r) const;

    typedef bool (Button::*PaintEvent)(QPainter &);

private:
    PaintEvent m_paintEvent[TypeCount];
    Type m_type;
    bool m_hasPress;
};

class Buttons : public QWidget
{
    Q_OBJECT
public:
    explicit Buttons(QWidget *parent = 0);
protected:
    bool eventFilter(QObject *, QEvent *);
};

class QToolBar;
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

protected:
    WinHandler(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);

private:
    static WinHandler s_instance;
};

#endif //UNOHANDLER_H
