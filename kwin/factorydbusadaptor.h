#ifndef FACTORYDBUSADAPTOR_H
#define FACTORYDBUSADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QObject>

#include "factory.h"


class FactoryDbusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.DBus.StyleProjectFactory")

public:
    FactoryDbusAdaptor(Factory *f) : QDBusAbstractAdaptor(f), m_f(f){}

public slots:
    Q_NOREPLY void update(unsigned int window) { m_f->update(window); }

private:
    Factory *m_f;
};

#endif // FACTORYDBUSADAPTOR_H
