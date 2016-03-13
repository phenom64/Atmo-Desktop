#include <QLineEdit>
#include <QTabBar>
#include <QStyleOption>
#include <QMainWindow>
#include <QComboBox>
#include <QAbstractButton>
#include <QLayout>
#include <QDockWidget>
#include <QGroupBox>
#include <QDebug>
#include <QProgressBar>
#include <QLabel>
#include <QStackedWidget>
#include <QAbstractScrollArea>
#include <QSplitter>
#include <QStyleOption>
#include <QToolBar>
#include <QMainWindow>
#include <QCalendarWidget>
//#include <QToolBar>

#include "dsp.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "config/settings.h"
#include "stylelib/gfx.h"
#include "stylelib/macros.h"
//#include "stylelib/handlers.h"
//#include "stylelib/windowdata.h"

using namespace DSP;

int
Style::layoutSpacingAndMargins(const QWidget *w)
{
    if (dConf.uno.enabled && w)
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(w->window()))
    if (QWidget *cw = mw->centralWidget())
    if (cw->isAncestorOf(w))
    {
        if (/*qobject_cast<const QAbstractScrollArea *>(w)
                || qobject_cast<const QSplitter *>(w)*/
                Overlay::overlay(w)
                || (qobject_cast<const QHBoxLayout *>(w->layout()) && w->findChild<const QSplitter *>() && w->children().count() == 2)) // <- maclike lego bricking for keepassx
            return 0;
    }
    if (qobject_cast<const QGroupBox *>(w))
        return 8;
    return 4;
}

int
Style::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric)
    {
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
    case PM_DefaultTopLevelMargin:
    case PM_DefaultChildMargin:
    case PM_DefaultLayoutSpacing:
        return 4;
    case PM_LayoutLeftMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
    case PM_LayoutTopMargin:
        return layoutSpacingAndMargins(widget);
    case PM_HeaderMarkSize:
        return 9;
    case PM_ButtonMargin:
        return 4;
    case PM_SpinBoxFrameWidth:
        return 0;
    case PM_SpinBoxSliderHeight:
        return widget?widget->height()/2:option->rect.height()/2;
    case PM_TabCloseIndicatorHeight:
    case PM_TabCloseIndicatorWidth:
        return 16;
    case PM_TabBarTabOverlap:	//19	Number of pixels the tabs should overlap. (Currently only used in styles, not inside of QTabBar)
        return Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget))?dConf.tabs.safrnd+4:0;
    case PM_TabBarTabHSpace:	//20	Extra space added to the tab width.
    {
//        const QTabBar *bar = qobject_cast<const QTabBar *>(widget);
//        const QStyleOptionTabV3 *tab = qstyleoption_cast<const QStyleOptionTabV3 *>(option);
//        if (tab && (!bar || !bar->expanding()))
//        {
//            const QString s(tab->text);
//            QFont f(bar ? bar->font() : qApp->font());
//            int nonBoldWidth(QFontMetrics(f).width(s));
//            f.setWeight(QFont::Black);
//            int boldWidth(QFontMetrics(f).width(s));
//            return (boldWidth-nonBoldWidth);
//        }
        return 0;
    }
    case PM_TabBarTabVSpace:	//21	Extra space added to the tab height.
        return 0;
//    case PM_TabBarBaseHeight:	//22	Height of the area between the tab bar and the tab pages.
//    case PM_TabBarBaseOverlap:	//23	Number of pixels the tab bar overlaps the tab bar base.
//    case PM_TabBarScrollButtonWidth:	//?
    case PM_TabBarTabShiftHorizontal:	//?	Horizontal pixel shift w hen a tab is selected.
    case PM_TabBarTabShiftVertical:
        return 0;
    case PM_IndicatorHeight:
    case PM_IndicatorWidth:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
    {
        static const int sm = GFX::shadowMargin(dConf.pushbtn.shadow);
        return 16+sm;
    }
    case PM_CheckBoxLabelSpacing:
    case PM_RadioButtonLabelSpacing:
        return 4;
    case PM_MenuVMargin: return 6;
    case PM_MenuHMargin: return 0;
    case PM_MenuBarPanelWidth:
    case PM_MenuPanelWidth: return 0;
    case PM_MenuBarItemSpacing: return 8;
    case PM_DockWidgetSeparatorExtent:
    case PM_SplitterWidth:
    {
        if (const QSplitter *splitter = qobject_cast<const QSplitter *>(widget))
        {
            for (int i = 0; i < splitter->count(); ++i)
            {
                const QWidget *w = splitter->widget(i);
                if (!w->isVisible())
                    continue;
                if (!Overlay::overlay(w, true))
                    return 8;
            }
        }
        if (widget && widget->parentWidget() && widget->parentWidget()->inherits("NavigationBar")) //qupzilla "toolbar" splitter
            return 8;
        return (dConf.uno.enabled && dConf.app != DSP::Settings::Eiskalt && qobject_cast<const QMainWindow *>(widget?widget->window():0)) ? 1 : 6;
    }
    case PM_DockWidgetTitleBarButtonMargin: return 0;
    case PM_DefaultFrameWidth:
    {
        if (!widget)
            return 2;
        if (Overlay::overlay(widget))
            return 0;
        if (qobject_cast<const QLineEdit *>(widget) || widget->isWindow())
            return 0;
        if (qobject_cast<const QCalendarWidget *>(widget))
            return 2;
        if (qobject_cast<const QGroupBox *>(widget))
            return 8;
        if (qobject_cast<const QAbstractScrollArea *>(widget))
            return 2;

        const QFrame *frame = qobject_cast<const QFrame *>(widget);
        if (frame && frame->frameShadow() == QFrame::Raised)
            return 8;
        if (option && option->state & State_Raised) //the buttons in qtcreator....
            return 0;

        if (dConf.uno.enabled)
        if (qobject_cast<const QTabWidget *>(widget))
        if (QMainWindow *mw = qobject_cast<QMainWindow *>(widget->window()))
        if (mw->centralWidget() && mw->centralWidget()->isAncestorOf(widget))
        if (!static_cast<const QFrame *>(widget)->frameStyle())
            return 0;
        return 2;
    }
    case PM_ComboBoxFrameWidth: return 0;
    case PM_ToolBarExtensionExtent: return dConf.arrowSize*2;
    case PM_ToolBarItemSpacing: return 0;
    case PM_ToolBarSeparatorExtent: return 8;
    case PM_ToolBarFrameWidth: return inUno(qobject_cast<QToolBar *>(const_cast<QWidget *>(widget)))?2:6;
    case PM_ToolBarHandleExtent:
    {
//        if (const QToolBar *bar = qobject_cast<const QToolBar *>(widget))
//        if (WindowData *data = WindowData::memory(bar->parentWidget()->winId(), bar->parentWidget()))
//            return data->value<uint>(WindowData::LeftEmbedSize, 0);
        return 9-GFX::shadowMargin(dConf.toolbtn.shadow);
    }
//    case PM_SliderThickness: return 12;
    case PM_ScrollBarExtent: if (dConf.scrollers.style) return dConf.scrollers.size; return Ops::isOrInsideA<const QDockWidget *>(widget)?(int)(dConf.scrollers.size*0.8f)|1:dConf.scrollers.size;
    case PM_SliderThickness:
    case PM_SliderLength:
    case PM_SliderControlThickness: return dConf.sliders.size/*qMin(option->rect.height(), option->rect.width())*/;
    case PM_SliderTickmarkOffset: return 8;
    case PM_ScrollBarSliderMin: return 32;
    case PM_ToolTipLabelFrameWidth:
        return 8;
#if QT_VERSION >= 0x050000
    case PM_ScrollView_ScrollBarOverlap:
    {
//        const QAbstractScrollArea *area = qobject_cast<const QAbstractScrollArea *>(widget&&widget->parent()?widget->parent()->parent():0);
//        if (!area || !area->cornerWidget())
//            return (bool)(dConf.scrollers.style == 0)*dConf.scrollers.size;
        return 0;
    }
#endif
    default: break;
    }
    return QCommonStyle::pixelMetric(metric, option, widget);
}
