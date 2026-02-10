#ifndef ATMO_DEBUG_H
#define ATMO_DEBUG_H

#include <QDebug>
#include <QProcessEnvironment>
#include <QString>

namespace NSE
{
inline bool atmoDebugEnabled()
{
    static const bool enabled = []() {
        const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        if (!env.contains(QStringLiteral("ATMO_DEBUG")))
            return false;
        const QString value = env.value(QStringLiteral("ATMO_DEBUG")).trimmed().toLower();
        return value.isEmpty()
            || value == QStringLiteral("1")
            || value == QStringLiteral("true")
            || value == QStringLiteral("on")
            || value == QStringLiteral("yes");
    }();
    return enabled;
}
}

#define ATMO_LOG if (!NSE::atmoDebugEnabled()) {} else qDebug().nospace() << "[ATMO] "

#endif // ATMO_DEBUG_H
