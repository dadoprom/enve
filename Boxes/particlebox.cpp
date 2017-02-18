#include "particlebox.h"
#include "mainwindow.h"


double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

ParticleBox::ParticleBox(BoxesGroup *parent) :
    BoundingBox(parent, TYPE_PARTICLES) {
    addEmitter(new ParticleEmitter(this));
}

void ParticleBox::getAccelerationAt(const QPointF &pos,
                                    const int &frame,
                                    QPointF *acc) {
    *acc = QPointF(0., 9.8)/24.;
}

void ParticleBox::updateAfterFrameChanged(int currentFrame) {
    BoundingBox::updateAfterFrameChanged(currentFrame);
    foreach(ParticleEmitter *emitter, mEmitters) {
        emitter->setFrame(currentFrame);
        emitter->scheduleUpdateParticlesForFrame();
    }
    scheduleAwaitUpdate();
}

void ParticleBox::updateBoundingRect() {
    mRelBoundingRect = QRectF();
    foreach(ParticleEmitter *emitter, mEmitters) {
        mRelBoundingRect = mRelBoundingRect.united(
                    emitter->getParticlesBoundingRect());
    }

    qreal effectsMargin = mEffectsMargin*mUpdateCanvasTransform.m11();
    mPixBoundingRect = mUpdateTransform.mapRect(mRelBoundingRect).
                        adjusted(-effectsMargin, -effectsMargin,
                                 effectsMargin, effectsMargin);
    BoundingBox::updateBoundingRect();
}

void ParticleBox::preUpdatePixmapsUpdates() {
    foreach(ParticleEmitter *emitter, mEmitters) {
        emitter->generateParticlesIfNeeded();
        emitter->updateParticlesForFrameIfNeeded();
    }

    BoundingBox::preUpdatePixmapsUpdates();
}

bool ParticleBox::relPointInsidePath(QPointF relPos) {
    if(mRelBoundingRectPath.contains(relPos) ) {
        foreach(ParticleEmitter *emitter, mEmitters) {
            if(emitter->relPointInsidePath(relPos)) {
                return true;
            }
        }
        return false;
    } else {
        return false;
    }
}

void ParticleBox::addEmitter(ParticleEmitter *emitter) {
    mEmitters << emitter;
    addActiveAnimator(emitter);
}

void ParticleBox::draw(QPainter *p)
{
    if(mVisible) {
        p->save();

        foreach(ParticleEmitter *emitter, mEmitters) {
            emitter->drawParticles(p);
        }

        p->restore();
    }
}

Particle::Particle(ParticleBox *parentBox) {
    mParentBox = parentBox;
}

void Particle::initializeParticle(int firstFrame, int nFrames,
                                  const QPointF &iniPos,
                                  const QPointF &iniVel,
                                  const qreal &partSize) {
    mSize = partSize;
    mPrevVelocityVar = QPointF(0., 0.);
    mNextVelocityVar = QPointF(0., 0.);
    mPrevVelocityDuration = 10000000.;
    mLastScale = 1.;
    mLastOpacity = 1.;

    mFirstFrame = firstFrame;
    mLastPos = iniPos;
    mLastVel = iniVel;
    if(mParticleStates != NULL) {
        if(nFrames == mNumberFrames) return;
        delete[] mParticleStates;
    }
    mNumberFrames = nFrames;
    mParticleStates = new ParticleState[nFrames];
}

void Particle::generatePathNextFrame(const int &frame,
                                     const qreal &velocityVar,
                                     const qreal &velocityVarPeriod,
                                     const QPointF &acc,
                                     const qreal &finalScale,
                                     const qreal &finalOpacity,
                                     const qreal &decayFrames,
                                     const qreal &length) {
    if(mPrevVelocityDuration > velocityVarPeriod) {
        mPrevVelocityVar = mNextVelocityVar;
        mNextVelocityVar = QPointF(fRand(-velocityVar, velocityVar),
                                   fRand(-velocityVar, velocityVar));
        mPrevVelocityDuration = 0.;
    }

    int arrayId = frame - mFirstFrame;

    if(arrayId == 0) {
        qreal iniTime = fRand(0., 1.);
        mLastPos += mLastVel*iniTime;
        mLastVel += acc*iniTime;
    }

    int remaining = mNumberFrames - arrayId;
    if(remaining <= decayFrames) {
        mLastScale += (finalScale - 1.)/decayFrames;
        mLastOpacity += (finalOpacity - 1.)/decayFrames;
    }

    if(length > 0.1) {
        QPainterPath linePath;
        qreal currLen = 0.;
        int currId = arrayId - 1;
        linePath.moveTo(mLastPos);
        QPointF lastPos = mLastPos;
        while(currId > -1) {
            QPointF currPos = mParticleStates[currId].pos;
            qreal lenInc = pointToLen(lastPos - currPos);
            qreal newLen = currLen + lenInc;
            if(newLen > length) {
                linePath.lineTo(lastPos + (currPos - lastPos)*
                                (length - currLen)/lenInc);
                break;
            } else {
                linePath.lineTo(currPos);
            }
            currLen = newLen;
            lastPos = currPos;
            currId--;
        }

        mParticleStates[arrayId] = ParticleState(mLastPos,
                                                 mLastScale,
                                                 mSize,
                                                 mLastOpacity,
                                                 linePath);
    } else {
        mParticleStates[arrayId] = ParticleState(mLastPos,
                                                 mLastScale,
                                                 mSize,
                                                 mLastOpacity);
    }

    qreal perPrevVelVar = (velocityVarPeriod - mPrevVelocityDuration)/
                            velocityVarPeriod;
    mLastPos += mLastVel +
            perPrevVelVar*mPrevVelocityVar +
            (1. - perPrevVelVar)*mNextVelocityVar;
    mLastVel += acc;

    mPrevVelocityDuration += 1.;
}

bool Particle::isVisibleAtFrame(const int &frame) {
    int arrayId = frame - mFirstFrame;
    if(arrayId < 0 || arrayId >= mNumberFrames) return false;
    return true;
}

ParticleState Particle::getParticleStateAtFrame(const int &frame) {
    int arrayId = frame - mFirstFrame;
    return mParticleStates[arrayId];
}

ParticleEmitter::ParticleEmitter() :
    ComplexAnimator() {

    setName("particle emitter");

    mPos.setName("pos");
    mPos.setCurrentValue(QPointF(0., 0.));

    mWidth.setName("width");
    mWidth.setValueRange(0., 6000.);
    mWidth.setCurrentValue(0.);

    mIniVelocity.setName("ini vel");
    mIniVelocity.setValueRange(-1000., 1000.);
    mIniVelocity.setCurrentValue(10.);

    mIniVelocityVar.setName("ini vel var");
    mIniVelocityVar.setValueRange(0., 1000.);
    mIniVelocityVar.setCurrentValue(5.);

    mIniVelocityAngle.setName("ini vel angle");
    mIniVelocityAngle.setValueRange(-3600., 3600.);
    mIniVelocityAngle.setCurrentValue(-90.);

    mIniVelocityAngleVar.setName("ini vel angle var");
    mIniVelocityAngleVar.setValueRange(0., 3600.);
    mIniVelocityAngleVar.setCurrentValue(15.);

    mAcceleration.setName("acceleration");
    mAcceleration.setValueRange(-100., 100.);
    mAcceleration.setCurrentValue(QPointF(0., 9.8));

    mParticlesPerSecond.setName("particles per second");
    mParticlesPerSecond.setValueRange(0., 10000.);
    mParticlesPerSecond.setCurrentValue(120);

    mParticlesFrameLifetime.setName("particles lifetime");
    mParticlesFrameLifetime.setValueRange(1., 1000.);
    mParticlesFrameLifetime.setCurrentValue(50.);

    mVelocityRandomVar.setName("velocity random var");
    mVelocityRandomVar.setValueRange(0., 1000.);
    mVelocityRandomVar.setCurrentValue(5.);

    mVelocityRandomVarPeriod.setName("velocity random var period");
    mVelocityRandomVarPeriod.setValueRange(1., 100.);
    mVelocityRandomVarPeriod.setCurrentValue(10.);

    mParticleSize.setName("particle size");
    mParticleSize.setValueRange(0., 100.);
    mParticleSize.setCurrentValue(5.);

    mParticleSizeVar.setName("particle size var");
    mParticleSizeVar.setValueRange(0., 100.);
    mParticleSizeVar.setCurrentValue(1.);

    mParticleLength.setName("length");
    mParticleLength.setValueRange(0., 2000.);
    mParticleLength.setCurrentValue(0.);

    mParticlesDecayFrames.setName("decay frames");
    mParticlesDecayFrames.setValueRange(0., 1000.);
    mParticlesDecayFrames.setCurrentValue(10.);

    mParticlesSizeDecay.setName("final scale");
    mParticlesSizeDecay.setValueRange(0., 10.);
    mParticlesSizeDecay.setCurrentValue(0.);

    mParticlesOpacityDecay.setName("final opacity");
    mParticlesOpacityDecay.setValueRange(0., 1.);
    mParticlesOpacityDecay.setCurrentValue(0.);

    mPos.blockPointer();
    mWidth.blockPointer();

    mIniVelocity.blockPointer();
    mIniVelocityVar.blockPointer();

    mIniVelocityAngle.blockPointer();
    mIniVelocityAngleVar.blockPointer();

    mAcceleration.blockPointer();

    mParticlesPerSecond.blockPointer();
    mParticlesFrameLifetime.blockPointer();

    mVelocityRandomVar.blockPointer();
    mVelocityRandomVarPeriod.blockPointer();

    mParticleSize.blockPointer();
    mParticleSizeVar.blockPointer();

    mParticleLength.blockPointer();

    mParticlesDecayFrames.blockPointer();
    mParticlesSizeDecay.blockPointer();
    mParticlesOpacityDecay.blockPointer();

    addChildAnimator(&mPos);
    addChildAnimator(&mWidth);

    addChildAnimator(&mIniVelocity);
    addChildAnimator(&mIniVelocityVar);

    addChildAnimator(&mIniVelocityAngle);
    addChildAnimator(&mIniVelocityAngleVar);

    addChildAnimator(&mAcceleration);

    addChildAnimator(&mParticlesPerSecond);
    addChildAnimator(&mParticlesFrameLifetime);

    addChildAnimator(&mVelocityRandomVar);
    addChildAnimator(&mVelocityRandomVarPeriod);

    addChildAnimator(&mParticleSize);
    addChildAnimator(&mParticleSizeVar);

    addChildAnimator(&mParticleLength);

    addChildAnimator(&mParticlesDecayFrames);
    addChildAnimator(&mParticlesSizeDecay);
    addChildAnimator(&mParticlesOpacityDecay);

    setUpdater(new ParticlesUpdater(this));
}

ParticleEmitter::ParticleEmitter(ParticleBox *parentBox) :
    ParticleEmitter(){
    setParentBox(parentBox);
}

void ParticleEmitter::scheduleGenerateParticles() {
    mGenerateParticlesScheduled = true;
    mParentBox->scheduleAwaitUpdate();
}

void ParticleEmitter::scheduleUpdateParticlesForFrame() {
    mUpdateParticlesForFrameScheduled = true;
    mParentBox->scheduleAwaitUpdate();
}

void ParticleEmitter::updateParticlesForFrameIfNeeded() {
    if(mUpdateParticlesForFrameScheduled) {
        mUpdateParticlesForFrameScheduled = false;
        updateParticlesForFrame(MainWindow::getInstance()->getCurrentFrame());
    }
}

void ParticleEmitter::generateParticlesIfNeeded() {
    if(mGenerateParticlesScheduled) {
        mGenerateParticlesScheduled = false;
        generateParticles();
    }
}

void ParticleEmitter::generateParticles() {
    qreal remainingPartFromFrame = 0.;
    QList<Particle*> notFinishedParticles;
    int nReuseParticles = mParticles.count();
    int currentReuseParticle = 0;
    bool reuseParticle = nReuseParticles > 0;

    int totalNeededParticles = 0;
    for(int i = mMinFrame; i < mMaxFrame; i++) {
        qreal iniVelocity =
                mIniVelocity.getCurrentValueAtFrame(i);
        qreal iniVelocityVar =
                mIniVelocityVar.getCurrentValueAtFrame(i);
        qreal iniVelocityAngle =
                mIniVelocityAngle.getCurrentValueAtFrame(i);
        qreal iniVelocityAngleVar =
                mIniVelocityAngleVar.getCurrentValueAtFrame(i);
        qreal particlesPerFrame =
                mParticlesPerSecond.getCurrentValueAtFrame(i)/24.;
        qreal particlesFrameLifetime =
                mParticlesFrameLifetime.getCurrentValueAtFrame(i);
        QPointF pos =
                mPos.getCurrentPointValueAtFrame(i);
        qreal width =
                mWidth.getCurrentValueAtFrame(i);
        qreal velocityVar =
                mVelocityRandomVar.getCurrentValueAtFrame(i);
        qreal velocityVarPeriod =
                mVelocityRandomVarPeriod.getCurrentValueAtFrame(i);
        QPointF acceleration = mAcceleration.getCurrentPointValueAtFrame(i)/24.;
        qreal finalScale =
                mParticlesSizeDecay.getCurrentValueAtFrame(i);
        qreal finalOpacity =
                mParticlesOpacityDecay.getCurrentValueAtFrame(i);
        qreal decayFrames =
                mParticlesDecayFrames.getCurrentValueAtFrame(i);
        qreal particleSize =
                mParticleSize.getCurrentValueAtFrame(i);
        qreal particleSizeVar =
                mParticleSizeVar.getCurrentValueAtFrame(i);
        qreal length = mParticleLength.getCurrentValueAtFrame(i);

        int particlesToCreate = remainingPartFromFrame + particlesPerFrame;
        remainingPartFromFrame += particlesPerFrame - particlesToCreate;
        if(remainingPartFromFrame < 0.) remainingPartFromFrame = 0.;

        for(int j = 0; j < particlesToCreate; j++) {
            Particle *newParticle;
            if(reuseParticle) {
                newParticle = mParticles.at(currentReuseParticle);
                currentReuseParticle++;
                reuseParticle = currentReuseParticle < nReuseParticles;
            } else {
                newParticle = new Particle(mParentBox);
                mParticles << newParticle;
            }
            qreal partVelAmp = fRand(iniVelocity - iniVelocityVar,
                                     iniVelocity + iniVelocityVar);


            QMatrix rotVelM;
            qreal velDeg = fRand(iniVelocityAngle - iniVelocityAngleVar,
                                 iniVelocityAngle + iniVelocityAngleVar);
            rotVelM.rotate(velDeg);
            QPointF partVel = rotVelM.map(QPointF(partVelAmp, 0.));

            qreal partSize = fRand(particleSize - particleSizeVar,
                                   particleSize + particleSizeVar);

            qreal xTrans = fRand(-width,
                                 width);

            newParticle->initializeParticle(i, particlesFrameLifetime,
                                            QPointF(pos.x() + xTrans,
                                                    pos.y()),
                                            partVel, partSize);
            notFinishedParticles << newParticle;
        }
        int nNotFinished = notFinishedParticles.count();
        int currPart = 0;
        while(currPart < nNotFinished) {
            Particle *particle = notFinishedParticles.at(currPart);

            if(particle->isVisibleAtFrame(i)) {
                particle->generatePathNextFrame(i,
                                                velocityVar,
                                                velocityVarPeriod,
                                                acceleration,
                                                finalScale,
                                                finalOpacity,
                                                decayFrames,
                                                length);
                currPart++;
            } else {
                notFinishedParticles.removeAt(currPart);
                nNotFinished--;
            }
        }
        totalNeededParticles += particlesToCreate;
    }
    int nToRemove = mParticles.count() - totalNeededParticles;
    for(int i = 0; i < nToRemove; i++) {
        delete mParticles.takeLast();
    }

    mUpdateParticlesForFrameScheduled = true;
}

void ParticleEmitter::drawParticles(QPainter *p) {
    p->save();
    if(mParticleLength.getCurrentValue() > 0.1) {
        QPen pen = QPen(Qt::black);
        pen.setCapStyle(Qt::RoundCap);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
    } else {
        p->setBrush(Qt::black);
        p->setPen(Qt::NoPen);
    }
    foreach(const ParticleState &state, mParticleStates) {
        state.draw(p);
    }
    p->restore();
}

void ParticleEmitter::updateParticlesForFrame(const int &frame) {
    mParticleStates.clear();
    foreach(Particle *particle, mParticles) {
        if(particle->isVisibleAtFrame(frame)) {
            mParticleStates << particle->getParticleStateAtFrame(frame);
        }
    }
    QRectF rect;
    foreach(const ParticleState &state, mParticleStates) {
        const QPointF &pos = state.pos;
        if(rect.isNull()) {
            rect.setTopLeft(pos);
            rect.setTopRight(pos);
        } else {
            if(rect.left() > pos.x()) {
                rect.setLeft(pos.x());
            } else if(rect.right() < pos.x()) {
                rect.setRight(pos.x());
            }
            if(rect.top() > pos.y()) {
                rect.setTop(pos.y());
            } else if(rect.bottom() < pos.y()) {
                rect.setBottom(pos.y());
            }
        }
    }
    if(!rect.isNull()) {
        rect.adjust(-5., -5, 5, 5);
    }
    mParticlesBoundingRect = rect;
}

QRectF ParticleEmitter::getParticlesBoundingRect() {
    return mParticlesBoundingRect;
}