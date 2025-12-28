/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**************************************************************************
*   Based on styleproject, Copyright (C) 2013 by Robert Metsaranta        *
*   therealestrob@gmail.com                                              *
***************************************************************************/
#include "toolbarhelpers.h"
#include "ops.h"
#include <QToolBar>
#include <QAction>
#include <QLayout>
#include <QMainWindow>
#include <QPointer>
#include <QSet>
#include "windowdata.h"
#include "xhandler.h"
#include <QWidgetAction>
#include <QTimer>

using namespace NSE;

ToolbarHelpers *ToolbarHelpers::s_instance(0);

static const char *s_spacerName("NSE_TOOLBARSPACER");

ToolbarHelpers
*ToolbarHelpers::instance()
{
    if (!s_instance)
        s_instance = new ToolbarHelpers();
    return s_instance;
}

ToolbarHelpers::ToolbarHelpers(QObject *parent)
    : QObject(parent)
{

}

static QList<QPair<QPointer<QToolBar>, int> > s_spacerQueue;

void
ToolbarHelpers::fixSpacerLater(QToolBar *toolbar, int width)
{
    QPointer<QToolBar> guard(toolbar);
    if (guard.isNull())
        return;
    const QPair<QPointer<QToolBar>, int> entry(guard, width);
    if (!s_spacerQueue.contains(entry))
    {
        s_spacerQueue << entry;
        /* delay spacer fix with a weak pointer guard so deleted toolbars are skipped safely */
        QTimer::singleShot(0, ToolbarHelpers::instance(), [guard, width]()
        {
            const QPair<QPointer<QToolBar>, int> current(guard, width);
            if (guard.isNull())
            {
                s_spacerQueue.removeOne(current);
                return;
            }
            ToolbarHelpers::instance()->fixSpacer(guard.data(), width);
        });
    }
}

void
ToolbarHelpers::fixSpacer(QToolBar *tb, int width)
{
    s_spacerQueue.removeOne(QPair<QPointer<QToolBar>, int>(tb, width));
    if (!tb
            || !qobject_cast<QMainWindow *>(tb->parentWidget())
            || tb->findChild<QTabBar *>()
            || !tb->styleSheet().isEmpty())
        return;
    /* avoid dereferencing first() on an empty action list during toolbar startup */
    if (tb->actions().isEmpty())
        return;

    if (tb->isMovable() && width == 7)
    {
        if (QAction *spacer = tb->findChild<QAction *>(s_spacerName))
            spacer->deleteLater();
        return;
    }
    QAction *spacer = tb->findChild<QAction *>(s_spacerName);
    if (!spacer)
    {
        QWidget *w = new QWidget(tb);
        w->setObjectName("NSE_spacer");
        spacer = tb->insertWidget(tb->actions().first(), w);
        spacer->setObjectName(s_spacerName);
    }
    tb->widgetForAction(spacer)->setFixedSize(width, 7);
    tb->removeAction(spacer);
    tb->insertAction(tb->actions().first(), spacer);
    spacer->setVisible(!tb->isMovable()||width>7);
}

static QList<QPointer<QToolBar>> s_marginsQueue;
static QSet<QToolBar *> s_adjustingMargins;  // Re-entrancy guard

void
ToolbarHelpers::adjustMarginsLater(QToolBar *toolBar)
{
    QPointer<QToolBar> guard(toolBar);
    if (guard.isNull())
        return;
    /* check if already queued */
    for (const auto &queued : s_marginsQueue)
        if (queued.data() == toolBar)
            return;
    s_marginsQueue << guard;
    QTimer::singleShot(0, ToolbarHelpers::instance(), [guard]()
    {
        s_marginsQueue.removeOne(guard);
        if (guard.isNull())
            return;
        ToolbarHelpers::instance()->adjustMarginsImpl(guard.data());
    });
}
void
ToolbarHelpers::adjustMarginsImpl(QToolBar *tb)
{
    if (!tb)
        return;

    // Re-entrancy guard: prevent recursive calls during layout updates
    if (s_adjustingMargins.contains(tb))
        return;
    s_adjustingMargins.insert(tb);

    // Use a scope guard pattern to ensure removal on all exit paths
    struct Guard {
        QToolBar *t;
        ~Guard() { s_adjustingMargins.remove(t); }
    } guard{tb};

    if (tb->isFloating())
    {
        // Block signals during margin changes to prevent layout re-entrancy
        const bool wasBlocked = tb->signalsBlocked();
        tb->blockSignals(true);
        tb->setMovable(true);
        tb->setContentsMargins(0, 0, 0, 0);
        if (tb->layout())
            tb->layout()->setContentsMargins(0, 0, 0, 0);
        tb->blockSignals(wasBlocked);
        return;
    }

    QMainWindow *win = qobject_cast<QMainWindow *>(tb->parentWidget());
//    if (!win || !toolBar->layout() || toolBar->actions().isEmpty() || win->toolBarArea(toolBar) != Qt::TopToolBarArea)
//        return;

    if (win
            && tb->geometry().top() <= win->rect().top()
            && win->toolBarArea(tb) == Qt::TopToolBarArea
            && !win->parentWidget())
    {
        if (tb->layout())
            tb->layout()->setContentsMargins(0, 0, 0, 0);
        WindowData d = WindowData::memory(XHandler::windowId(win), win);
        int m(0);
        if (d && d.lock())
        {
            m = d->rightEmbedSize;
            d.unlock();
        }
        if (tb->style())
        {
            // Block signals during setContentsMargins to prevent layout re-entrancy
            const bool wasBlocked = tb->signalsBlocked();
            tb->blockSignals(true);
            tb->QWidget::setContentsMargins(0, 0, tb->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent)+m, 6);
            tb->blockSignals(wasBlocked);
        }
    }
    else if (tb->findChild<QTabBar *>()) //sick, put a tabbar in a toolbar... eiskaltdcpp does this :)
    {
        if (tb->layout())
            tb->layout()->setContentsMargins(0, 0, 0, 0);
        tb->setMovable(false);
    }
    else
    {
        QEvent e(QEvent::StyleChange);
        QApplication::sendEvent(tb, &e);
    }
}
