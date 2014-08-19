#ifndef STYLECONFIG_H
#define STYLECONFIG_H

#include <QDialog>
#include "ui_config.h"

class StyleConfig : public QDialog
{
    Q_OBJECT
public:
    explicit StyleConfig(QWidget *parent = 0);
private:
    Ui::Dialog ui;
};

#endif //STYLECONFIG_H
