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

#include "eboxorsound.h"

#include "canvas.h"
#include "Timeline/durationrectangle.h"
#include "Properties/emimedata.h"

eBoxOrSound::eBoxOrSound(const QString &name) :
    StaticComplexAnimator(name) {
    connect(this, &Property::prp_nameChanged, this,
            &SingleWidgetTarget::SWT_scheduleSearchContentUpdate);
}

void eBoxOrSound::setParentGroup(ContainerBox * const parent) {
    if(parent == mParentGroup) return;
    emit aboutToChangeAncestor();
    if(mParentGroup) {
        disconnect(mParentGroup, &eBoxOrSound::aboutToChangeAncestor,
                   this, &eBoxOrSound::aboutToChangeAncestor);
    }
    prp_afterWholeInfluenceRangeChanged();
    mParentGroup = parent;
    if(mParentGroup) {
        anim_setAbsFrame(mParentGroup->anim_getCurrentAbsFrame());
        connect(mParentGroup, &eBoxOrSound::aboutToChangeAncestor,
                this, &eBoxOrSound::aboutToChangeAncestor);
    }

    setParent(mParentGroup);
    emit parentChanged(parent);
}

void eBoxOrSound::removeFromParent_k() {
    if(!mParentGroup) return;
    mParentGroup->removeContained_k(ref<eBoxOrSound>());
}

bool eBoxOrSound::isAncestor(const BoundingBox * const box) const {
    if(!mParentGroup) return false;
    if(mParentGroup == box) return true;
    if(box->SWT_isContainerBox()) return mParentGroup->isAncestor(box);
    return false;
}

bool eBoxOrSound::isFrameInDurationRect(const int relFrame) const {
    if(!mDurationRectangle) return true;
    return relFrame <= mDurationRectangle->getMaxRelFrame() &&
            relFrame >= mDurationRectangle->getMinRelFrame();
}

bool eBoxOrSound::isFrameFInDurationRect(const qreal relFrame) const {
    if(!mDurationRectangle) return true;
    return qCeil(relFrame) <= mDurationRectangle->getMaxRelFrame() &&
           qFloor(relFrame) >= mDurationRectangle->getMinRelFrame();
}

void eBoxOrSound::shiftAll(const int shift) {
    if(hasDurationRectangle()) mDurationRectangle->changeFramePosBy(shift);
    else anim_shiftAllKeys(shift);
}

QMimeData *eBoxOrSound::SWT_createMimeData() {
    return new eMimeData(QList<eBoxOrSound*>() << this);
}

FrameRange eBoxOrSound::prp_relInfluenceRange() const {
    if(mDurationRectangle) return mDurationRectangle->getRelFrameRange();
    return ComplexAnimator::prp_relInfluenceRange();
}

FrameRange eBoxOrSound::prp_getIdenticalRelRange(const int relFrame) const {
    if(mVisible) {
        const auto cRange = ComplexAnimator::prp_getIdenticalRelRange(relFrame);
        if(mDurationRectangle) {
            const auto dRange = mDurationRectangle->getRelFrameRange();
            if(relFrame > dRange.fMax) {
                return mDurationRectangle->getRelFrameRangeToTheRight();
            } else if(relFrame < dRange.fMin) {
                return mDurationRectangle->getRelFrameRangeToTheLeft();
            } else return cRange*dRange;
        }
        return cRange;
    }
    return {FrameRange::EMIN, FrameRange::EMAX};
}

int eBoxOrSound::prp_getRelFrameShift() const {
    if(!mDurationRectangle) return 0;
    return mDurationRectangle->getRelShift();
}

void eBoxOrSound::prp_writeProperty(eWriteStream& dst) const {
    StaticComplexAnimator::prp_writeProperty(dst);
    dst << mVisible;
    dst << mLocked;

    const bool hasDurRect = mDurationRectangle;
    dst << hasDurRect;
    if(hasDurRect) mDurationRectangle->writeDurationRectangle(dst);
}

void eBoxOrSound::prp_readProperty(eReadStream& src) {
    StaticComplexAnimator::prp_readProperty(src);
    src >> mVisible;
    src >> mLocked;

    bool hasDurRect;
    src >> hasDurRect;
    if(hasDurRect) {
        if(!mDurationRectangle) createDurationRectangle();
        mDurationRectangle->readDurationRectangle(src);
    }
}

TimelineMovable *eBoxOrSound::anim_getTimelineMovable(
        const int relX, const int minViewedFrame,
        const qreal pixelsPerFrame) {
    if(!mDurationRectangle) return nullptr;
    return mDurationRectangle->getMovableAt(relX, pixelsPerFrame,
                                            minViewedFrame);
}

void eBoxOrSound::prp_drawTimelineControls(
        QPainter * const p, const qreal pixelsPerFrame,
        const FrameRange &absFrameRange, const int rowHeight) {
    if(mDurationRectangle) {
        p->save();
        const int width = qCeil(absFrameRange.span()*pixelsPerFrame);
        const QRect drawRect(0, 0, width, rowHeight);
        const auto pScene = getParentScene();
        const qreal fps = pScene ? pScene->getFps() : 1;
        mDurationRectangle->draw(p, drawRect, fps,
                                 pixelsPerFrame, absFrameRange);
        p->restore();
    }

    ComplexAnimator::prp_drawTimelineControls(
                p, pixelsPerFrame, absFrameRange, rowHeight);
}

void eBoxOrSound::setDurationRectangle(
        const qsptr<DurationRectangle>& durationRect) {
    if(durationRect == mDurationRectangle) return;
    Q_ASSERT(!mDurationRectangleLocked);
    if(mDurationRectangle) {
        disconnect(mDurationRectangle.data(), nullptr, this, nullptr);
    }
    const FrameRange oldRange = mDurationRectangle ?
                mDurationRectangle->getAbsFrameRange() :
                FrameRange{FrameRange::EMIN, FrameRange::EMAX};
    const FrameRange newRange = durationRect ?
                durationRect->getAbsFrameRange() :
                FrameRange{FrameRange::EMIN, FrameRange::EMAX};
    const auto oldDurRect = mDurationRectangle;
    mDurationRectangle = durationRect;
    prp_afterFrameShiftChanged(oldRange, newRange);
    if(!mDurationRectangle) return shiftAll(oldDurRect->getRelShift());

    connect(mDurationRectangle.data(), &DurationRectangle::shiftChanged,
            this, [this](const int oldShift, const int newShift) {
        const auto newRange = prp_absInfluenceRange();
        const auto oldRange = newRange.shifted(oldShift - newShift);
        prp_afterFrameShiftChanged(oldRange, newRange);
    });

    connect(mDurationRectangle.data(), &DurationRectangle::minRelFrameChanged,
            this, [this](const int oldMin, const int newMin) {
        const int min = qMin(newMin, oldMin);
        const int max = qMax(newMin, oldMin);
        prp_afterChangedRelRange(FrameRange{min, max}.adjusted(-1, 1), false);
    });
    connect(mDurationRectangle.data(), &DurationRectangle::maxRelFrameChanged,
            this, [this](const int oldMax, const int newMax) {
        const int min = qMin(newMax, oldMax);
        const int max = qMax(newMax, oldMax);
        prp_afterChangedRelRange(FrameRange{min, max}.adjusted(-1, 1), false);
    });
}

bool eBoxOrSound::isVisibleAndInVisibleDurationRect() const {
    return isFrameInDurationRect(anim_getCurrentRelFrame()) && mVisible;
}

bool eBoxOrSound::isVisibleAndInDurationRect(
        const int relFrame) const {
    return isFrameInDurationRect(relFrame) && mVisible;
}

bool eBoxOrSound::isFrameFVisibleAndInDurationRect(
        const qreal relFrame) const {
    return isFrameFInDurationRect(relFrame) && mVisible;
}

bool eBoxOrSound::hasDurationRectangle() const {
    return mDurationRectangle;
}

void eBoxOrSound::startDurationRectPosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startPosTransform();
    }
}

void eBoxOrSound::finishDurationRectPosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishPosTransform();
    }
}

void eBoxOrSound::moveDurationRect(const int dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->changeFramePosBy(dFrame);
    }
}

void eBoxOrSound::startMinFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startMinFramePosTransform();
    }
}

void eBoxOrSound::finishMinFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishMinFramePosTransform();
    }
}

void eBoxOrSound::moveMinFrame(const int dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->moveMinFrame(dFrame);
    }
}

void eBoxOrSound::startMaxFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->startMaxFramePosTransform();
    }
}

void eBoxOrSound::finishMaxFramePosTransform() {
    if(hasDurationRectangle()) {
        mDurationRectangle->finishMaxFramePosTransform();
    }
}

void eBoxOrSound::moveMaxFrame(const int dFrame) {
    if(hasDurationRectangle()) {
        mDurationRectangle->moveMaxFrame(dFrame);
    }
}

DurationRectangle *eBoxOrSound::getDurationRectangle() const {
    return mDurationRectangle.get();
}

void eBoxOrSound::createDurationRectangle() {
    const auto durRect = enve::make_shared<DurationRectangle>(*this);
//    durRect->setMinFrame(0);
//    if(mParentScene) durRect->setFramesDuration(mParentScene->getFrameCount());
    durRect->setMinRelFrame(anim_getCurrentRelFrame() - 5);
    durRect->setFramesDuration(10);
    setDurationRectangle(durRect);
}

void eBoxOrSound::setSelected(const bool select) {
    if(mSelected == select) return;
    mSelected = select;
    SWT_scheduleContentUpdate(SWT_BR_SELECTED);
    emit selectionChanged(select);
}

void eBoxOrSound::select() {
    setSelected(true);
}

void eBoxOrSound::deselect() {
    setSelected(false);
}

void eBoxOrSound::selectionChangeTriggered(const bool shiftPressed) {
    const auto pScene = getParentScene();
    if(!pScene) return;
    if(!SWT_isBoundingBox()) return;
    const auto bb = static_cast<BoundingBox*>(this);
    if(shiftPressed) {
        if(mSelected) {
            pScene->removeBoxFromSelection(bb);
        } else {
            pScene->addBoxToSelection(bb);
        }
    } else {
        pScene->clearBoxesSelection();
        pScene->addBoxToSelection(bb);
    }
}

void eBoxOrSound::setVisibile(const bool visible) {
    if(mVisible == visible) return;
    mVisible = visible;

    prp_afterWholeInfluenceRangeChanged();

    SWT_scheduleContentUpdate(SWT_BR_VISIBLE);
    SWT_scheduleContentUpdate(SWT_BR_HIDDEN);

    emit visibilityChanged(visible);
}

void eBoxOrSound::switchVisible() {
    setVisibile(!mVisible);
}

void eBoxOrSound::switchLocked() {
    setLocked(!mLocked);
}

void eBoxOrSound::hide() {
    setVisibile(false);
}

void eBoxOrSound::show() {
    setVisibile(true);
}

bool eBoxOrSound::isVisible() const {
    return mVisible;
}

bool eBoxOrSound::isVisibleAndUnlocked() const {
    return isVisible() && !mLocked;
}

bool eBoxOrSound::isLocked() const {
    return mLocked;
}

void eBoxOrSound::lock() {
    setLocked(true);
}

void eBoxOrSound::unlock() {
    setLocked(false);
}

void eBoxOrSound::setLocked(const bool locked) {
    if(locked == mLocked) return;
    const auto pScene = getParentScene();
    if(pScene && mSelected && SWT_isBoundingBox()) {
        const auto bb = static_cast<BoundingBox*>(this);
        pScene->removeBoxFromSelection(bb);
    }
    mLocked = locked;
    SWT_scheduleContentUpdate(SWT_BR_LOCKED);
    SWT_scheduleContentUpdate(SWT_BR_UNLOCKED);
    emit lockedChanged(locked);
}

void eBoxOrSound::moveUp() {
    mParentGroup->decreaseContainedZInList(this);
}

void eBoxOrSound::moveDown() {
    mParentGroup->increaseContainedZInList(this);
}

void eBoxOrSound::bringToFront() {
    mParentGroup->bringContainedToFrontList(this);
}

void eBoxOrSound::bringToEnd() {
    mParentGroup->bringContainedToEndList(this);
}
