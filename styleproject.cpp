#include <QStylePlugin>

#include "styleproject.h"
#include "render.h"

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

StyleProject::StyleProject()
{
    init();
    assignMethods();
    Render::generateData();
}

void
StyleProject::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_ce[element] && (this->*m_ce[element])(option, painter, widget)))
        QPlastiqueStyle::drawControl(element, option, painter, widget);
}

void
StyleProject::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_cc[control] && (this->*m_cc[control])(option, painter, widget)))
        QPlastiqueStyle::drawComplexControl(control, option, painter, widget);
}

void
StyleProject::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_pe[element] && (this->*m_pe[element])(option, painter, widget)))
        QPlastiqueStyle::drawPrimitive(element, option, painter, widget);
}

void
StyleProject::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    QPlastiqueStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

void
StyleProject::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    QPlastiqueStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

QPixmap
StyleProject::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
{
    return QPlastiqueStyle::generatedIconPixmap(iconMode, pixmap, opt);
}
