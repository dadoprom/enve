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

#ifndef SMARTPATHCOLLECTION_H
#define SMARTPATHCOLLECTION_H
#include <QList>
#include "../dynamiccomplexanimator.h"
#include "smartpathanimator.h"
#include "../../MovablePoints/segment.h"

class SmartPathAnimator;
class SmartNodePoint;
typedef DynamicComplexAnimator<SmartPathAnimator> SmartPathCollectionBase;

class SmartPathCollection : public SmartPathCollectionBase {
    Q_OBJECT
    e_OBJECT
protected:
    SmartPathCollection();
public:
    bool SWT_isSmartPathCollection() const { return true; }

    void prp_writeProperty(eWriteStream& dst) const;
    void prp_readProperty(eReadStream& src);

    SmartNodePoint * createNewSubPathAtRelPos(const QPointF &relPos);
    SmartNodePoint * createNewSubPathAtPos(const QPointF &absPos);

    void moveAllFrom(SmartPathCollection * const from);

    SkPath getPathAtRelFrame(const qreal relFrame) const;

    void applyTransform(const QMatrix &transform) const;

    void loadSkPath(const SkPath& path);

    void setFillType(const SkPath::FillType fillType);
    SkPath::FillType getFillType() const
    { return mFillType; }
signals:
    void fillTypeChanged(SkPath::FillType);
private:
    SmartPathAnimator *createNewPath();
    SmartPathAnimator *createNewPath(const SkPath& path);

    void updatePathColors();

    SkPath::FillType mFillType = SkPath::kWinding_FillType;
};

#endif // SMARTPATHCOLLECTION_H
