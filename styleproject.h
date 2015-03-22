#ifndef STYLEPROJECT_H
#define STYLEPROJECT_H

#include <QCommonStyle>
#include <QStylePlugin>
#include <QEvent>
#include <QStyleOption>
#include "stylelib/macros.h"
#include "stylelib/ops.h"
#include "config/settings.h"

#define DEBUG 1

class QToolBar;
class QTabBar;
class QProgressBar;
class QStyleOptionTabV3;
class Q_DECL_EXPORT StyleProject : public QCommonStyle
{
    Q_OBJECT
    Q_CLASSINFO ("X-KDE-CustomElements", "true")
public:
    StyleProject();
    ~StyleProject();
    void init();
    void assignMethods();

    /* reimplemented functions
     * for now only some, we will
     * reimplement as we go
     */
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = 0) const;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;

    void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;
    void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    int styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w = 0, QStyleHintReturn *shret = 0) const;

    void polish(QWidget *widget);
    void polish(QPalette &);
    void polish(QApplication *app);
    void unpolish(QWidget *widget);

    bool eventFilter(QObject *, QEvent *);

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *widget = 0) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const;
    QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const;
    QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const;
    QPixmap standardPixmap(StandardPixmap sp, const QStyleOption *opt, const QWidget *widget) const;

    inline const QStyle *proxy() const { return this; }

    /* functions called for drawing */
    bool controlSkipper(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const { return true; }
    bool primitiveSkipper(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const { return true; }
    bool complexSkipper(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = 0) const { return true; }

    /* control elements */
    bool drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawCheckBox(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawCheckBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawRadioButton(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawRadioButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawToolButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawToolBar(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawTab(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawViewItem(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawViewItemBg(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawComboBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawProgressBar(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawProgressBarGroove(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawProgressBarContents(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawProgressBarLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawHeader(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawHeaderSection(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawHeaderLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawDockTitle(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawToolBoxTab(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawToolBoxTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawToolBoxTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;

    /* complex controls */
    bool drawToolButton(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawScrollBar(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawSlider(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawComboBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawSpinBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawGroupBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = 0) const;

    /* primitive elements */
    bool drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawStatusBar(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawSplitter(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawWindow(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawTabBar(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawTabWidget(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawTabCloser(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawMenu(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawScrollAreaCorner(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawToolTip(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawTree(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;
    bool drawMenuBar(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const;

    /* custom drawRoutines */
    bool drawSafariTab(const QStyleOptionTab *opt, QPainter *painter, const QTabBar *bar) const;
    bool drawSelector(const QStyleOptionTab *opt, QPainter *painter, const QTabBar *bar) const;

#define DRAWARROW(_VAR_) \
    bool drawArrow##_VAR_(const QStyleOption *option, QPainter *painter, const QWidget *widget = 0) const \
    { \
        Ops::drawArrow(painter, option->palette.color(Ops::fgRole(widget)), option->rect, Ops::_VAR_, dConf.arrowSize); \
        return true; \
    }
    /* Yes Thomas, macro-concept stolen from bespin */
    DRAWARROW(Left)
    DRAWARROW(Up)
    DRAWARROW(Right)
    DRAWARROW(Down)
#undef DRAWARROW

    /* events */
    bool paintEvent(QObject *o, QEvent *e);
    bool resizeEvent(QObject *o, QEvent *e);
    bool showEvent(QObject *o, QEvent *e);

    /* subcontrolrects */
    QRect scrollBarRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const;
    QRect comboBoxRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const;
    QRect sliderRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const;
    QRect groupBoxRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const;
    QRect toolButtonRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const;
    QRect spinBoxRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const;

    QRect progressContents(const QStyleOption *opt, const QWidget *widget = 0) const;

    /* pointers to these functions */
    typedef bool (StyleProject::*StyleComplexControl)(const QStyleOptionComplex *, QPainter *, const QWidget *) const;
    typedef bool (StyleProject::*StyleControl)(const QStyleOption *, QPainter *, const QWidget *) const;
    typedef bool (StyleProject::*StylePrimitive)(const QStyleOption *, QPainter *, const QWidget *) const;
    typedef bool (StyleProject::*EventFilter)(QObject *o, QEvent *e);
    typedef QRect (StyleProject::*SubControlRect)(const QStyleOptionComplex *, SubControl, const QWidget *) const;

protected:
    static bool isVertical(const QStyleOptionTabV3 *tab = 0, const QTabBar *tabBar = 0);
    static int layoutSpacingAndMargins(const QWidget *w);

private:
    StyleComplexControl m_cc[CCSize];
    StyleControl m_ce[CESize];
    StylePrimitive m_pe[PESize];
    EventFilter m_ev[EVSize];
    SubControlRect m_sc[CCSize];
};

class ProjectStylePlugin : public QStylePlugin
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "styleproject.json")
#endif
public:
    QStringList keys() const;
    QStyle *create(const QString &key);
};

#endif // STYLEPROJECT_H
