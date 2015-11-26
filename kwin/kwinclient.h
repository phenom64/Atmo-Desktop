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

namespace DSP
{
class Factory;
class KwinClient;
class SizeGrip;

class DButton : public ButtonBase, public QSpacerItem
{
public:
    DButton(const ButtonBase::Type &t, KwinClient *client = 0);
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
    class Data
    {
    public:
        int noise;
        Gradient grad;
        QColor bg, fg;
        bool separator;
        void operator =(const Data &d)
        {
            noise = d.noise;
            grad = d.grad;
            bg = d.bg;
            fg = d.fg;
            separator = d.separator;
        }
        static void decoData(const QString &winClass, KwinClient *d);
        static QMap<QString, Data> s_data;
    };

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
    void updateBgPixmap();
    void updateData();

protected:
    bool eventFilter(QObject *, QEvent *);
    void paint(QPainter &p);
    void populate(const QString &buttons, int &size);
    void updateMask();
    void checkForDataFromWindowClass();
    QColor bgColor() const;
    QColor fgColor() const;
    const QRect positionRect(const KDecorationDefines::Position pos) const;
    const int titleHeight() const;

protected slots:
    void readCompositing();
    void memoryDestroyed(QObject *);

private:
    QHBoxLayout *m_titleLayout;
    QLinearGradient m_unoGradient;
    QColor m_bg, m_fg;
    Factory *m_factory;
    QPixmap m_bevelCorner[3], m_pix;
    int m_leftButtons, m_rightButtons, m_frameSize, m_prevLum, m_opacity;
    unsigned int m_noise;
    bool m_separator, m_contAware, m_uno, m_compositingActive, m_hor;
    friend class SizeGrip;
    SizeGrip *m_sizeGrip;
    friend class DButton;
    Buttons m_buttons;
    QSharedMemory *m_mem;
    Gradient m_gradient;
    WindowData *m_wd;
};

class AdaptorManager : public QObject
{
    Q_OBJECT
public:
    static AdaptorManager *instance();
    inline void addDeco(KwinClient *d) { m_decos << d; }
    inline void removeDeco(KwinClient *d) { m_decos.removeOne(d); }
    inline const bool hasDecos() const { return !m_decos.isEmpty(); }
    inline void updateData(uint win)
    {
        for (int i = 0; i < m_decos.count(); ++i)
        {
            KwinClient *d = m_decos.at(i);
            if (d->windowId() == win)
            {
                d->updateData();
                return;
            }
        }
    }
    inline void updateDeco(uint win)
    {
        for (int i = 0; i < m_decos.count(); ++i)
        {
            KwinClient *d = m_decos.at(i);
            if (d->windowId() == win)
            {
                d->update();
                return;
            }
        }
    }

protected:
    AdaptorManager();
    ~AdaptorManager();

private:
    static AdaptorManager *s_instance;
    QList<KwinClient *> m_decos;
};
} //DSP

#endif //KWINCLIENT_H
