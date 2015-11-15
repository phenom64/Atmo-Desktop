#ifndef DECOADAPTOR_H
#define DECOADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QtGlobal>
#if QT_VERSION < 0x050000
#include "kwinclient.h"
#else
#include "kwinclient2.h"
#endif

namespace DSP
{
class DecoAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.dsp.deco")

public:
    DecoAdaptor(AdaptorManager *parent = 0) : QDBusAbstractAdaptor(parent), m_manager(parent){}

public slots:
    Q_NOREPLY void updateData(uint win) { m_manager->updateData(win); }
    Q_NOREPLY void updateDeco(uint win) { m_manager->updateDeco(win); }

signals:
     void windowActiveChanged(uint win, bool active);
     void dataChanged(uint win);

private:
    AdaptorManager *m_manager;
};
} //namespace

#endif //DECOADAPTOR_H
