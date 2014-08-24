#ifndef UNOHANDLER_H
#define UNOHANDLER_H

#include <QWidget>
#include <QTimer>
#include <QQueue>

class QToolBar;
class Q_DECL_EXPORT UNOHandler : public QObject
{
    Q_OBJECT
public:
    UNOHandler(QObject *parent = 0);
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
    bool eventFilter(QObject *, QEvent *);

private:
    static UNOHandler *s_instance;
    static QMap<int, QPixmap> s_pix;
};

#endif //UNOHANDLER_H
