#include <QStylePlugin>
#include <QWidget>
#include <QStyleOption>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QStyleOptionComboBox>
#include <QDebug>
#include <qmath.h>
#include <QEvent>
#include <QToolBar>
#include <QAbstractItemView>
#include <QMainWindow>
#include <QGroupBox>
#include <QStyleOptionProgressBarV2>
#include <QStyleOptionDockWidget>
#include <QStyleOptionDockWidgetV2>
#include <QProgressBar>
#include <QStyleOptionToolButton>
#include <QStyleOptionViewItemV4>

#include "styleproject.h"
#include "stylelib/progresshandler.h"
#include "stylelib/ops.h"
#include "config/settings.h"
#include "stylelib/render.h"

QRect
StyleProject::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    if (cc < CCSize && m_sc[cc])
        return (this->*m_sc[cc])(opt, sc, w);

    switch (cc)
    {
    default: return QCommonStyle::subControlRect(cc, opt, sc, w);
    }
}

QRect
StyleProject::subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget) const
{
    if (!opt)
        return QRect();
    switch (r)
    {
//    case SE_ToolBoxTabContents:
//        return opt->rect; //this is not called at all is it?
    case SE_ItemViewItemCheckIndicator:
    case SE_ItemViewItemDecoration:
    case SE_ItemViewItemText:
    {
        const QStyleOptionViewItemV4 *item = qstyleoption_cast<const QStyleOptionViewItemV4 *>(opt);
        if (!item)
            return QRect();

        const int m(2);
        QRect textRect(item->rect.adjusted(m, 0, -m, 0)), iconRect(textRect), checkRect(textRect);
        if (item->features & QStyleOptionViewItemV2::HasCheckIndicator)
        {
            int size(pixelMetric(PM_IndicatorWidth, item, widget));
            checkRect.setWidth(size);
            textRect.setLeft(checkRect.right()+m);
            iconRect.setLeft(checkRect.right()+m);
        }

        QSize iconSize(16, 16);
        if (qobject_cast<const QAbstractItemView *>(widget))
            if (static_cast<const QAbstractItemView *>(widget)->iconSize().isValid())
                iconSize = static_cast<const QAbstractItemView *>(widget)->iconSize();
        if (!item->icon.isNull())
        {
            switch (item->decorationPosition)
            {
            case QStyleOptionViewItem::Left:
                iconRect.setWidth(iconSize.width());
                textRect.setLeft(iconRect.right()+m);
                break;
            case QStyleOptionViewItem::Right:
                iconRect.setWidth(iconSize.width()+m);
                iconRect.moveLeft(item->rect.right()-(iconSize.width()+m));
                textRect.setRight(iconRect.left()-m);
                break;
            case QStyleOptionViewItem::Top:
                textRect.setHeight(opt->fontMetrics.height());
                textRect.moveTop(item->rect.bottom()-textRect.height());
                iconRect.setBottom(textRect.top()-m);
                break;
            case QStyleOptionViewItem::Bottom: // seriously? name one case?
                textRect.setHeight(item->fontMetrics.height());
                iconRect.setTop(textRect.bottom()+m);
                break;
            default : break;
            }
        }
        switch (r)
        {
        case SE_ItemViewItemCheckIndicator: return visualRect(item->direction, item->rect, checkRect);
        case SE_ItemViewItemDecoration: return visualRect(item->direction, item->rect, iconRect);
        case SE_ItemViewItemText: return visualRect(item->direction, item->rect, textRect);
        default: return QRect();
        }
    }
    case SE_LineEditContents:
    {
        QRect r(Render::maskRect(dConf.input.shadow, opt->rect));
        int h(r.height());
        int hor(qMin(dConf.input.rnd, (h/2))/2);
        r.adjust(hor, 0, -hor, 0);
        return r;
    }
    case SE_HeaderArrow:
    {
        castOpt(Header, hdr, opt);
        QRect r(opt->rect);
        const int extraMargin(16); // some space for good measure...
        const int margin(pixelMetric(PM_HeaderMarkSize)+extraMargin);
        if (hdr->textAlignment & Qt::AlignRight)
            r.setRight(r.left()+margin);
        else
            r.setLeft(r.right()-margin);
        return r;
    }
    case SE_HeaderLabel:
    {
        castOpt(Header, hdr, opt);
        QRect r(opt->rect);
        int space(0);
        if (opt->state & State_Sunken)
            space = pixelMetric(PM_HeaderMarkSize);

        if (hdr->textAlignment & Qt::AlignRight)
            r.setLeft(r.left()+space);
        else
            r.setRight(r.right()-space);

        int m(4);
        r.adjust(m, 0, -m, 0);
        return r;
    }
    case SE_TabBarTabLeftButton:
    case SE_TabBarTabRightButton:
    case SE_TabBarTabText:
    {
        const QStyleOptionTabV3 *tab = qstyleoption_cast<const QStyleOptionTabV3 *>(opt);
        const QTabBar *bar = qobject_cast<const QTabBar *>(widget);
        if (!tab)
            return QRect();

        const bool safBar(Ops::isSafariTabBar(bar));
        int o(safBar?dConf.tabs.safrnd:4);
        int ladd((safBar && (tab->position == QStyleOptionTab::Beginning || tab->position == QStyleOptionTab::OnlyOneTab))*o);
        int radd((safBar && (tab->position == QStyleOptionTab::End || tab->position == QStyleOptionTab::OnlyOneTab) && bar->expanding())*(o));
        QRect rect(tab->rect.adjusted(ladd, 0, -radd, 0));
        QRect textRect(rect);
        QRect leftRect;

        const bool east(tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast);
        const bool west(tab->shape == QTabBar::RoundedWest || tab->shape == QTabBar::TriangularWest);
        const bool leftClose(styleHint(SH_TabBar_CloseButtonPosition, opt, widget) == QTabBar::LeftSide);
        const bool closable(bar && bar->tabsClosable());

        if (tab->leftButtonSize.isValid() ||
                ((!closable || (closable && !leftClose)) && !tab->icon.isNull()))
        {
            const QSize sz(tab->leftButtonSize.isValid()?tab->leftButtonSize:tab->iconSize);
            leftRect.setSize(sz);
            leftRect.moveCenter(rect.center());
            if (west)
                leftRect.moveBottom(rect.bottom()-o);
            else if (east)
                leftRect.moveTop(rect.top()+o);
            else
                leftRect.moveLeft(rect.left()+o);
        }

        if (leftRect.isValid())
        {
            if (west)
                textRect.setBottom(leftRect.top());
            else if (east)
                textRect.setTop(leftRect.bottom());
            else
                textRect.setLeft(leftRect.right());
        }

        QRect rightRect;
        if (tab->rightButtonSize.isValid() ||
                (closable && leftClose && !tab->icon.isNull()))
        {
            const QSize sz(tab->leftButtonSize.isValid()?tab->leftButtonSize:tab->iconSize);
            rightRect.setSize(sz);
            rightRect.moveCenter(rect.center());
            if (west)
                rightRect.moveBottom(rect.bottom()-o);
            else if (east)
                rightRect.moveTop(rect.top()+o);
            else
                rightRect.moveRight(rect.right()-o);
        }

        if (rightRect.isValid())
        {
            if (west)
                textRect.setBottom(rightRect.top());
            else if (east)
                textRect.setTop(rightRect.bottom());
            else
                textRect.setRight(rightRect.left());
        }

        if (tab->text.isEmpty())
            textRect = QRect();

        int trans(0);
        if (bar->documentMode() && !safBar && !isVertical(tab, bar))
            trans = /*opt->state & State_Selected?-4:*/-2;

        switch (r)
        {
        case SE_TabBarTabLeftButton:
            return visualRect(tab->direction, tab->rect, leftRect).translated(0, trans);
        case SE_TabBarTabRightButton:
            return visualRect(tab->direction, tab->rect, rightRect).translated(0, trans);
        case SE_TabBarTabText:
            return visualRect(tab->direction, tab->rect, textRect).translated(0, trans);
        default: return QCommonStyle::subElementRect(r, opt, widget);
        }
    }
    case SE_ProgressBarLabel:
    case SE_ProgressBarGroove:
    case SE_ProgressBarContents: return opt->rect;
    case SE_RadioButtonIndicator:
    case SE_CheckBoxIndicator:
    {
        QRect r(opt->rect.topLeft(), QPoint(opt->rect.topLeft()+QPoint(pixelMetric(PM_IndicatorWidth), pixelMetric(PM_IndicatorHeight))));
        return visualRect(opt->direction, opt->rect, r);
    }
    case SE_RadioButtonContents:
    case SE_CheckBoxContents:
    {
        const int pmInd(pixelMetric(PM_IndicatorWidth)), pmSpc(pixelMetric(PM_CheckBoxLabelSpacing)), w(opt->rect.width()-(pmInd+pmSpc));
        return visualRect(opt->direction, opt->rect, QRect(opt->rect.left()+pmInd+pmSpc, opt->rect.top(), w, opt->rect.height()));
    }
    case SE_DockWidgetCloseButton:
    case SE_DockWidgetFloatButton:
    case SE_DockWidgetTitleBarText:
    case SE_DockWidgetIcon:
    {
        castOpt(DockWidget, dw, opt);
        if (!dw)
            return QRect();

        QRect rect(dw->rect);

        QRect cr, fr;
        int left(rect.left());
        if (dw->closable)
        {
            cr = rect;
            cr.setWidth(cr.height());
            cr.moveLeft(left);
            left += cr.width();
        }
        if (dw->floatable)
        {
            fr = rect;
            fr.setWidth(fr.height());
            fr.moveLeft(left);
            left += cr.width();
        }
        switch (r)
        {
        case SE_DockWidgetCloseButton:
            return visualRect(dw->direction, rect, cr);
        case SE_DockWidgetFloatButton:
            return visualRect(dw->direction, rect, fr);
        case SE_DockWidgetTitleBarText:
            return visualRect(dw->direction, rect, QRect(left, rect.top(), qMax(0, rect.width()-left), rect.height()));
        case SE_DockWidgetIcon:
            return QRect(); // ever needed?
        default: return QRect();
        }
    }
    default: break;
    }
    return QCommonStyle::subElementRect(r, opt, widget);
}

QRect
StyleProject::comboBoxRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
    const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt);
    if (!cb)
        return ret;
    const int arrowSize(20);
    const int m(cb->editable?Render::shadowMargin(dConf.input.shadow):0);
    switch (sc)
    {
    case SC_ComboBoxListBoxPopup: //what kinda rect should be returned here? seems only topleft needed...
    case SC_ComboBoxFrame: ret = cb->rect; break;
    case SC_ComboBoxArrow: ret = cb->rect.adjusted((cb->rect.width()-m)-arrowSize, 0, -m, 0); break;
    case SC_ComboBoxEditField: ret = cb->rect.adjusted(0, 0, -arrowSize, 0); break;
    default: ret = QRect(); break;
    }
    return visualRect(cb->direction, cb->rect, ret);
}

QRect
StyleProject::scrollBarRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
//    castOpt(Slider, slider, opt);
    const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt);
    if (!slider)
        return ret;

    QRect r(slider->rect);

    const int buttonSize(bool(dConf.scrollers.style)*16);
    bool hor(slider->orientation == Qt::Horizontal);
    if (hor)
        r.adjust(buttonSize, 0, -buttonSize, 0);
    else
        r.adjust(0, buttonSize, 0, -buttonSize);

    int grooveSize(hor ? r.width() : r.height());
    unsigned int range(slider->maximum-slider->minimum);
    int gs((int)((slider->pageStep*grooveSize)));
    int ss((range+slider->pageStep));
    int sliderSize(0);
    if (ss && gs)
        sliderSize = gs/ss;
    if (sliderSize < pixelMetric(PM_ScrollBarSliderMin, opt, w))
        sliderSize = pixelMetric(PM_ScrollBarSliderMin, opt, w);
    int pos = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition, grooveSize-sliderSize, slider->upsideDown);

    switch (sc)
    {
    case SC_ScrollBarAddLine:
        if (hor)
            ret = slider->rect.adjusted(slider->rect.width()-buttonSize, 0, 0, 0);
        else
            ret = slider->rect.adjusted(0, slider->rect.height()-buttonSize, 0, 0);
        break;
    case SC_ScrollBarSubLine:
        if (hor)
            ret = slider->rect.adjusted(0, 0, -(slider->rect.width()-buttonSize), 0);
        else
            ret = slider->rect.adjusted(0, 0, 0, -(slider->rect.height()-buttonSize));
        break;
    case SC_ScrollBarGroove: ret = r; break;
    case SC_ScrollBarSlider:
    {
        if (!range)
            break;
        if (hor)
            ret = QRect(r.left()+pos, 0, sliderSize, r.height());
        else
            ret = QRect(0, r.top()+pos, r.width(), sliderSize);
        break;
    }
    case SC_ScrollBarLast:
    case SC_ScrollBarAddPage:
        if (hor)
            ret = QRect(r.left()+pos+sliderSize, 0, r.width()-(pos+sliderSize), r.height());
        else
            ret = QRect(0, r.top()+pos+sliderSize, r.width(), r.height()-(pos+sliderSize));
        break;
    case SC_ScrollBarFirst:
    case SC_ScrollBarSubPage:
        if (hor)
            ret = QRect(0, 0, pos, r.height());
        else
            ret = QRect(0, 0, r.width(), pos);
        break;
    default: return QCommonStyle::subControlRect(CC_ScrollBar, opt, sc, w);
    }
    return visualRect(slider->direction, slider->rect, ret);
}

QRect
StyleProject::sliderRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
    castOpt(Slider, slider, opt);
    if (!slider)
        return ret;

    int tm(pixelMetric(PM_SliderTickmarkOffset, opt, w));
    const QSlider::TickPosition tp(slider->tickPosition);
    if (tp == QSlider::TicksBothSides)
        tm/=2;
    QRect r(slider->rect);
    bool hor(slider->orientation == Qt::Horizontal);
    if (slider->tickPosition)
    {
        const bool needLeft(!hor&&(tp==QSlider::TicksLeft||tp==QSlider::TicksBothSides)),
                needRight(!hor&&(tp==QSlider::TicksRight||tp==QSlider::TicksBothSides)),
                needTop(hor&&(tp==QSlider::TicksAbove||tp==QSlider::TicksBothSides)),
                needBottom(hor&&(tp==QSlider::TicksBelow||tp==QSlider::TicksBothSides));
        r.adjust(needLeft*tm, needTop*tm, -(needRight*tm), -(needBottom*tm));
    }
    int grooveSize(hor ? r.width() : r.height());
    unsigned int range(slider->maximum-slider->minimum);
    int sliderSize(pixelMetric(PM_SliderLength, opt, w));

    int pos = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition, grooveSize-sliderSize, slider->upsideDown);

    switch (sc)
    {
    case SC_SliderGroove: ret = r; break;
    case SC_SliderHandle:
    {
        if (!range)
            break;
        if (hor)
            ret = QRect(pos, r.height()/2-sliderSize/2, sliderSize, sliderSize);
        else
            ret = QRect(r.width()/2-sliderSize/2, pos, sliderSize, sliderSize);
        break;
    }
    default: return QCommonStyle::subControlRect(CC_ScrollBar, opt, sc, w);
    }
    return visualRect(slider->direction, slider->rect, ret);
}

QRect
StyleProject::groupBoxRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
    castOpt(GroupBox, box, opt);
    if (!box)
        return ret;
    ret = box->rect;
    const int top(qMax(16, opt->fontMetrics.height()));
    const int left(8);
    switch (sc)
    {
    case SC_GroupBoxCheckBox:
        ret = QRect(ret.left()+left, ret.top(), top, top);
        break;
    case SC_GroupBoxContents:
        ret.setTop(ret.top()+top);
        break;
    case SC_GroupBoxFrame:
        break;
    case SC_GroupBoxLabel:
        ret = QRect(ret.left()+left+(box->subControls&SC_GroupBoxCheckBox?top:0), ret.top(), ret.width()-top, top);
        break;
    default: return QCommonStyle::subControlRect(CC_GroupBox, opt, sc, w);
    }
    return visualRect(box->direction, opt->rect, ret);
}

QRect
StyleProject::toolButtonRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
    const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(opt);
    const QToolBar *bar(0);
    bool hor(true);
    if (w && qobject_cast<QToolBar *>(w->parentWidget()))
    {
        bar = static_cast<QToolBar *>(w->parentWidget());
        hor = bar->orientation()==Qt::Horizontal;
    }
    if (!tb)
        return ret;
    switch (sc)
    {
    case SC_ToolButton: ret = tb->rect; break;
    case SC_ToolButtonMenu:
    {
        ret = tb->rect;
        if (hor)
            ret.setLeft(ret.right()-16);
        else
            ret.setTop(ret.bottom()-16);
        break;
    }
    default: break;
    }
    return visualRect(tb->direction, tb->rect, ret);
}

QRect
StyleProject::spinBoxRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt);
    if (!sb)
        return QRect();

    QRect ret(sb->rect);
    int spinSize(pixelMetric(PM_SpinBoxSliderHeight, opt, w));
    QRect arrowUp(0, 0, spinSize, spinSize);
    QRect arrowDown(arrowUp);
    int sm(Render::shadowMargin(dConf.input.shadow));
//    int m((qMin(dConf.input.rnd, ret.height()/2)/2) + sm);
    int m(0);
    arrowUp.moveTopRight(ret.topRight()-QPoint(m, 0));
    arrowDown.moveBottomRight(ret.bottomRight()-QPoint(m, 0));
    switch (sc)
    {
    case SC_SpinBoxFrame:
        break;
    case SC_SpinBoxUp:
        ret = arrowUp;
        break;
    case SC_SpinBoxDown:
        ret = arrowDown;
        break;
    case SC_SpinBoxEditField:
        ret.setRight(arrowUp.left());
        break;
    default: break;
    }
    return visualRect(sb->direction, sb->rect, ret);
}

