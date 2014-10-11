#ifndef KWINCLIENT_H
#define KWINCLIENT_H

#include <QWidget>
#include <kdecoration.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "sizegrip.h"
#include "factory.h"
#include "../stylelib/widgets.h"

class Factory;
class KwinClient;
class SizeGrip;
class DButton : public Button
{
public:
    DButton(const Type &t, KwinClient *client = 0, QWidget *parent = 0);
    ~DButton(){}

protected:
    bool isActive();
    void onClick(QMouseEvent *e, const Type &t);
    bool paintOnAllDesktopsButton(QPainter &p);
    bool paintWindowMenuButton(QPainter &p);
    bool paintKeepAboveButton(QPainter &p);
    bool paintKeepBelowButton(QPainter &p);
    bool paintApplicationMenuButton(QPainter &p);
    bool paintShadeButton(QPainter &p);

private:
    KwinClient *m_client;
};

class KwinClient : public KDecoration
{
    Q_OBJECT
public:
    KwinClient(KDecorationBridge *bridge, Factory *factory);
    ~KwinClient();

    // functions not implemented in KDecoration
    void init();
    void activeChange();
    void borders(int &left, int &right, int &top, int &bottom) const;
    void captionChange();
    void desktopChange() {}
    void iconChange() {}
    void maximizeChange() {}
    QSize minimumSize() const;
    KDecorationDefines::Position mousePosition(const QPoint &p) const { return KDecorationDefines::PositionCenter; }
    void resize(const QSize &s);
    void shadeChange() {}
    void reset(unsigned long changed);

    bool compositingActive() const;
    int buttonCornerWidth(bool left) { return left ? m_leftButtons : m_rightButtons; }

protected:
    bool eventFilter(QObject *, QEvent *);
    void paint(QPainter &p);
    void populate(const QString &buttons, bool left);

protected slots:
    void postInit();

private:
    QHBoxLayout *m_titleLayout;
    QLinearGradient m_unoGradient;
    QColor m_titleColor[2], m_textColor;
    Factory *m_factory;
    QBrush m_brush;
    QPixmap m_pix, m_bgPix[2];
    float m_opacity;
    int m_headHeight, m_leftButtons, m_rightButtons;
    bool m_needSeparator;
    friend class SizeGrip;
    SizeGrip *m_sizeGrip;
    friend class DButton;
};

#endif //KWINCLIENT_H
