#include <QStylePlugin>
#include <QWidget>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QDebug>
#include <qmath.h>
#include <QEvent>
#include <QToolBar>

#include "styleproject.h"
#include "render.h"
#include "ops.h"

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
        QCommonStyle::drawControl(element, option, painter, widget);
}

void
StyleProject::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_cc[control] && (this->*m_cc[control])(option, painter, widget)))
        QCommonStyle::drawComplexControl(control, option, painter, widget);
}

void
StyleProject::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!(m_pe[element] && (this->*m_pe[element])(option, painter, widget)))
        QCommonStyle::drawPrimitive(element, option, painter, widget);
}

void
StyleProject::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    // we need to add either hide/show mnemonic, otherwise
    // we are rendering text w/ '&' characters.
    flags |= Qt::TextShowMnemonic; // Qt::TextHideMnemonic
    QCommonStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

void
StyleProject::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    QCommonStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

QPixmap
StyleProject::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const
{
    return QCommonStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

QSize
StyleProject::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *widget) const
{
    switch (ct)
    {
    case CT_ToolButton: return contentsSize+QSize(8, 8);
    case CT_MenuBarItem: return contentsSize+QSize(8, 0);
    default: break;
    }

    return QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
}

QRect
StyleProject::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    switch (cc)
    {
    default: break;
    }
    return QCommonStyle::subControlRect(cc, opt, sc, w);
}

int
StyleProject::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *shret) const
{
    switch (sh)
    {
    case SH_TabBar_Alignment:
    {
        if (Ops::isSafariTabBar(qobject_cast<const QTabBar *>(w)))
            return Qt::AlignLeft;
        return Qt::AlignCenter;
    }
    default: break;
    }
    return QCommonStyle::styleHint(sh, opt, w, shret);
}

bool
StyleProject::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type())
    {
    /* for some reason KTabWidget is an idiot and
     * doesnt use the style at all for painting, only
     * for the tabBar apparently (that inside the
     * KTabWidget) so we simply override the painting
     * for the KTabWidget and paint what we want
     * anyway.
     */
    case QEvent::Paint:
    {
        if (o->inherits("KTabWidget"))
        {
            QTabWidget *tabWidget = static_cast<QTabWidget *>(o);
            QPainter p(tabWidget);
            QStyleOptionTabWidgetFrameV2 opt;
            opt.initFrom(tabWidget);
            drawTabWidget(&opt, &p, tabWidget);
            p.end();
            return true;
        }
        break;
    }
    case QEvent::ActionChanged:
    {
        if (castObj(QToolBar *, toolBar, o))
            toolBar->update();
    }
    default: break;
    }
    return QCommonStyle::eventFilter(o, e);
}
