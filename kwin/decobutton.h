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

    void hoverChanged();

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

class EmbeddedWidget : public QWidget
{
    Q_OBJECT
    friend class EmbeddedButton;
public:
    explicit EmbeddedWidget(Deco *d);

protected:
    void wheelEvent(QWheelEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
    void restack();
    void updatePosition();

private:
    Deco *m_deco;
};

class EmbeddedButton : public QWidget, public ButtonBase
{
    Q_OBJECT
public:
    EmbeddedButton(QWidget *parent, const Type &t);
    const QRect buttonRect() const { return rect(); }

protected:
    inline EmbeddedWidget *embeddedWidget() const { return static_cast<EmbeddedWidget *>(const_cast<QWidget *>(parentWidget())); }
    Deco *decoration() const;
    void hoverChanged();
    void paintEvent(QPaintEvent *e);
    void enterEvent(QEvent *e) { hover(); QWidget::enterEvent(e); }
    void leaveEvent(QEvent *e) { unhover(); QWidget::enterEvent(e); }
    void mousePressEvent(QMouseEvent *e) { processMouseEvent(e); /*QWidget::mousePressEvent(e);*/ }
    void mouseReleaseEvent(QMouseEvent *e) { processMouseEvent(e); /*QWidget::mouseReleaseEvent(e);*/ }

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
    void onClick(const Qt::MouseButton &button);
};

}

#endif //DECOBUTTON_H
