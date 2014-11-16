#ifndef KWINCLIENT_H
#define KWINCLIENT_H

#include <QWidget>
#include <kdecoration.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSharedMemory>

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
    bool isActive() const;
    void onClick(QMouseEvent *e, const Type &t);

    bool paintMaxButton(QPainter &p);
    bool paintOnAllDesktopsButton(QPainter &p);
    bool paintWindowMenuButton(QPainter &p);
    bool paintKeepAboveButton(QPainter &p);
    bool paintKeepBelowButton(QPainter &p);
    bool paintApplicationMenuButton(QPainter &p);
    bool paintShadeButton(QPainter &p);
    bool paintQuickHelpButton(QPainter &p);


    const QColor color(const ColorRole &c) const;
    const bool isDark() const;
private:
    KwinClient *m_client;
};

class KwinClient : public KDecoration
{
    Q_OBJECT
public:
    enum CustomColors { Text = 0, Bg, CustColCount };
    typedef QList<Button *> Buttons;
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

    void updateContBg();

    bool compositingActive() const;
    int buttonCornerWidth(bool left) { return left ? m_leftButtons : m_rightButtons; }

protected:
    bool eventFilter(QObject *, QEvent *);
    void paint(QPainter &p);
    void populate(const QString &buttons, bool left);
    QColor bgColor() const;
    QColor fgColor() const;

private:
    QHBoxLayout *m_titleLayout;
    QLinearGradient m_unoGradient;
    QColor m_custcol[CustColCount];
    Factory *m_factory;
    QPixmap m_bgPix[2];
    float m_opacity;
    int m_headHeight, m_leftButtons, m_rightButtons;
    bool m_needSeparator, m_contAware;
    friend class SizeGrip;
    SizeGrip *m_sizeGrip;
    friend class DButton;
    Buttons m_buttons;
    QSharedMemory *m_mem;
};

#endif //KWINCLIENT_H
