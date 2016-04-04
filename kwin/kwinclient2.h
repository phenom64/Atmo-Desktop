#ifndef KWINCLIENT2_H
#define KWINCLIENT2_H

#include <QDBusAbstractAdaptor>
#include <KCModule>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <QVariant>
#include <QDebug>
#include <netwm_def.h>
#include "../config/settings.h"
#include "../stylelib/windowdata.h"
namespace KDecoration2 { class DecorationButtonGroup; }
class WindowData;
class QPixmap;
class QSharedMemory;
namespace DSP
{
class ConfigModule : public KCModule
{
    Q_OBJECT
public:
    ConfigModule(QWidget *parent = 0, const QVariantList &args = QVariantList());
    ~ConfigModule() {}
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
class Grip;
class EmbedHandler;
class Deco : public KDecoration2::Decoration
{
    friend class Button;
    friend class Grip;
    friend class EmbeddedWidget;
    friend class EmbeddedButton;
    Q_OBJECT
public:
    class Data
    {
    public:
        int noise, btnStyle;
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
            btnStyle = d.btnStyle;
        }
        static void addDataForWinClass(const QString &winClass, QSettings &s);
        static void readWindowData();
        static void decoData(const QString &winClass, Deco *d);
        static QMap<QString, Data> s_data;
    };
    explicit Deco(QObject *parent = 0, const QVariantList &args = QVariantList());
    ~Deco();
    void paint(QPainter *painter, const QRect &repaintArea);
    bool event(QEvent *event);
    const int titleHeight() const;
    void setTitleHeight(const int h);
    const int border();

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
    void dataDestroyed() { if (sender() == m_wd) m_wd = 0; }
    void maximizedChanged(const bool max);

protected:
    void hoverEnter();
    void hoverLeave();

    void checkForDataFromWindowClass();
    void updateBgPixmap();
    void initMemory(WindowData *data);
    void paintBevel(QPainter *painter, const int bgLum);

    const QColor bgColor() const;
    const QColor fgColor() const;

    void wheelEvent(QWheelEvent *event);

    void removeEmbedder();

private:
    KDecoration2::DecorationButtonGroup *m_leftButtons, *m_rightButtons;
    EmbedHandler *m_embedder;
    QPixmap m_pix/*, m_bevelCorner[3]*/;
    QSharedMemory *m_mem;
    QColor m_bg, m_fg;
    Gradient m_gradient, m_winGradient;
    WindowData *m_wd; 
    Grip *m_grip;
    int m_prevLum, m_noise, m_buttonStyle, m_tries;
    bool m_separator, m_isHovered;
};

class DecoAdaptor;
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
    void windowChanged(uint win, bool active);
    void dataChanged(uint win);

protected:
    AdaptorManager();
    ~AdaptorManager();

private:
    static AdaptorManager *s_instance;
    QList<DSP::Deco *> m_decos;
    DecoAdaptor *m_adaptor;
};

class Grip : public QWidget
{
    friend class Deco;
    Q_OBJECT
public:
    Grip(Deco *d);
    enum Data { Margin = -1, Size = 13 };

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void restack();
    void setColor(const QColor &c);
    void regenPix();
    static QPolygon shape();

protected slots:
    void updatePosition();

private:
    Deco *m_deco;
    QPixmap m_pix;
};

} //DSP
#endif //KWINCLIENT2_H
