#ifndef DECOBUTTON_H
#define DECOBUTTON_H

#include "../stylelib/widgets.h" //buttonbase class
#include <KDecoration2/DecorationButton>

namespace KDecoration2 { class Decoration; }
namespace DSP
{
class Deco;
class Button : public KDecoration2::DecorationButton, public ButtonBase
{
    Q_OBJECT
public:
    explicit Button(QObject *parent, const QVariantList &args);
    static Button *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);
    void paint(QPainter *painter, const QRect &repaintArea);
    const QRect buttonRect() const {return geometry().toRect();}

protected:
    explicit Button(KDecoration2::DecorationButtonType type, Deco *decoration, QObject *parent = 0);

    void hoverEnterEvent(QHoverEvent *event);
    void hoverLeaveEvent(QHoverEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    //pure virtuals from buttonbase
    const QColor color(const ColorRole &c = Fg) const;
    const bool isDark() const;
    const bool isMaximized() const;
    const bool isActive() const;

    //reimplemented virtuals from buttonbase
    virtual const bool onAllDesktops() const;
    virtual const bool keepAbove() const;
    virtual const bool keepBelow() const;
    virtual const bool shade() const;
    void onClick(const Qt::MouseButton &button) {}
};
}

#endif //DECOBUTTON_H
