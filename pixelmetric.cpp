#include <QLineEdit>
#include <QTabBar>
#include <QStyleOption>
#include <QMainWindow>
#include <QComboBox>
#include <QAbstractButton>
#include <QLayout>
#include <QDockWidget>
#include <Q3GroupBox>
#include <QDebug>
#include <QProgressBar>
#include <QLabel>
#include <QStackedWidget>
#include <QAbstractScrollArea>
#include <QSplitter>
#include <QStyleOption>

#include "styleproject.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "config/settings.h"
#include "stylelib/render.h"
#include "stylelib/macros.h"

int
StyleProject::layoutSpacingAndMargins(const QWidget *w)
{
    if (dConf.uno.enabled && w)
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(w->window()))
    if (QWidget *cw = mw->centralWidget())
    if (cw->isAncestorOf(w))
    {
        if (qobject_cast<const QAbstractScrollArea *>(w) ||
                qobject_cast<const QSplitter *>(w) ||
                (qobject_cast<const QHBoxLayout *>(w->layout()) && w->findChild<const QSplitter *>() && w->children().count() == 2)) // <- maclike lego bricking for keepassx
            return 0;
#if 0
        bool hasClickables(false);
        const QList<QWidget *> kids(widget->findChildren<QWidget *>());
        for (int i = 0; i < kids.count(); ++i)
        {
            const QWidget *w = kids.at(i);
            if (!w->isVisibleTo(cw) || w->parentWidget() != widget)
                continue;
            hasClickables |= qobject_cast<const QAbstractButton *>(w) ||
                    qobject_cast<const QComboBox *>(w) ||
                    qobject_cast<const QAbstractSlider *>(w) ||
                    qobject_cast<const QGroupBox *>(w) ||
                    qobject_cast<const QLineEdit *>(w) ||
                    qobject_cast<const QProgressBar *>(w) ||
                    qobject_cast<const QLabel *>(w) ||
                    qobject_cast<const QTabWidget *>(w) ||
                    w->inherits("KTitleWidget") ||
                    qobject_cast<const QBoxLayout *>(w->layout()); //widget w/ possible clickables...
            if (hasClickables) //one is enough
                break;
        }
        if (!hasClickables)
            return 0;
#endif
    }
    if (qobject_cast<const QGroupBox *>(w))
        return 8;
    return 4;
}

int
StyleProject::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric)
    {
    case PM_DefaultTopLevelMargin:
        return 4;
    case PM_DefaultChildMargin:
        return 4;
    case PM_DefaultLayoutSpacing:
        return 4;
    case PM_LayoutLeftMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
        return layoutSpacingAndMargins(widget);
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        return 4;
    case PM_HeaderMarkSize:
        return 9;
    case PM_ButtonMargin:
        return 4;
    case PM_SpinBoxFrameWidth:
        return 0;
    case PM_SpinBoxSliderHeight:
        return option->rect.height()/2;
    case PM_TabCloseIndicatorHeight:
    case PM_TabCloseIndicatorWidth:
        return 16;
    case PM_TabBarTabOverlap:	//19	Number of pixels the tabs should overlap. (Currently only used in styles, not inside of QTabBar)
        return Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget))?dConf.tabs.safrnd+4:0;
    case PM_TabBarTabHSpace:	//20	Extra space added to the tab width.
//        if (Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget)))
//        {
//            castOpt(TabV3, tab, option);
//            if (tab && tab->position == QStyleOptionTab::Beginning)
//                return 128;
//        }
        return 16;
    case PM_TabBarTabVSpace:	//21	Extra space added to the tab height.
        return Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget))?8:4;
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
        return 16;
    case PM_CheckBoxLabelSpacing:
    case PM_RadioButtonLabelSpacing:
        return 4;
    case PM_MenuVMargin: return 6;
    case PM_MenuHMargin: return 0;
    case PM_MenuBarPanelWidth:
    case PM_MenuPanelWidth: return 0;
    case PM_MenuBarItemSpacing: return 8;
    case PM_DockWidgetSeparatorExtent:;
    case PM_SplitterWidth:
        return dConf.uno.enabled&&dConf.app!=Settings::Eiskalt?1:4;
    case PM_DockWidgetTitleBarButtonMargin: return 0;
    case PM_DefaultFrameWidth:
    {
        if (!widget)
            return 2;
        if (qobject_cast<const QLineEdit *>(widget) || widget->isWindow())
            return 0;
        if (qobject_cast<const QGroupBox *>(widget))
            return 8;

        const QFrame *frame = qobject_cast<const QFrame *>(widget);
        if (OverLay::hasOverLay(frame))
            return 0;

        if (frame && frame->frameShadow() == QFrame::Raised)
            return 8;
        if (option && option->state & State_Raised) //the buttons in qtcreator....
            return 0;

        if (dConf.uno.enabled)
        if (qobject_cast<const QTabWidget *>(widget))
        if (QMainWindow *mw = qobject_cast<QMainWindow *>(widget->window()))
        if (mw->centralWidget() && mw->centralWidget()->isAncestorOf(widget))
        if (!static_cast<const QFrame *>(widget)->frameStyle())
        {
            return 0;
        }
        return (frame&&frame->frameShadow()==QFrame::Sunken||!dConf.uno.enabled)*2;
    }
    case PM_ComboBoxFrameWidth: return 0;
    case PM_ToolBarItemSpacing: return 0;
    case PM_ToolBarSeparatorExtent: return 8;
    case PM_ToolBarFrameWidth: return 2;
    case PM_ToolBarHandleExtent: return 9-Render::shadowMargin(dConf.toolbtn.shadow);
//    case PM_SliderThickness: return 12;
    case PM_ScrollBarExtent: return dConf.scrollers.size;
    case PM_SliderThickness:
    case PM_SliderLength:
    case PM_SliderControlThickness: return dConf.sliders.size/*qMin(option->rect.height(), option->rect.width())*/;
    case PM_SliderTickmarkOffset: return 8;
    case PM_ScrollBarSliderMin: return 32;
    case PM_ToolTipLabelFrameWidth:
        return 8;
    default: break;
    }
    return QCommonStyle::pixelMetric(metric, option, widget);
}
