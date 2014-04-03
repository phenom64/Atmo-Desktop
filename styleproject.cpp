#include <QStylePlugin>

#include "styleproject.h"

class ProjectStylePlugin : public QStylePlugin
{
public:
    QStringList keys() const { return QStringList() << "StyleProject"; }
    QStyle *create(const QString &key)
    {
        if (key == "styleproject")
            return new StyleProject;
        return 0;
    }
};

Q_EXPORT_PLUGIN2(StyleProject, ProjectStylePlugin)
