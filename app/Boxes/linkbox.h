#ifndef LINKBOX_H
#define LINKBOX_H
#include "canvas.h"
#include "Properties/boxtargetproperty.h"
#include "Properties/boolproperty.h"

class ExternalLinkBox : public BoxesGroup {
    Q_OBJECT
    friend class SelfRef;
public:
    void reload();

    void changeSrc(QWidget *dialogParent);

    void setSrc(const QString &src);
private:
    ExternalLinkBox();
    QString mSrc;
};

class InternalLinkBox : public BoundingBox {
    Q_OBJECT
    friend class SelfRef;
protected:
    InternalLinkBox(BoundingBox * const linkTarget);
public:
    ~InternalLinkBox() {
        setLinkTarget(nullptr);
    }

    bool SWT_isLinkBox() const { return true; }

    void writeBoundingBox(QIODevice *target) {
        BoundingBox::writeBoundingBox(target);
        mBoxTarget->writeProperty(target);
    }

    void readBoundingBox(QIODevice *target) {
        BoundingBox::readBoundingBox(target);
        mBoxTarget->readProperty(target);
    }

    void setLinkTarget(BoundingBox * const linkTarget) {
        disconnect(mBoxTarget.data(), nullptr, this, nullptr);
        if(getLinkTarget()) {
            disconnect(getLinkTarget(), nullptr, this, nullptr);
            getLinkTarget()->removeLinkingBox(this);
        }
        if(!linkTarget) {
            setName("empty link");
            mBoxTarget->setTarget(nullptr);
        } else {
            setName(linkTarget->getName() + " link");
            mBoxTarget->setTarget(linkTarget);
            linkTarget->addLinkingBox(this);
            connect(linkTarget, &BoundingBox::prp_absFrameRangeChanged,
                    this, &BoundingBox::prp_updateAfterChangedRelFrameRange);
        }
        scheduleUpdate(Animator::USER_CHANGE);
        connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
                this, &InternalLinkBox::setTargetSlot);
    }

    bool relPointInsidePath(const QPointF &relPos) const;
    QPointF getRelCenterPosition();

    qsptr<BoundingBox> createLink();

    qsptr<BoundingBox> createLinkForLinkGroup();

    stdsptr<BoundingBoxRenderData> createRenderData();
    void setupRenderData(const qreal &relFrame,
                                                BoundingBoxRenderData * const data);
    const SkBlendMode &getBlendMode() {
        if(mParentGroup->SWT_isLinkBox()) {
            return getLinkTarget()->getBlendMode();
        }
        return BoundingBox::getBlendMode();
    }

    qreal getEffectsMarginAtRelFrame(const int &relFrame) {
        qreal margin = 0.;
        margin += getLinkTarget()->getEffectsMarginAtRelFrame(relFrame);
        margin += BoundingBox::getEffectsMarginAtRelFrame(relFrame);
        return margin;
    }

    QRectF getRelBoundingRectAtRelFrame(const qreal &relFrame);
    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;

    QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame) {
        if(mParentGroup ? mParentGroup->SWT_isLinkBox() : false) {
            return getLinkTarget()->getRelativeTransformAtRelFrameF(relFrame);
        } else {
            return BoundingBox::getRelativeTransformAtRelFrameF(relFrame);
        }
    }
    bool isRelFrameInVisibleDurationRect(const int &relFrame) const;
    bool isRelFrameFInVisibleDurationRect(const qreal &relFrame) const;
    BoundingBox *getLinkTarget() const;
public slots:
    void setTargetSlot(BoundingBox *target) {
        setLinkTarget(target);
    }
protected:
    qsptr<BoxTargetProperty> mBoxTarget =
            SPtrCreate(BoxTargetProperty)("link target");
};

#endif // LINKBOX_H
