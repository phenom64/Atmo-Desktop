#ifndef DECOADAPTOR_H
#define DECOADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QtGlobal>
#if QT_VERSION < 0x050000
#include "kwinclient.h"
#else
#include "kwinclient2.h"
#endif

class DecoAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.dsp.deco")

public:
    DecoAdaptor(DSP::AdaptorManager *parent = 0) : QDBusAbstractAdaptor(parent), m_manager(parent){}

public slots:
    Q_NOREPLY void updateData(uint win) { m_manager->updateData(win); }
    Q_NOREPLY void updateDeco(uint win) { m_manager->updateDeco(win); }

private:
    DSP::AdaptorManager *m_manager;
};

#endif //DECOADAPTOR_H
