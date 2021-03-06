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

#include "pathpointshandler.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "Animators/SmartPath/smartpathcollection.h"

PathPointsHandler::PathPointsHandler(
        SmartPathAnimator * const targetAnimator) :
    scheduleNodesRemoval([this]() { flushNodesRemoval(); }),
    mTargetAnimator(targetAnimator) {}

SmartNodePoint *PathPointsHandler::createNewNodePoint(const int nodeId) {
    const auto newPt = enve::make_shared<SmartNodePoint>(this, mTargetAnimator);
    insertPt(nodeId, newPt);
    return newPt.get();
}

SmartNodePoint *PathPointsHandler::createAndAssignNewNodePoint(const int nodeId) {
    const auto newPt = createNewNodePoint(nodeId);
    updatePoint(newPt, nodeId);
    return newPt;
}

SmartPath *PathPointsHandler::targetPath() const {
    return mTargetAnimator->getCurrentlyEditedPath();
}

void PathPointsHandler::updatePoint(const int nodeId) {
    const auto pt = getPointWithId<SmartNodePoint>(nodeId);
    updatePoint(pt, nodeId);
}

void PathPointsHandler::updatePoint(SmartNodePoint * const pt, const int nodeId) {
    pt->setNode(targetPath()->getNodePtr(nodeId));
}

void PathPointsHandler::updateAllPoints() {
    const int newCount = targetPath()->getNodeCount();
    while(newCount < count()) removeLast();
    for(int i = 0; i < count(); i++) getPointWithId<SmartNodePoint>(i)->clear();
    while(count() < newCount) createNewNodePoint(count());
    for(int i = 0; i < count(); i++) updatePoint(i);
}

void PathPointsHandler::setCtrlsMode(const int nodeId,
                                     const CtrlsMode mode) {
    mTargetAnimator->beforeBinaryPathChange();
    targetPath()->actionSetNormalNodeCtrlsMode(nodeId, mode);
    updatePoint(nodeId);
    mTargetAnimator->pathChanged();
}

void PathPointsHandler::removeNode(const int nodeId, const bool approx) {
    mTargetAnimator->actionRemoveNode(nodeId, approx);
}

SmartNodePoint* PathPointsHandler::addNewAtStart(const QPointF &relPos) {
    const int id = mTargetAnimator->actionAddNewAtStart(relPos);
    return getPointWithId<SmartNodePoint>(id);
}

SmartNodePoint* PathPointsHandler::addNewAtEnd(const QPointF &relPos) {
    const int id = mTargetAnimator->actionAddNewAtEnd(relPos);
    return getPointWithId<SmartNodePoint>(id);
}

void PathPointsHandler::promoteToNormal(const int nodeId) {
    mTargetAnimator->beforeBinaryPathChange();
    targetPath()->actionPromoteDissolvedNodeToNormal(nodeId);
    mTargetAnimator->pathChanged();

    const int prevNormalId = targetPath()->prevNormalId(nodeId);
    const int nextNormalId = targetPath()->nextNormalId(nodeId);
    updatePoints(qMin(prevNormalId, nextNormalId),
                 qMax(prevNormalId, nextNormalId));
}

void PathPointsHandler::demoteToDissolved(const int nodeId,
                                          const bool approx) {
    mTargetAnimator->beforeBinaryPathChange();
    targetPath()->actionDemoteToDissolved(nodeId, approx);
    mTargetAnimator->pathChanged();

    const int prevNormalId = targetPath()->prevNormalId(nodeId);
    const int nextNormalId = targetPath()->nextNormalId(nodeId);
    updatePoints(qMin(prevNormalId, nextNormalId),
                 qMax(prevNormalId, nextNormalId));
}

int PathPointsHandler::moveToClosestSegment(const int nodeId,
                                            const QPointF &relPos) {
    NormalSegment::SubSegment minSubSeg{nullptr, nullptr, nullptr};
    qreal minDist = TEN_MIL;
    for(int i = 0; i < count(); i++) {
        const auto pt = getPointWithId<SmartNodePoint>(i);
        const auto seg = pt->getNextNormalSegment();
        if(!seg.isValid()) continue;
        qreal dist;
        const auto subSeg = seg.getClosestSubSegmentForDummy(relPos, dist);
        if(dist < minDist) {
            minDist = dist;
            minSubSeg = subSeg;
        }
    }
    if(!minSubSeg.isValid()) return nodeId;
    const auto prevPt = minSubSeg.fFirstPt;
    const int prevNodeId = prevPt->getNodeId();
    if(prevNodeId == nodeId) return nodeId;
    const auto nextPt = minSubSeg.fLastPt;
    const int nextNodeId = nextPt->getNodeId();
    if(nextNodeId == nodeId) return nodeId;

    mTargetAnimator->actionMoveNodeBetween(nodeId, prevNodeId, nextNodeId);
    return (nodeId < prevNodeId ? prevNodeId : nextNodeId);
}

void PathPointsHandler::mergeNodes(const int nodeId1, const int nodeId2) {
    mTargetAnimator->actionMergeNodes(nodeId1, nodeId2);
}

SmartNodePoint* PathPointsHandler::divideSegment(const int node1Id,
                                                 const int node2Id,
                                                 const qreal t) {
    const int id = mTargetAnimator->actionInsertNodeBetween(node1Id, node2Id, t);
    return getPointWithId<SmartNodePoint>(id);
}

void PathPointsHandler::createSegment(const int node1Id, const int node2Id) {
    mTargetAnimator->actionConnectNodes(node1Id, node2Id);
}

void PathPointsHandler::removeSegment(const NormalSegment &segment) {
    if(!segment.isValid()) return;
    const auto node1 = segment.getNodeAt(0);
    const auto node2 = segment.getNodeAt(1);
    if(!node1 || !node2) return;
    const int node1Id = node1->getNodeId();
    const int node2Id = node2->getNodeId();
    mTargetAnimator->actionDisconnectNodes(node1Id, node2Id);
    if(mTargetAnimator->hasDetached()) {
        const auto spColl = mTargetAnimator->getParent<SmartPathCollection>();
        if(!spColl) return;
        const auto newAnim = mTargetAnimator->createFromDetached();
        spColl->addChild(newAnim);
    }
}

SmartNodePoint *PathPointsHandler::getClosestNode(
        const QPointF &absPos, const qreal &maxDist) const {
    SmartNodePoint* result = nullptr;
    qreal minDist = maxDist;
    for(int i = 0; i < count(); i++) {
        const auto sNode = getPointWithId<SmartNodePoint>(i);
        const qreal dist = pointToLen(sNode->getAbsolutePos() - absPos);
        if(dist > minDist) continue;
        minDist = dist;
        result = sNode;
    }
    return result;
}
