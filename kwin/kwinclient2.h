#ifndef KWINCLIENT2_H
#define KWINCLIENT2_H

#include <QDBusAbstractAdaptor>
#include <KCModule>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <QVariant>
#include <QDebug>
#include "../config/settings.h"
#include "../stylelib/windowdata.h"
namespace KDecoration2 { class DecorationButtonGroup; }
class WindowData;
class QPixmap;
class QSharedMemory;
namespace DSP
{
class Grip;
class ConfigModule : public KCModule
{
    Q_OBJECT
public:
    ConfigModule(QWidget *parent = 0, const QVariantList &args = QVariantList());
public slots:
    /**
     * Load the configuration data into the module.
     *
     * The load method sets the user interface elements of the
     * module to reflect the current settings stored in the
     * configuration files.
     *
     * This method is invoked whenever the module should read its configuration
     * (most of the times from a config file) and update the user interface.
     * This happens when the user clicks the "Reset" button in the control
     * center, to undo all of his changes and restore the currently valid
     * settings. It is also called right after construction.
     */
    void load() {}

    /**
     * Save the configuration data.
     *
     * The save method stores the config information as shown
     * in the user interface in the config files.
     *
     * If necessary, this method also updates the running system,
     * e.g. by restarting applications. This normally does not apply for
     * KSettings::Dialog modules where the updating is taken care of by
     * KSettings::Dispatcher.
     *
     * save is called when the user clicks "Apply" or "Ok".
     *
     * If you use KConfigXT, saving is taken care off automatically and
     * you do not need to load manually. However, if you for some reason reimplement it and
     * also are using KConfigXT, you must call this function, otherwise the saving of KConfigXT
     * options will not work. Call it at the very end of your reimplementation, to avoid
     * changed() signals getting emitted when you modify widgets.
     */
    void save() {}

    /**
     * Sets the configuration to sensible default values.
     *
     * This method is called when the user clicks the "Default"
     * button. It should set the display to useful values.
     *
     * If you use KConfigXT, you do not have to reimplement this function since
     * the fetching and settings of default values is done automatically. However, if you
     * reimplement and also are using KConfigXT, remember to call the base function at the
     * very end of your reimplementation.
     */
    void defaults() {}
};


class Deco : public KDecoration2::Decoration
{
    friend class Button;
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
        static void decoData(const QString &winClass, Deco *d);
        static QMap<QString, Data> s_data;
    };
    explicit Deco(QObject *parent = 0, const QVariantList &args = QVariantList());
    ~Deco();
    void paint(QPainter *painter, const QRect &repaintArea);

public slots:
    /**
     * This method gets invoked from the framework once the Decoration is created and
     * completely setup.
     *
     * An inheriting class should override this method and perform all initialization in
     * this method instead of the constructor.
     **/
    void init();

    void updateData();

protected slots:
    void widthChanged(const int width);
    void activeChanged(const bool active);
    void captionChanged(const QString &caption) { update(); }

protected:
    void checkForDataFromWindowClass();
    void updateBgPixmap();
    void initMemory();
    void paintBevel(QPainter *painter, const int bgLum);

    const QColor bgColor() const;
    const QColor fgColor() const;

private:
    KDecoration2::DecorationButtonGroup *m_leftButtons, *m_rightButtons;
    QPixmap m_pix, m_bevelCorner[3];
    QSharedMemory *m_mem;
    QColor m_bg, m_fg;
    Gradient m_gradient;
    WindowData *m_wd;
    int m_prevLum, m_noise;
    bool m_separator;
};

class AdaptorManager : public QObject
{
    Q_OBJECT
public:
    static AdaptorManager *instance();
    inline void addDeco(DSP::Deco *d) { m_decos << d; }
    inline void removeDeco(DSP::Deco *d) { m_decos.removeOne(d); }
    inline void updateData(uint win)
    {
        for (int i = 0; i < m_decos.count(); ++i)
        {
            DSP::Deco *d = m_decos.at(i);
            if (d->client().data()->windowId() == win)
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
            DSP::Deco *d = m_decos.at(i);
            if (d->client().data()->windowId() == win)
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
    QList<DSP::Deco *> m_decos;
};

class Grip : public QWidget
{
public:
    Grip(Deco *d);
    void updatePosition();

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);

private:
    Deco *m_deco;
};

} //DSP
#endif //KWINCLIENT2_H
