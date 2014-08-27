#ifndef UNOHANDLER_H
#define UNOHANDLER_H

#include <QWidget>

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
    static bool drawUnoPart(QPainter *p, QRect r, const QWidget *w, int offset = 0);

public slots:
    void fixTitle();

protected:
    UNOHandler(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);

private:
    static UNOHandler *s_instance;
    static QMap<int, QPixmap> s_pix;
};

class Q_DECL_EXPORT WinHandler : public QObject
{
    Q_OBJECT
public:
    ~WinHandler(){}
    static WinHandler *instance();
    static void manage(QWidget *win);
    static void release(QWidget *win);

protected:
    WinHandler(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);

private:
    static WinHandler *s_instance;
    bool m_hasDrag;
    QWidget *m_hasPress;
    QPoint m_presPos;
};

#endif //UNOHANDLER_H
