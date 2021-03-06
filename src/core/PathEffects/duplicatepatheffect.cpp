// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "duplicatepatheffect.h"
#include "Animators/qpointfanimator.h"
#include "Animators/intanimator.h"

DuplicatePathEffect::DuplicatePathEffect() :
    PathEffect("duplicate effect", PathEffectType::DUPLICATE) {
    mTranslation = enve::make_shared<QPointFAnimator>("translation");
    mTranslation->setBaseValue(QPointF(10, 10));
    ca_addChild(mTranslation);

    mCount = enve::make_shared<IntAnimator>(1, 0, 25, 1, "count");
    ca_addChild(mCount);
}

class DuplicateEffectCaller : public PathEffectCaller {
public:
    DuplicateEffectCaller(const int count, const qreal dX, const qreal dY) :
        mCount(count), mDX(toSkScalar(dX)), mDY(toSkScalar(dY)) {}

    void apply(SkPath& path);
private:
    const int mCount;
    const float mDX;
    const float mDY;
};

void DuplicateEffectCaller::apply(SkPath &path) {
    const SkPath src = path;
    for(int i = 1; i <= mCount; i++)
        path.addPath(src, i*mDX, i*mDY);
}


stdsptr<PathEffectCaller> DuplicatePathEffect::getEffectCaller(const qreal relFrame) const {
    const int count = mCount->getEffectiveIntValue(relFrame);
    const qreal dX = mTranslation->getEffectiveXValue(relFrame);
    const qreal dY = mTranslation->getEffectiveYValue(relFrame);
    return enve::make_shared<DuplicateEffectCaller>(count, dX, dY);
}
