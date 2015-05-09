#ifndef DECOBUTTON_H
#define DECOBUTTON_H

#include "../stylelib/widgets.h" //buttonbase class

#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButton>

class DSPDeco;
class DSPButton : public ButtonBase, public KDecoration2::DecorationButton
{
//    Q_OBJECT
public:
    explicit DSPButton(QObject *parent, const QVariantList &args);
    static DSPButton *create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);
    void paint(QPainter *painter, const QRect &repaintArea);
    const QRect buttonRect() const {return geometry().toRect();}

protected:
    explicit DSPButton(KDecoration2::DecorationButtonType type, const QPointer<KDecoration2::Decoration> &decoration, QObject *parent = 0);

    //pure virtuals from buttonbase
    const QColor color(const ColorRole &c = Fg) const;
    const bool isDark() const;
    const bool isMaximized() const;
    const bool isActive() const;
    void onClick(const Qt::MouseButton &button) {}
};

#endif //DECOBUTTON_H
