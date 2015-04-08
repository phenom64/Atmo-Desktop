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
#include "../config/settings.h"

class Factory;
class KwinClient;
class SizeGrip;

class DButton : public ButtonBase, public QSpacerItem
{
public:
    DButton(const Type &t, KwinClient *client = 0);
    ~DButton(){}

    QSize sizeHint() const { return QSize(16, 16); }
    const QRect buttonRect() const { return geometry(); }

protected:
    void onClick(const Qt::MouseButton &button);
    void hoverChanged();

    const bool isActive() const;
    const bool isMaximized() const;
    const bool onAllDesktops() const;
    const bool keepAbove() const;
    const bool keepBelow() const;
    const bool shade() const;

    const ButtonStyle buttonStyle() const;
    const QColor color(const ColorRole &c) const;
    const bool isDark() const;

private:
    KwinClient *m_client;
};

class WindowData;
class KwinClient : public KDecoration
{
    Q_OBJECT
public:
    enum CustomColors { Text = 0, Bg, CustColCount };
    typedef QList<DButton *> Buttons;
    KwinClient(KDecorationBridge *bridge, Factory *factory);
    ~KwinClient();

    // functions not implemented in KDecoration
    void init();
    void activeChange();
    void borders(int &left, int &right, int &top, int &bottom) const;
    void captionChange();
    void desktopChange() {}
    void iconChange() {}
    void maximizeChange();
    QSize minimumSize() const;
    KDecorationDefines::Position mousePosition(const QPoint &point) const;
    void resize(const QSize &s);
    void shadeChange() {}
    void reset(unsigned long changed);
    void update();
    void updateButtons();
    void updateBgPixmaps();
    void updateDataFromX();
    void setBgPix(const unsigned long pix, const QSize &sz);
    void setWindowData(WindowData wd);

protected:
    bool eventFilter(QObject *, QEvent *);
    void paint(QPainter &p);
    void populate(const QString &buttons, int &size);
    void updateMask();
    void checkForDataFromWindowClass();
    QColor bgColor(const bool *active = 0) const;
    QColor fgColor(const bool *active = 0) const;
    const QRect positionRect(const KDecorationDefines::Position pos) const;
    const int titleHeight() const;

protected slots:
    void readCompositing();

private:
    QHBoxLayout *m_titleLayout;
    QLinearGradient m_unoGradient;
    QColor m_custcol[CustColCount];
    Factory *m_factory;
    QPixmap m_bgPix[2], m_bevelCorner[3], m_pix;
    int m_headHeight, m_leftButtons, m_rightButtons, m_buttonStyle, m_frameSize, m_prevLum, m_opacity;
    unsigned int m_noise;
    bool m_needSeparator, m_contAware, m_uno, m_compositingActive, m_hor;
    friend class SizeGrip;
    SizeGrip *m_sizeGrip;
    friend class DButton;
    Buttons m_buttons;
    QSharedMemory *m_mem;
    Gradient m_gradient;
};

#endif //KWINCLIENT_H
