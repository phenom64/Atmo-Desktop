#ifndef PROGRESSHANDLER_H
#define PROGRESSHANDLER_H

#include <QObject>
#include <QMap>

struct TimerData
{
    bool goingBack;
    int busyValue, timerId;
};

class QProgressBar;
class Q_DECL_EXPORT ProgressHandler : public QObject
{
    Q_OBJECT
public:
    explicit ProgressHandler(QObject *parent = 0);
    static ProgressHandler *instance();
    static void manage(QProgressBar *bar);
    static void release(QProgressBar *bar);
    static int busyValue(const QProgressBar *bar);

protected slots:
    void valueChanged();

protected:
    void checkBar(QProgressBar *bar);
    void initBar(QProgressBar *bar);
    bool eventFilter(QObject *, QEvent *);

private:
    static ProgressHandler m_instance;
    QList<QProgressBar *> m_bars;
    QMap<QProgressBar *, TimerData *> m_data;
};

#endif // PROGRESSHANDLER_H
