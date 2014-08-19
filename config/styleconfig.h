#ifndef STYLECONFIG_H
#define STYLECONFIG_H

#include "ui_config.h"

class StyleConfig : public QWidget
{
    Q_OBJECT
public:
    explicit StyleConfig(QWidget *parent = 0);
private:
    Ui::Config ui;
};

#endif //STYLECONFIG_H
