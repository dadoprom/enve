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

#ifndef ZIGZAGPATHEFFECT_H
#define ZIGZAGPATHEFFECT_H
#include "PathEffects/patheffect.h"

class ZigZagPathEffect : public PathEffect {
    e_OBJECT
protected:
    ZigZagPathEffect();
public:
    stdsptr<PathEffectCaller> getEffectCaller(const qreal relFrame) const;
private:
    qsptr<QrealAnimator> mAngle;
    qsptr<QrealAnimator> mDistance;
};

#endif // ZIGZAGPATHEFFECT_H
