#ifndef KWINCLIENT2_H
#define KWINCLIENT2_H

#include "decobutton.h"

#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationButton>
#include <KDecoration2/DecorationButtonGroup>

#include <KSharedConfig>
#include <KCModule>

#include <QVariant>

class DSPConfigModule: public KCModule
{
    Q_OBJECT
public:
    DSPConfigModule(QWidget *parent = 0, const QVariantList &args = QVariantList());
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

class DSPDeco : public KDecoration2::Decoration
{
    Q_OBJECT
public:
    explicit DSPDeco(QObject *parent = 0, const QVariantList &args = QVariantList());
    ~DSPDeco(){}
    void paint(QPainter *painter, const QRect &repaintArea);
    bool event(QEvent *event);

public slots:
    /**
     * This method gets invoked from the framework once the Decoration is created and
     * completely setup.
     *
     * An inheriting class should override this method and perform all initialization in
     * this method instead of the constructor.
     **/
    void init();

private:
    KDecoration2::DecorationButtonGroup *m_leftButtons, *m_rightButtons;
};

#endif //KWINCLIENT2_H
