#ifndef BOXSCROLLWIDGET_H
#define BOXSCROLLWIDGET_H

#include "OptimalScrollArea/scrollwidget.h"
#include "GUI/keyfocustarget.h"
class BoxScrollWidgetVisiblePart;
class ScrollArea;
class WindowSingleWidgetTarget;
class Document;

class BoxScrollWidget : public ScrollWidget {
    Q_OBJECT
public:
    explicit BoxScrollWidget(Document& document,
                             ScrollArea * const parent = nullptr);
    ~BoxScrollWidget();
    BoxScrollWidgetVisiblePart *getVisiblePartWidget();
    //    void updateAbstraction();
protected:
    stdptr<SWT_Abstraction> mCoreAbs;
    void createVisiblePartWidget();
};

#endif // BOXSCROLLWIDGET_H