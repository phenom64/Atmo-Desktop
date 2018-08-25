#ifndef MENUBAR_H
#define MENUBAR_H

#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationButton>

class QAction;
class QTimer;
class DBusMenuImporter;
class QMenu;
namespace DSP
{
class Deco;
class MenuBarItem;
class MenuBar : public KDecoration2::DecorationButtonGroup
{
    Q_OBJECT
public:
    MenuBar(Deco *deco, const QString &service, const QString &path);
    ~MenuBar();
    bool hasShownMenues() const;
    void hideAllMenues();
    void startMousePolling();
    void stopMousePolling();
    QAction *actionFromText(const QString &text) const;

    Deco *deco;
    DBusMenuImporter *importer;
    QTimer *timer;
    MenuBarItem *prevHovered;
    QStringList tbActions;
    int active;

public Q_SLOTS:
    void updateMenu();

Q_SIGNALS:
    void activeChanged();

protected Q_SLOTS:
    void pollMouse();
    void menuUpdated();
};

class MenuBarItem : public KDecoration2::DecorationButton
{
    Q_OBJECT
    friend class MenuBar;
public:
    MenuBarItem(MenuBar *menu, int index);
    ~MenuBarItem();
    void paint(QPainter *painter, const QRect &repaintArea);
    QString text() const;
    virtual QAction *action() const;
    QMenu *menu() const;
    MenuBar *m;
    QRect topLevelGeo() const;
    bool hasGeo;
    int idx;
    void hoverEnter();
protected:
    void hoverEnterEvent(QHoverEvent *event);
protected slots:
    virtual void click();
};

} //namespace DSP

#endif //MENUBAR_H
