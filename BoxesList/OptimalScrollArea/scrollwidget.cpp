#include "scrollwidget.h"
#include "scrollwidgetvisiblepart.h"
#include "singlewidget.h"
#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "scrollarea.h"
#include "global.h"

ScrollWidget::ScrollWidget(ScrollArea *parent) :
    MinimalScrollWidget(parent) {
    //createVisiblePartWidget();
}

void ScrollWidget::updateAbstraction() {
    if(mMainAbstraction != NULL) {
        mMainAbstraction->setIsMainTarget(false);
//        if(mMainAbstraction->isDeletable()) {
//            delete mMainAbstraction;
//        }
    }
    if(mMainTarget == NULL) {
        mMainAbstraction = NULL;
    } else {
        mMainAbstraction = mMainTarget->SWT_getAbstractionForWidget(
                                                mVisiblePartWidget);
        mMainAbstraction->setIsMainTarget(true);
    }
    mVisiblePartWidget->setMainAbstraction(mMainAbstraction);
    updateHeight();
}

void ScrollWidget::createVisiblePartWidget() {
    mVisiblePartWidget = new ScrollWidgetVisiblePart(this);
    mMinimalVisiblePartWidget = mVisiblePartWidget;
}

void ScrollWidget::setMainTarget(SingleWidgetTarget *target) {
    mMainTarget = target;
    updateAbstraction();
}

void ScrollWidget::updateHeight() {
    if(mMainAbstraction == NULL) {
        hide();
        return;
    }
    setFixedHeight(mMainAbstraction->getHeight(
                       mVisiblePartWidget->getCurrentRulesCollection(),
                       false,
                       false) + MIN_WIDGET_HEIGHT/2);
    if(isHidden()) show();
}
