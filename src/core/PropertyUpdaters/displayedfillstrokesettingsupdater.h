#ifndef DISPLAYEDFILLSTROKESETTINGSUPDATER_H
#define DISPLAYEDFILLSTROKESETTINGSUPDATER_H
#include "PropertyUpdaters/propertyupdater.h"
class BoundingBox;

class DisplayedFillStrokeSettingsUpdater : public PropertyUpdater {
public:
    DisplayedFillStrokeSettingsUpdater(BoundingBox *path);

    void update();
    void frameChangeUpdate();
    void finishedChange();
private:
    BoundingBox *mTarget;
};

#endif // DISPLAYEDFILLSTROKESETTINGSUPDATER_H