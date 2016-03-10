#ifndef WINDOWHELPERS_H
#define WINDOWHELPERS_H

#include <QObject>
#include "defines.h"

#if HASDBUS
#include <QDBusMessage>
#endif

namespace DSP
{
class Q_DECL_EXPORT WindowHelpers : public QObject
{
    Q_OBJECT
public:
    static WindowHelpers *instance();
    static void updateWindowDataLater(QWidget *win);
    static bool isActiveWindow(const QWidget *w);
    static int unoHeight(QWidget *win, bool includeClientPart = true, bool includeTitleBar = false);

protected:
    explicit WindowHelpers(QObject *parent = 0);
    static bool scheduleWindow(const qulonglong w);
    static unsigned int getHeadHeight(QWidget *win, bool &separator);
    static void unoBg(QWidget *win, int &w, int h, const QPalette &pal, uchar *data);

signals:
    void windowDataChanged(QWidget *win);

protected slots:
    void updateWindowData(qulonglong window);
#if HASDBUS
    void dataChanged(QDBusMessage msg);
#endif

private:
    static WindowHelpers *s_instance;
    QList<qulonglong> m_scheduled;
    QMap<QWidget *, int> m_uno;
};
}

#endif // WINDOWHELPERS_H
