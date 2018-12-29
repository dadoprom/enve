#ifndef GRAPHANIMATOR_H
#define GRAPHANIMATOR_H
#include "animator.h"
#define GetAsGK(key) GetAsPtr(key, GraphKey)
class GraphKey;
class GraphAnimator : public Animator {
public:
    GraphAnimator(const QString& name);

    bool SWT_isGraphAnimator() { return true; }
    virtual void anim_getMinAndMaxValues(
            qreal &minValP, qreal &maxValP) const;
    virtual void anim_getMinAndMaxValuesBetweenFrames(
            const int &startFrame, const int &endFrame,
            qreal &minValP, qreal &maxValP) const;
    virtual qreal clampGraphValue(const qreal &value) { return value; }
    virtual bool anim_graphValuesCorrespondToFrames() const {
        return true;
    }
    virtual void anim_updateKeysPath();

    void addKeysInRectToList(const QRectF &frameValueRect,
                             QList<GraphKey*> &keys);


    void incSelectedForGraph() {
        if(!anim_mSelected) anim_updateKeysPath();
        anim_mSelected++;
    }

    void decSelectedForGraph() {
        anim_mSelected--;
    }

    bool isSelectedForGraph() {
        return anim_mSelected;
    }

    QColor getAnimatorColor(void *ptr) const {
        for(const auto& x : mAnimatorColors) {
            if(x.first == ptr) {
                return x.second;
            }
        }
        return QColor();
    }

    void setAnimatorColor(void *ptr, const QColor &color) {
        mAnimatorColors[ptr] = color;
    }
    void removeAnimatorColor(void *ptr) {
        mAnimatorColors.erase(ptr);
    }

    bool isCurrentAnimator(void *ptr) const {
        return mAnimatorColors.find(ptr) != mAnimatorColors.end();
    }
    void drawKeysPath(QPainter * const p,
                      const QColor &paintColor) const;
    void getMinAndMaxMoveFrame(GraphKey *key,
                               QrealPoint * const currentPoint,
                               qreal &minMoveFrame,
                               qreal &maxMoveFrame);
    void anim_constrainCtrlsFrameValues();

    QrealPoint *anim_getPointAt(const qreal &value,
                                const qreal &frame,
                                const qreal &pixelsPerFrame,
                                const qreal &pixelsPerValUnit);
    void changeSelectedKeysFrameAndValueStart(const QPointF &frameVal);
    void changeSelectedKeysFrameAndValue(const QPointF& frameVal);
    void enableCtrlPtsForSelected();
    void setCtrlsModeForSelectedKeys(const CtrlsMode &mode);

    void anim_appendKey(const stdsptr<Key> &newKey,
                        const bool &saveUndoRedo,
                        const bool &update);
    void anim_removeKey(const stdsptr<Key> &keyToRemove,
                        const bool &saveUndoRedo);
    void getSelectedSegments(QList<QList<GraphKey*>> &segments);
protected:
    QPainterPath mKeysPath;
private:
    int anim_mSelected = 0;

    std::map<void*, QColor> mAnimatorColors;
};

#endif // GRAPHANIMATOR_H