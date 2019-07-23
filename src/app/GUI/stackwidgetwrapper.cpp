#include "stackwidgetwrapper.h"
#include "GUI/widgetstack.h"
#include "stacklayouts.h"

StackWidgetWrapper::StackWidgetWrapper(WidgetStackLayoutItem* const layoutItem,
                                       const LayoutItemCreator &layoutItemCreator,
                                       const Creator &creator,
                                       const SetupOp &setup,
                                       QWidget* const parent) :
    QWidget(parent), mLayoutItem(layoutItem),
    mLayoutItemCreator(layoutItemCreator),
    mCreator(creator), mSetupOp(setup) {
    mLayoutItem->setCurrent(this);
    mCornerMenu = new StackWrapperCornerMenu(this);
    mCornerMenu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    mLayout = new QVBoxLayout(this);
    setLayout(mLayout);
    mLayout->setSpacing(0);
    mLayout->setMargin(0);
    mSetupOp(this);
}

void StackWidgetWrapper::setMenuBar(StackWrapperMenu * const menu) {
    if(mMenuBar) delete mMenuBar;
    mMenuBar = menu;
    mMenuBar->setCornerWidget(mCornerMenu);
    mLayout->insertWidget(0, mMenuBar);
}

void StackWidgetWrapper::setCentralWidget(QWidget * const widget) {
    if(mCenterWidget) delete mCenterWidget;
    mCenterWidget = widget;
    mLayout->insertWidget(-1, mCenterWidget);
    mCenterWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
                                 QSizePolicy::MinimumExpanding);
}

StackWidgetWrapper* StackWidgetWrapper::splitH() {
    auto otherLayoutItem = mLayoutItemCreator();
    const auto otherLayoutItemPtr = otherLayoutItem.get();
    const auto lItem = mLayoutItem->splitH(std::move(otherLayoutItem));
    return split<HWidgetStack>(lItem, otherLayoutItemPtr);
}

StackWidgetWrapper* StackWidgetWrapper::splitV() {
    auto otherLayoutItem = mLayoutItemCreator();
    const auto otherLayoutItemPtr = otherLayoutItem.get();
    const auto lItem = mLayoutItem->splitV(std::move(otherLayoutItem));
    return split<VWidgetStack>(lItem, otherLayoutItemPtr);
}

void StackWidgetWrapper::closeWrapper() {
    mLayoutItem->close();
    const auto vStack = dynamic_cast<VWidgetStack*>(parentWidget());
    if(vStack) {
        vStack->takeWidget(this);
        if(vStack->count() == 1) {
            const auto wid = vStack->takeWidget(0);
            bool central = false;
            gReplaceWidget(vStack, wid, &central);
            vStack->deleteLater();
            if(central) {
                const auto sww = dynamic_cast<StackWidgetWrapper*>(wid);
                if(sww) sww->disableClose();
            }
        }
    } else {
        const auto hStack = dynamic_cast<HWidgetStack*>(parentWidget());
        if(hStack) {
            hStack->takeWidget(this);
            if(hStack->count() == 1) {
                const auto wid = hStack->takeWidget(0);
                bool central = false;
                gReplaceWidget(hStack, wid, &central);
                hStack->deleteLater();
                if(central) {
                    const auto sww = dynamic_cast<StackWidgetWrapper*>(wid);
                    if(sww) sww->disableClose();
                }
            }
        }
    }

    deleteLater();
}

void StackWidgetWrapper::disableClose() {
    mCornerMenu->disableClose();
}

StackWrapperMenu::StackWrapperMenu() {
    //setStyleSheet("QMenuBar { border-bottom: 1px solid black; }");
    //setStyleSheet("QMenuBar#cornerMenuBar::item { padding: 2px 4px; }");

    setStyleSheet("QMenuBar#cornerMenuBar::item { padding-top: 1px; margin-bottom: 2px; }");

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

StackWrapperCornerMenu::StackWrapperCornerMenu(StackWidgetWrapper * const target) {
    mSplitV = addAction("split v");
    mSplitV->setIcon(QIcon(":/icons/split_v.png"));
    mSplitH = addAction("split h");
    mSplitH->setIcon(QIcon(":/icons/split_h.png"));
    mClose = addAction("x");
    mClose->setIcon(QIcon(":/icons/close.png"));

    connect(mSplitV, &QAction::triggered,
            target, &StackWidgetWrapper::splitV);
    connect(mSplitH, &QAction::triggered,
            target, &StackWidgetWrapper::splitH);
    connect(mClose, &QAction::triggered,
            target, &StackWidgetWrapper::closeWrapper);

    setObjectName("cornerMenuBar");

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}