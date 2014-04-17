#ifndef KWINCLIENT_H
#define KWINCLIENT_H

#include <QWidget>
#include <kdecoration.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "factory.h"

class KwinClient;
class TitleBar : public QWidget
{
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
class KwinClient : public KDecoration
{
    Q_OBJECT
public:
    KwinClient(KDecorationBridge *bridge, Factory *factory);

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

protected:
    bool eventFilter(QObject *, QEvent *);
    void paint(QPainter &p);
    void populate(const QString &buttons);

protected slots:
    void postInit();

private:
    QHBoxLayout *m_titleLayout;
    TitleBar *m_titleBar;
    QLinearGradient m_unoGradient;
    QColor m_unoColor;
    Factory *m_factory;
    bool m_isUno;
};

#endif //KWINCLIENT_H
