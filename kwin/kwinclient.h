#ifndef KWINCLIENT_H
#define KWINCLIENT_H

#include <QWidget>
#include <kdecoration.h>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "factory.h"

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

protected:
    bool eventFilter(QObject *, QEvent *);
    void paint(QPainter &p);
    void populate(const QString &buttons);

protected slots:
    void postInit();

private:
    QHBoxLayout *m_titleLayout;
    QWidget *m_stretch;
    QLinearGradient m_unoGradient;
    QColor m_unoColor;
    bool m_isUno;
};

#endif //KWINCLIENT_H
