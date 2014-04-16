#ifndef STYLEPROJECT_H
#define STYLEPROJECT_H

#include <QCommonStyle>
#include <QEvent>

#define CCSize CC_MdiControls+1
#define CESize CE_ShapedFrame+1
#define PESize PE_PanelMenu+1
#define EVSize QEvent::PlatformPanel+1

#define SUNKEN state & (State_Sunken | State_Selected | State_On)
#define HOVER state & State_MouseOver
#define ENABLED state & State_Enabled

#define castOpt(_Type_, _varName_, _fromVar_) const QStyleOption##_Type_ *_varName_ = qstyleoption_cast<const QStyleOption##_Type_ *>(_fromVar_)
#define castObj(_Type_, _varName_, _fromVar_) _Type_ _varName_ = qobject_cast<_Type_>(_fromVar_)

#define sAdjusted(_X1_, _Y1_, _X2_, _Y2_) adjusted(bool(sides&Render::Left)*_X1_, bool(sides&Render::Top)*_Y1_, bool(sides&Render::Right)*_X2_, bool(sides&Render::Bottom)*_Y2_)
#define sAdjust(_X1_, _Y1_, _X2_, _Y2_) adjust(bool(sides&Render::Left)*_X1_, bool(sides&Render::Top)*_Y1_, bool(sides&Render::Right)*_X2_, bool(sides&Render::Bottom)*_Y2_)

class StyleProject : public QCommonStyle
{
    Q_OBJECT
    Q_CLASSINFO ("X-KDE-CustomElements", "true")
public:
    StyleProject();
    ~StyleProject(){}

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
    void unpolish(QWidget *widget);

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *widget = 0) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = 0) const;
    QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const;

    bool eventFilter(QObject *, QEvent *);

    /* functions called for drawing */
    bool controlSkipper(const QStyleOption *option, QPainter *painter, const QWidget *widget) const { return true; }
    bool primitiveSkipper(const QStyleOption *option, QPainter *painter, const QWidget *widget) const { return true; }
    bool complexSkipper(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const { return true; }

    /* control elements */
    bool drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawCheckBox(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawCheckBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawRadioButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawRadioButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawToolBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawTab(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawViewItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

    /* complex controls */
    bool drawToolButton(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;
    bool drawScrollBar(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;
    bool drawSlider(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;
    bool drawComboBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

    /* primitive elements */
    bool drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawStatusBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawSplitter(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawWindow(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawTabBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawTabWidget(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawTabCloser(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

    /* events */
    bool paintEvent(QObject *o, QEvent *e);
    bool resizeEvent(QObject *o, QEvent *e);

    /* pointers to these functions */
    typedef bool (StyleProject::*StyleComplexControl)(const QStyleOptionComplex *, QPainter *, const QWidget *) const;
    typedef bool (StyleProject::*StyleControl)(const QStyleOption *, QPainter *, const QWidget *) const;
    typedef bool (StyleProject::*StylePrimitive)(const QStyleOption *, QPainter *, const QWidget *) const;
    typedef bool (StyleProject::*EventFilter)(QObject *o, QEvent *e);

private:
    StyleComplexControl m_cc[CCSize];
    StyleControl m_ce[CESize];
    StylePrimitive m_pe[PESize];
    EventFilter m_ev[EVSize];
    QColor m_specialColor[2];
};

#endif // STYLEPROJECT_H
