#ifndef KWINCLIENT_H
#define KWINCLIENT_H

#include <QWidget>
#include <kdecoration.h>

class KwinClient : public KDecoration
{
    Q_OBJECT
public:
    KwinClient(KDecorationBridge *bridge = 0, KDecorationFactory *factory = 0);

    // functions not implemented in KDecoration
    void init();
    void activeChange() {}
    void borders(int &left, int &right, int &top, int &bottom) const {}
    void captionChange() {}
    void desktopChange() {}
    void iconChange() {}
    void maximizeChange() {}
    QSize minimumSize() const { return QSize(); }
    KDecorationDefines::Position mousePosition(const QPoint &p) const { return KDecorationDefines::PositionCenter; }
    void resize(const QSize &s) {}
    void shadeChange() {}
};

#endif
