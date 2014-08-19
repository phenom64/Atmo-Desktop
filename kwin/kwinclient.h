#ifndef KWINCLIENT_H
#define KWINCLIENT_H

#include <QWidget>
#include <kdecoration.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "factory.h"

class KwinClient;
class Q_DECL_EXPORT TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(KwinClient *client, QWidget *parent = 0);
    void setBrush(const QBrush &brush);
    inline QBrush brush() const { return m_brush; }

protected:
    void paintEvent(QPaintEvent *);

private:
    QBrush m_brush;
    KwinClient *m_client;
};

class Factory;
class SizeGrip;
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

    inline TitleBar *titleBar() const { return m_titleBar; }
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
    TitleBar *m_titleBar;
    QLinearGradient m_unoGradient;
    QColor m_titleColor[2], m_textColor;
    Factory *m_factory;
    int m_headHeight, m_leftButtons, m_rightButtons;
    bool m_needSeparator;
    friend class TitleBar;
    SizeGrip *m_sizeGrip;
};

#endif //KWINCLIENT_H
