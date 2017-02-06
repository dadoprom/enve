#ifndef PARTICLEBOX_H
#define PARTICLEBOX_H
#include "boundingbox.h"

class ParticleBox;

struct ParticleState {
    ParticleState() {}
    ParticleState(const QPointF &posT,
                  const qreal &scaleT,
                  const qreal &sizeT,
                  const qreal &opacityT) {
        pos = posT;
        scale = scaleT;
        size = sizeT;
        opacity = opacityT;
    }

    QPointF pos;
    qreal scale;
    qreal size;
    qreal opacity;
};

class Particle {
public:
    Particle(ParticleBox *parentBox);
    void initializeParticle(int firstFrame,
                            int nFrames,
                            const QPointF &iniPos,
                            const QPointF &iniVel,
                            const qreal &partSize);
    void generatePathNextFrame(const int &frame,
                               const qreal &velocityVar,
                               const qreal &velocityVarPeriod,
                               const QPointF &acc,
                               const qreal &finalScale,
                               const qreal &finalOpacity,
                               const qreal &decayFrames);

    bool isVisibleAtFrame(const int &frame);
    ParticleState getParticleStateAtFrame(const int &frame);
private:
    qreal mSize;
    QPointF mPrevVelocityVar;
    QPointF mNextVelocityVar;
    qreal mPrevVelocityDuration;

    ParticleBox *mParentBox;
    int mFirstFrame;
    int mNumberFrames;
    ParticleState *mParticleStates = NULL;

    qreal mLastScale;
    qreal mLastOpacity;
    QPointF mLastPos;
    QPointF mLastVel;
};

class ParticleEmitter : public ComplexAnimator {
public:
    ParticleEmitter(ParticleBox *parentBox);

    void generateParticles();

    void drawParticles(QPainter *p);
    void updateParticlesForFrame(const int &frame);
    QRectF getParticlesBoundingRect();
    void scheduleGenerateParticles();
    void scheduleUpdateParticlesForFrame();
    void generateParticlesIfNeeded();
    void updateParticlesForFrameIfNeeded();
    bool relPointInsidePath(const QPointF &relPos) {
        foreach(const ParticleState &state, mParticleStates) {
            if(pointToLen(state.pos - relPos) < 5.) {
                return true;
            }
        }

        return false;
    }

private:
    QRectF mParticlesBoundingRect;
    bool mGenerateParticlesScheduled = false;
    bool mUpdateParticlesForFrameScheduled = false;
    int mMinFrame = 0;
    int mMaxFrame = 200;

    QList<ParticleState> mParticleStates;
    QList<Particle*> mParticles;
    ParticleBox *mParentBox;

    QPointFAnimator mPos;
    QrealAnimator mWidth;

    QrealAnimator mIniVelocity;
    QrealAnimator mIniVelocityVar;
    QrealAnimator mIniVelocityAngle;
    QrealAnimator mIniVelocityAngleVar;
    QPointFAnimator mAcceleration;

    QrealAnimator mParticlesPerSecond;
    QrealAnimator mParticlesFrameLifetime;

    QrealAnimator mVelocityRandomVar;
    QrealAnimator mVelocityRandomVarPeriod;

    QrealAnimator mParticleSize;
    QrealAnimator mParticleSizeVar;

//    QrealAnimator mParticleAspectRatio;

//    QrealAnimator mParticleRotation;
//    QrealAnimator mParticleRotationVar;

//    QrealAnimator mParticleRotationRandomVar;
//    QrealAnimator mParticleRotationRandomVarPeriod;

    QrealAnimator mParticlesDecayFrames;
    QrealAnimator mParticlesSizeDecay;
    QrealAnimator mParticlesOpacityDecay;
};

class ParticleBox : public BoundingBox
{
public:
    ParticleBox(BoxesGroup *parent);
    void getAccelerationAt(const QPointF &pos,
                           const int &frame,
                           QPointF *acc);
    void updateAfterFrameChanged(int currentFrame);
    void draw(QPainter *p);
    void updateBoundingRect();
    void preUpdatePixmapsUpdates();
    bool relPointInsidePath(QPointF relPos);
private:
    QList<ParticleEmitter*> mEmitters;
};

#endif // PARTICLEBOX_H
