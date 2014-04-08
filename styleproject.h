#ifndef STYLEPROJECT_H
#define STYLEPROJECT_H

#include <QPlastiqueStyle>

#define CCSize CC_MdiControls+1
#define CESize CE_ShapedFrame+1
#define PESize PE_PanelMenu+1

#define isSunken state & (State_Sunken | State_Selected | State_On)
#define isHovered state & State_MouseOver
#define isEnabled state & State_Enabled

class StyleProject : public QPlastiqueStyle
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
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

    void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;
    void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const;

    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;

    void polish(QWidget *widget);

    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const;

    /* functions called for drawing */
    bool controlSkipper(const QStyleOption *option, QPainter *painter, const QWidget *widget) const { return true; }
    bool primitiveSkipper(const QStyleOption *option, QPainter *painter, const QWidget *widget) const { return true; }
    bool complexSkipper(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const { return true; }

    /* controls */
    bool drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawCheckBox(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawCheckBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawRadioButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawRadioButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

    /* complex controls */
    bool drawToolButton(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

    /* primitives */
    bool drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    bool drawFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

    /* pointers to these functions */
    typedef bool (StyleProject::*StyleComplexControl)(const QStyleOptionComplex *, QPainter *, const QWidget *) const;
    typedef bool (StyleProject::*StyleControl)(const QStyleOption *, QPainter *, const QWidget *) const;
    typedef bool (StyleProject::*StylePrimitive)(const QStyleOption *, QPainter *, const QWidget *) const;

private:
    StyleComplexControl m_cc[CCSize];
    StyleControl m_ce[CESize];
    StylePrimitive m_pe[PESize];
};

#endif // STYLEPROJECT_H
