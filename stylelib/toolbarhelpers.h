#ifndef TOOLBARHELPERS_H
#define TOOLBARHELPERS_H

#include <QObject>

class QToolBar;
namespace DSP
{
class Q_DECL_EXPORT ToolbarHelpers : public QObject
{
    Q_OBJECT
public:
    static ToolbarHelpers *instance();
    static void fixSpacerLater(QToolBar *toolbar, int width = 7);
    static void adjustMarginsLater(QToolBar *toolBar);

protected:
    explicit ToolbarHelpers(QObject *parent = 0);

protected slots:
    void fixSpacer(qulonglong toolbar, int width = 7);
    void adjustMargins(qulonglong toolbar);

private:
    static ToolbarHelpers *s_instance;
};
}

#endif // TOOLBARHELPERS_H
