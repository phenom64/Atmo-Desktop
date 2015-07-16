#ifndef STYLECONFIG_H
#define STYLECONFIG_H

#include "ui_config.h"
#include <QWidget>

class StyleConfig : public QWidget
{
    Q_OBJECT
public:
    explicit StyleConfig(QWidget *parent = 0);

public slots:
    //slots that the systemsettings tries and connect to
    void save();
    void defaults();

signals:
    //signals used by the systemsettings
    void changed(bool changed);

protected:
    void readSettings();

private:
    Ui::Config ui;
};

#endif //STYLECONFIG_H
