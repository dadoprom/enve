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

#include "actions.h"
#include "Private/document.h"
#include "canvas.h"
#include "Paint/simplebrushwrapper.h"
#include "paintsettingsapplier.h"
Actions* Actions::sInstance = nullptr;

Actions::Actions(Document &document) : mDocument(document),
    mActiveScene(mDocument.fActiveScene) {
    Q_ASSERT(!sInstance);
    sInstance = this;
}

void Actions::undoAction() const {
    if(!mActiveScene) return;
    mActiveScene->undo();
    afterAction();
}

void Actions::redoAction() const {
    if(!mActiveScene) return;
    mActiveScene->redo();
    afterAction();
}

void Actions::raiseAction() const {
    if(!mActiveScene) return;
    mActiveScene->raiseSelectedBoxes();
    afterAction();
}

void Actions::lowerAction() const {
    if(!mActiveScene) return;
    mActiveScene->lowerSelectedBoxes();
    afterAction();
}

void Actions::raiseToTopAction() const {
    if(!mActiveScene) return;
    mActiveScene->raiseSelectedBoxesToTop();
    afterAction();
}

void Actions::lowerToBottomAction() const {
    if(!mActiveScene) return;
    mActiveScene->lowerSelectedBoxesToBottom();
    afterAction();
}

void Actions::objectsToPathAction() const {
    if(!mActiveScene) return;
    mActiveScene->convertSelectedBoxesToPath();
    afterAction();
}

void Actions::strokeToPathAction() const {
    if(!mActiveScene) return;
    mActiveScene->convertSelectedPathStrokesToPath();
    afterAction();
}

void Actions::rotate90CWAction() const {
    if(!mActiveScene) return;
    mActiveScene->rotateSelectedBoxesStartAndFinish(90);
    afterAction();
}

void Actions::rotate90CCWAction() const {
    if(!mActiveScene) return;
    mActiveScene->rotateSelectedBoxesStartAndFinish(-90);
    afterAction();
}

void Actions::flipHorizontalAction() const {
    if(!mActiveScene) return;
    mActiveScene->flipSelectedBoxesHorizontally();
    afterAction();
}

void Actions::flipVerticalAction() const {
    if(!mActiveScene) return;
    mActiveScene->flipSelectedBoxesVertically();
    afterAction();
}

void Actions::pathsUnionAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsUnion();
    afterAction();
}

void Actions::pathsDifferenceAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsDifference();
    afterAction();
}

void Actions::pathsIntersectionAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsIntersection();
    afterAction();
}

void Actions::pathsDivisionAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsDivision();
    afterAction();
}

void Actions::pathsExclusionAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsExclusion();
    afterAction();
}

void Actions::pathsCombineAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsCombine();
    afterAction();
}

void Actions::pathsBreakApartAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectedPathsBreakApart();
    afterAction();
}

void Actions::setFontFamilyAndStyle(const QString& family,
                                    const QString& style) const {
    if(!mActiveScene) return;
    mDocument.fFontFamily = family;
    mDocument.fFontStyle = style;
    mActiveScene->setSelectedFontFamilyAndStyle(family, style);
    afterAction();
}

void Actions::setFontSize(const qreal size) const {
    if(!mActiveScene) return;
    mDocument.fFontSize = size;
    mActiveScene->setSelectedFontSize(size);
    afterAction();
}

void Actions::connectPointsSlot() const {
    if(!mActiveScene) return;
    mActiveScene->connectPoints();
    afterAction();
}

void Actions::disconnectPointsSlot() const {
    if(!mActiveScene) return;
    mActiveScene->disconnectPoints();
    afterAction();
}

void Actions::mergePointsSlot() const {
    if(!mActiveScene) return;
    mActiveScene->mergePoints();
    afterAction();
}

void Actions::subdivideSegments() const {
    if(!mActiveScene) return;
    mActiveScene->subdivideSegments();
    afterAction();
}

void Actions::makePointCtrlsSymmetric() const {
    if(!mActiveScene) return;
    mActiveScene->makePointCtrlsSymmetric();
    afterAction();
}

void Actions::makePointCtrlsSmooth() const {
    if(!mActiveScene) return;
    mActiveScene->makePointCtrlsSmooth();
    afterAction();
}

void Actions::makePointCtrlsCorner() const {
    if(!mActiveScene) return;
    mActiveScene->makePointCtrlsCorner();
    afterAction();
}

void Actions::makeSegmentLine() const {
    if(!mActiveScene) return;
    mActiveScene->makeSegmentLine();
    afterAction();
}

void Actions::makeSegmentCurve() const {
    if(!mActiveScene) return;
    mActiveScene->makeSegmentCurve();
    afterAction();
}

void Actions::newEmptyPaintFrame() {
    if(!mActiveScene) return;
    mActiveScene->newEmptyPaintFrameAction();
    afterAction();
}

void Actions::deleteAction() const {
    if(!mActiveScene) return;
    mActiveScene->deleteAction();
    afterAction();
}

void Actions::copyAction() const {
    if(!mActiveScene) return;
    mActiveScene->copyAction();
}

void Actions::pasteAction() const {
    if(!mActiveScene) return;
    mActiveScene->pasteAction();
    afterAction();
}

void Actions::cutAction() const {
    if(!mActiveScene) return;
    mActiveScene->cutAction();
    afterAction();
}

void Actions::duplicateAction() const {
    if(!mActiveScene) return;
    mActiveScene->duplicateAction();
    afterAction();
}

void Actions::selectAllAction() const {
    if(!mActiveScene) return;
    mActiveScene->selectAllAction();
}

void Actions::invertSelectionAction() const {
    if(!mActiveScene) return;
    mActiveScene->invertSelectionAction();
}

void Actions::clearSelectionAction() const {
    if(!mActiveScene) return;
    mActiveScene->clearSelectionAction();
}

void Actions::groupSelectedBoxes() const {
    if(!mActiveScene) return;
    mActiveScene->groupSelectedBoxes();
    afterAction();
}

void Actions::ungroupSelectedBoxes() const {
    if(!mActiveScene) return;
    mActiveScene->ungroupSelectedBoxes();
    afterAction();
}

void Actions::startSelectedStrokeColorTransform() const {
    if(!mActiveScene) return;
    mActiveScene->startSelectedStrokeColorTransform();
    afterAction();
}

void Actions::startSelectedFillColorTransform() const {
    if(!mActiveScene) return;
    mActiveScene->startSelectedFillColorTransform();
    afterAction();
}

void Actions::strokeCapStyleChanged(const SkPaint::Cap capStyle) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedCapStyle(capStyle);
    afterAction();
}

void Actions::strokeJoinStyleChanged(const SkPaint::Join joinStyle) const {
    if(!mActiveScene) return;
    mActiveScene->setSelectedJoinStyle(joinStyle);
    afterAction();
}

void Actions::strokeWidthAction(const QrealAction &action) const {
    if(!mActiveScene) return;
    mActiveScene->strokeWidthAction(action);
    afterAction();
}

void Actions::applyPaintSettingToSelected(
        const PaintSettingsApplier &setting) const {
    if(!mActiveScene) return;
    mActiveScene->applyPaintSettingToSelected(setting);
    afterAction();
}

void Actions::updateAfterFrameChanged(const int currentFrame) const {
    if(!mActiveScene) return;
    mActiveScene->anim_setAbsFrame(currentFrame);
    afterAction();
}

void Actions::setClipToCanvas(const bool clip) {
    if(!mActiveScene) return;
    if(mActiveScene->clipToCanvas() == clip) return;
    mActiveScene->setClipToCanvas(clip);
    mActiveScene->updateAllBoxes(UpdateReason::userChange);
    mActiveScene->sceneFramesUpToDate();
    afterAction();
}

void Actions::setRasterEffectsVisible(const bool bT) {
    if(!mActiveScene) return;
    mActiveScene->setRasterEffectsVisible(bT);
    mActiveScene->updateAllBoxes(UpdateReason::userChange);
    afterAction();
}

void Actions::setPathEffectsVisible(const bool bT) {
    if(!mActiveScene) return;
    mActiveScene->setPathEffectsVisible(bT);
    mActiveScene->updateAllBoxes(UpdateReason::userChange);
    afterAction();
}

#include "filesourcescache.h"
//#include "svgimporter.h"
#include "Boxes/videobox.h"
#include "Boxes/imagebox.h"
#include "importhandler.h"

void Actions::importFile(const QString &path,
                         const QPointF &relDropPos) {
    if(!mActiveScene) return;

    const QFile file(path);
    if(!file.exists())
        RuntimeThrow("File " + path + " does not exit.");

    QFileInfo fInfo(path);
    if(fInfo.isDir()) {
        mActiveScene->createImageSequenceBox(path);
    } else { // is file
        const QString extension = path.split(".").last();
        if(isSoundExt(extension)) {
            mActiveScene->createSoundForPath(path);
        } else {
            qsptr<BoundingBox> importedBox;
            mActiveScene->blockUndoRedo();
            if(isImageExt(extension)) {
                mActiveScene->createImageBox(path);
            } else if(isVideoExt(extension)) {
                mActiveScene->createVideoForPath(path);
            } else {
                try {
                    importedBox = ImportHandler::sInstance->import(path);
                } catch(const std::exception& e) {
                    mActiveScene->unblockUndoRedo();
                    gPrintExceptionCritical(e);
                    return;
                }
            }
            mActiveScene->unblockUndoRedo();

            if(importedBox) {
                importedBox->planCenterPivotPosition();
                mActiveScene->getCurrentGroup()->addContained(importedBox);
                importedBox->startPosTransform();
                importedBox->moveByAbs(relDropPos);
                importedBox->finishTransform();
            }
        }
    }
    afterAction();
}

void Actions::setMovePathMode() {
    mDocument.setCanvasMode(CanvasMode::boxTransform);
}

void Actions::setMovePointMode() {
    mDocument.setCanvasMode(CanvasMode::pointTransform);
}

void Actions::setAddPointMode() {
    mDocument.setCanvasMode(CanvasMode::pathCreate);
}

void Actions::setRectangleMode() {
    mDocument.setCanvasMode(CanvasMode::rectCreate);
}

void Actions::setPickPaintSettingsMode() {
    mDocument.setCanvasMode(CanvasMode::pickFillStroke);
}

void Actions::setCircleMode() {
    mDocument.setCanvasMode(CanvasMode::circleCreate);
}

void Actions::setTextMode() {
    mDocument.setCanvasMode(CanvasMode::textCreate);
}

void Actions::setPaintMode() {
    mDocument.setCanvasMode(CanvasMode::paint);
}

void Actions::finishSmoothChange() {
    mSmoothChange = false;
//    mDocument.actionFinished();
}

void Actions::afterAction() const {
    Document::sInstance->actionFinished();
}
