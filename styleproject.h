#ifndef STYLEPROJECT_H
#define STYLEPROJECT_H

#include <QPlastiqueStyle>

class StyleProject : public QPlastiqueStyle
{
    Q_OBJECT
    Q_CLASSINFO ("X-KDE-CustomElements", "true")
public:
    StyleProject(){}
    ~StyleProject(){}
};

#endif // STYLEPROJECT_H
