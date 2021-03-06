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

#include "keysview.h"
#include "Animators/qrealkey.h"
#include "clipboardcontainer.h"
#include <QPainter>
#include <QMenu>
#include "mainwindow.h"
#include "GUI/BoxesList/boxscroller.h"
#include "Timeline/durationrectangle.h"
#include "GUI/global.h"
#include "pointhelpers.h"
#include "canvaswindow.h"
#include "GUI/durationrectsettingsdialog.h"
#include <QApplication>
#include "clipboardcontainer.h"
#include "Animators/qrealpoint.h"

KeysView::KeysView(BoxScroller *boxesListVisible,
                   QWidget *parent) :
    QWidget(parent) {
    mBoxesListVisible = boxesListVisible;
    mBoxesListVisible->setKeysView(this);

    setMouseTracking(true);
    mScrollTimer = new QTimer(this);
}

void KeysView::setCurrentScene(Canvas * const scene) {
    mCurrentScene = scene;
}

void KeysView::setGraphViewed(const bool bT) {
    mGraphViewed = bT;
    if(bT) {
        graphResetValueScaleAndMinShown();
        mValueInput.setForce1D(false);
    } else {
        mValueInput.setForce1D(true);
    }
    update();
}

void KeysView::middlePress(const QPointF &pressPos) {
    mSavedMinViewedFrame = mMinViewedFrame;
    mSavedMaxViewedFrame = mMaxViewedFrame;
    mMiddlePressPos = pressPos;
}

void KeysView::setViewedVerticalRange(const int top,
                                      const int bottom) {
    mViewedTop = top;
    mViewedBottom = bottom;
    update();
}

void KeysView::middleMove(const QPointF &movePos) {
    const qreal diffFrame = (movePos.x() - mMiddlePressPos.x())/
                                    mPixelsPerFrame;
    const int roundX = qRound(diffFrame);
    setFramesRange({mSavedMinViewedFrame - roundX,
                    mSavedMaxViewedFrame - roundX});
}

void KeysView::deleteSelectedKeys() {
    if(mHoveredKey && mHoveredKey->isSelected()) {
        clearHoveredKey();
    }
    for(const auto& anim : mSelectedKeysAnimators)
        anim->anim_deleteSelectedKeys();
}

void KeysView::selectKeysInSelectionRect() {
    if(mGraphViewed) {
        QList<GraphKey*> keysList;
        for(const auto& anim : mGraphAnimators) {
            anim->gAddKeysInRectToList(mSelectionRect, keysList);
        }
        for(const auto& key : keysList) {
            addKeyToSelection(key);
        }
    } else {
        QList<Key*> listKeys;
        mBoxesListVisible->getKeysInRect(mSelectionRect.translated(-0.5, 0),
                                         mPixelsPerFrame,
                                         listKeys);
        for(const auto& key : listKeys) {
            addKeyToSelection(key);
        }
    }
}

void KeysView::resizeEvent(QResizeEvent *e) {
    updatePixelsPerFrame();
    if(mGraphViewed) graphResizeEvent(e);
}

void KeysView::wheelEvent(QWheelEvent *e) {
    if(mGraphViewed) {
        graphWheelEvent(e);
    } else {
        emit wheelEventSignal(e);
        if(mSelecting) {
            const QPointF posU = mapFromGlobal(QCursor::pos()) +
                           QPointF(-MIN_WIDGET_DIM/2, 0.);
            mSelectionRect.setBottom(posU.y() + mViewedTop);
        }
        //mBoxesList->handleWheelEvent(e);
    }
}

void KeysView::mousePressEvent(QMouseEvent *e) {
    KFT_setFocus();
    const QPoint posU = e->pos() + QPoint(-MIN_WIDGET_DIM/2, 0);
    if(e->button() == Qt::MiddleButton) {
        if(mGraphViewed) gMiddlePress(posU);
        else  middlePress(posU);
    } else if(e->button() == Qt::LeftButton) {
        if(mIsMouseGrabbing) return;
        mFirstMove = true;
        mLastPressPos = posU;
        if(mGraphViewed) graphMousePress(posU);
        else {
            mLastPressedKey = mBoxesListVisible->getKeyAtPos(
                                                      posU.x(), posU.y(),
                                                      mPixelsPerFrame,
                                                      mMinViewedFrame);
            if(!mLastPressedKey) {
                mLastPressedMovable =
                        mBoxesListVisible->getRectangleMovableAtPos(
                                            posU.x(), posU.y(),
                                            mPixelsPerFrame,
                                            mMinViewedFrame);
                if(!mLastPressedMovable) {
                    mSelecting = true;
                    const qreal posUXFrame = posU.x()/mPixelsPerFrame + mMinViewedFrame;
                    const QPointF xFramePos(posUXFrame,
                                            posU.y() + mViewedTop);
                    mSelectionRect.setTopLeft(xFramePos);
                    mSelectionRect.setBottomRight(xFramePos);
                } else {
                    mLastPressedMovable->pressed(e->modifiers() & Qt::SHIFT);
                    mMovingRect = true;
                }
            } else {
                const bool shiftPressed = e->modifiers() & Qt::SHIFT;
                if(!shiftPressed && !mLastPressedKey->isSelected()) {
                    clearKeySelection();
                }
                if(shiftPressed && mLastPressedKey->isSelected()) {
                    removeKeyFromSelection(mLastPressedKey);
                } else {
                    clearHoveredKey();
                    addKeyToSelection(mLastPressedKey);

                    mMovingKeys = true;
                    mValueInput.setupMove();
                }
            }
        }
    } else {
        if(mMovingKeys) {
            if(!mFirstMove) {
                if(mGraphViewed) {
                    for(const auto& anim : mGraphAnimators) {
                        anim->graph_cancelSelectedKeysTransform();
                    }
                } else {
                    for(const auto& anim : mSelectedKeysAnimators) {
                        anim->anim_cancelSelectedKeysTransform();
                    }
                }
            }

            releaseMouseAndDontTrack();
        } else {
            auto movable = mBoxesListVisible->getRectangleMovableAtPos(
                                        posU.x(), posU.y(),
                                        mPixelsPerFrame,
                                        mMinViewedFrame);
            if(!movable) {
            } else if(movable->isDurationRect()) {
                QMenu menu;
                menu.addAction("Settings...");
                const auto selectedAction = menu.exec(e->globalPos());
                if(selectedAction) {
                    const auto durRect = static_cast<DurationRectangle*>(movable);
                    durRect->openDurationSettingsDialog(this);
                }
            }
        }
    }

    grabMouseAndTrack();
    mValueInput.clearAndDisableInput();
    Document::sInstance->actionFinished();
}

stdsptr<KeysClipboard> KeysView::getSelectedKeysClipboardContainer() {
    stdsptr<KeysClipboard> container =
            enve::make_shared<KeysClipboard>();
    for(const auto& anim : mSelectedKeysAnimators) {
        QByteArray keyData;
        QBuffer buffer(&keyData);
        buffer.open(QIODevice::WriteOnly);
        eWriteStream writeStream(&buffer);
        anim->anim_writeSelectedKeys(writeStream);
        buffer.close();
        container->addTargetAnimator(anim, keyData);
    }
    return container;
}

bool KeysView::KFT_keyPressEvent(QKeyEvent *event) {
    bool inputHandled = false;
    if(mMovingKeys) {
        if(mValueInput.handleTransormationInputKeyEvent(event->key())) {
            inputHandled = true;
        }
    }
    const QPoint posU = mapFromGlobal(QCursor::pos()) +
            QPoint(-MIN_WIDGET_DIM/2, 0);
    if(inputHandled) {
        handleMouseMove(mLastMovePos, QApplication::mouseButtons());
    } else if(graphProcessFilteredKeyEvent(event)) {
    } else if(event->modifiers() & Qt::ControlModifier &&
              event->key() == Qt::Key_V) {
        if(event->isAutoRepeat()) return false;
        const auto container = Document::sInstance->getKeysClipboard();
        if(!container) return false;
        clearKeySelection();
        container->paste(mCurrentScene->getCurrentFrame(), true,
                         [this](Key* key) { addKeyToSelection(key); });
    } else if(!mSelectedKeysAnimators.isEmpty()) {
        if(event->modifiers() & Qt::ControlModifier &&
           event->key() == Qt::Key_C) {
            if(event->isAutoRepeat()) return false;
            auto container = getSelectedKeysClipboardContainer();
            Document::sInstance->replaceClipboard(container);
        } else if(event->key() == Qt::Key_S) {
            if(!mMovingKeys) {
                mValueInput.setupScale();
                mScalingKeys = true;
                mMovingKeys = true;
                mFirstMove = true;
                mLastPressPos = posU;
                grabMouseAndTrack();
            }
        } else if(event->key() == Qt::Key_G) {
            if(!mMovingKeys) {
                mValueInput.setupMove();
                mMovingKeys = true;
                mFirstMove = true;
                mLastPressPos = posU;
                grabMouseAndTrack();
            }
        } else if(event->modifiers() & Qt::SHIFT &&
                  event->key() == Qt::Key_D) {
            auto container = getSelectedKeysClipboardContainer();
            int lowestKey = FrameRange::EMAX;
            for(const auto& anim : mSelectedKeysAnimators) {
                const int animLowest = anim->anim_getLowestAbsFrameForSelectedKey();
                if(animLowest < lowestKey) {
                    lowestKey = animLowest;
                }
            }
            clearKeySelection();
            container->paste(lowestKey, false,
                             [this](Key* key) { addKeyToSelection(key); });

            mValueInput.setupMove();
            mMovingKeys = true;
            mFirstMove = true;
            mLastPressPos = posU;
            grabMouseAndTrack();
        } else if(event->modifiers() & Qt::CTRL &&
                  event->key() == Qt::Key_D) {
            auto container = getSelectedKeysClipboardContainer();
            clearKeySelection();
            container->paste(mCurrentScene->getCurrentFrame(), true,
                             [this](Key* key) { addKeyToSelection(key); });
         } else if(event->key() == Qt::Key_Delete) {
            if(mGraphViewed) {
                graphDeletePressed();
            } else {
                deleteSelectedKeys();
            }
            update();
        } else if(event->modifiers() & Qt::CTRL &&
                  event->key() == Qt::Key_Right) {
            for(const auto& anim : mSelectedKeysAnimators) {
                anim->anim_incSelectedKeysFrame(1);
            }
        } else if(event->modifiers() & Qt::CTRL &&
                  event->key() == Qt::Key_Left) {
            for(const auto& anim : mSelectedKeysAnimators) {
                anim->anim_incSelectedKeysFrame(-1);
            }
        } else return false;
    } else return false;
    return true;
}

#include "GUI/BoxesList/boxsinglewidget.h"

void KeysView::paintEvent(QPaintEvent *) {
    QPainter p(this);

    if(mGraphViewed) p.fillRect(rect(), QColor(60, 60, 60));
    else p.fillRect(rect(), QColor(60, 60, 60));

    if(mPixelsPerFrame < 0.001) return;
    if(!mGraphViewed) {    
        int currY = MIN_WIDGET_DIM;
        p.setPen(QPen(QColor(40, 40, 40), 1));
        while(currY < height()) {
            p.drawLine(0, currY, width(), currY);
            currY += MIN_WIDGET_DIM;
        }
    }
    p.translate(MIN_WIDGET_DIM/2, 0);

    p.setPen(QPen(QColor(75, 75, 75), 1));
    qreal xT = mPixelsPerFrame*0.5;
    int iInc = 1;
    bool mult5 = true;
    while(iInc*mPixelsPerFrame < MIN_WIDGET_DIM/2) {
        if(mult5) iInc *= 5;
        else iInc *= 2;
    }
    int minFrame = mMinViewedFrame;
    int maxFrame = mMaxViewedFrame;

    minFrame += ceil((-xT)/mPixelsPerFrame);
    minFrame = minFrame - minFrame%iInc - 1;
    maxFrame += floor((width() - 40 - xT)/mPixelsPerFrame) - maxFrame%iInc;
    for(int i = minFrame; i <= maxFrame; i += iInc) {
        const qreal xTT = xT + (i - mMinViewedFrame + 1)*mPixelsPerFrame;
        p.drawLine(QPointF(xTT, 0), QPointF(xTT, height()));
    }

    if(mCurrentScene) {
        if(mCurrentScene->getCurrentFrame() <= maxFrame &&
           mCurrentScene->getCurrentFrame() >= minFrame) {
            xT = (mCurrentScene->getCurrentFrame() - mMinViewedFrame)*mPixelsPerFrame +
                    mPixelsPerFrame*0.5;
            p.setPen(QPen(Qt::darkGray, 2));
            p.drawLine(QPointF(xT, 0), QPointF(xT, height()));
        }
    }

    p.setPen(QPen(Qt::black, 1));

    if(mGraphViewed) {
        p.save();
        graphPaint(&p);
        p.restore();
    } else {
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        const qreal transDFrame = 0.5*MIN_WIDGET_DIM/mPixelsPerFrame;
        const qreal frameAtZeroX = mMinViewedFrame - transDFrame;
        const int frameAtZeroXi = qFloor(frameAtZeroX);
        p.translate((frameAtZeroXi - mMinViewedFrame)*mPixelsPerFrame, 0);
        const int maxFrame = qCeil(mMaxViewedFrame + 3*transDFrame);
        const FrameRange viewedFrameRange{frameAtZeroXi, maxFrame};
        mBoxesListVisible->drawKeys(&p, mPixelsPerFrame, viewedFrameRange);
        p.restore();
        if(mSelecting) {
            p.setPen(QPen(Qt::blue, 2, Qt::DotLine));
            p.setBrush(Qt::NoBrush);
            p.drawRect(QRectF((mSelectionRect.x() - mMinViewedFrame)*mPixelsPerFrame,
                              mSelectionRect.y() - mViewedTop,
                              mSelectionRect.width()*mPixelsPerFrame,
                              mSelectionRect.height()));
        }
    }

    p.resetTransform();
    if(mMovingKeys && mValueInput.inputEnabled())
        mValueInput.draw(&p, height() - MIN_WIDGET_DIM);
    if(hasFocus()) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(Qt::red, 4));
        p.drawRect(0, 0, width(), height());
    }

    p.end();
}

void KeysView::KFT_setFocusToWidget() {
    if(mCurrentScene) Document::sInstance->setActiveScene(mCurrentScene);
    setFocus();
    update();
}

void KeysView::KFT_clearFocus() {
    clearFocus();
    update();
}

void KeysView::updateHovered(const QPoint &posU) {
    if(mGraphViewed) {
        clearHoveredKey();
        clearHoveredMovable();

        if(mGHoveredPoint) mGHoveredPoint->setHovered(false);
        mGHoveredPoint = graphGetPointAtPos(posU);
        if(mGHoveredPoint) mGHoveredPoint->setHovered(true);
        return;
    }
    if(mHoveredKey) mHoveredKey->setHovered(false);
    mHoveredKey = mBoxesListVisible->getKeyAtPos(posU.x(), posU.y(),
                                                 mPixelsPerFrame,
                                                 mMinViewedFrame);
    if(mHoveredKey) {
        mHoveredKey->setHovered(true);
        clearHoveredMovable();
    } else {
        const auto lastMovable = mHoveredMovable;
        mHoveredMovable = mBoxesListVisible->getRectangleMovableAtPos(
                            posU.x(), posU.y(), mPixelsPerFrame,
                            mMinViewedFrame);
        if(lastMovable != mHoveredMovable) {
            if(lastMovable) lastMovable->setHovered(false);
            if(mHoveredMovable) {
                mHoveredMovable->setHovered(true);
                setCursor(mHoveredMovable->getHoverCursorShape());
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
        clearHoveredKey();
    }
}

void KeysView::clearHovered() {
    clearHoveredMovable();
    clearHoveredKey();
}

void KeysView::clearHoveredKey() {
    if(!mHoveredKey) return;
    mHoveredKey->setHovered(false);
    mHoveredKey = nullptr;
}

void KeysView::clearHoveredMovable() {
    if(!mHoveredMovable) return;
    mHoveredMovable->setHovered(false);
    mHoveredMovable = nullptr;
    setCursor(Qt::ArrowCursor);
}

void KeysView::scrollRight() {
    const int pixelInc = 2*MIN_WIDGET_DIM;
    const int inc = qMax(1, qFloor(pixelInc/mPixelsPerFrame));
    mMinViewedFrame += inc;
    mMaxViewedFrame += inc;
    emit changedViewedFrames({mMinViewedFrame, mMaxViewedFrame});
    if(mSelecting) {
        mSelectionRect.setBottomRight(mSelectionRect.bottomRight() +
                                      QPointF(inc, 0));
    } else if(mMovingKeys) {
        mLastPressPos.setX(mLastPressPos.x() - qRound(inc*mPixelsPerFrame));
        handleMouseMove(mLastMovePos, QApplication::mouseButtons());
    }
    update();
}

void KeysView::scrollLeft() {
    const int pixelInc = 2*MIN_WIDGET_DIM;
    const int inc = qMax(1, qFloor(pixelInc/mPixelsPerFrame));
    mMinViewedFrame -= inc;
    mMaxViewedFrame -= inc;
    emit changedViewedFrames({mMinViewedFrame, mMaxViewedFrame});
    if(mSelecting) {
        mSelectionRect.setBottomRight(mSelectionRect.bottomRight() -
                                      QPointF(inc, 0));
    } else if(mMovingKeys) {
        mLastPressPos.setX(mLastPressPos.x() + qRound(inc*mPixelsPerFrame));
        handleMouseMove(mLastMovePos, QApplication::mouseButtons());
    }
    update();
}

void KeysView::handleMouseMoveScroll(const QPoint &pos) {
    if(pos.x() < MIN_WIDGET_DIM/2) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, &QTimer::timeout,
                    this, &KeysView::scrollLeft);
            mScrollTimer->start(300);
        }
    } else if(pos.x() > width() - MIN_WIDGET_DIM/2) {
        if(!mScrollTimer->isActive()) {
            connect(mScrollTimer, &QTimer::timeout,
                    this, &KeysView::scrollRight);
            mScrollTimer->start(300);
        }
    } else {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
}

void KeysView::handleMouseMove(const QPoint &pos,
                               const Qt::MouseButtons &buttons) {
    const QPoint posU = pos + QPoint(-MIN_WIDGET_DIM/2, 0);
    if(buttons & Qt::MiddleButton) {
        if(mGraphViewed) graphMiddleMove(posU);
        else middleMove(posU);
        emit changedViewedFrames({mMinViewedFrame, mMaxViewedFrame});
    } else if(mIsMouseGrabbing || buttons & Qt::LeftButton) {
        handleMouseMoveScroll(pos);
        qreal dFrame;
        if(mValueInput.inputEnabled()) {
            dFrame = mValueInput.getValue();
        } else {
            const qreal dX = posU.x() - mLastPressPos.x();
            dFrame = dX/mPixelsPerFrame;
            mValueInput.setDisplayedValue(dFrame);
        }
        const bool ctrlPt = mGPressedPoint && mGPressedPoint->isCtrlPt();
        if(!ctrlPt) dFrame = round(dFrame);
        const qreal dDFrame = dFrame - mMoveDFrame;
        const int iDDFrame = qRound(dDFrame);
        mMoveDFrame = dFrame;

        if(mGraphViewed && ctrlPt) {
            if(mFirstMove) mGPressedPoint->startTransform();
            qreal value;
            qreal frame;
            graphGetValueAndFrameFromPos(posU, value, frame);
            const int dY = mLastPressPos.y() - posU.y();
            const qreal dValue = mValueInput.xOnlyMode() ? 0 : dY/mPixelsPerValUnit;
            const qreal dFrameV = mValueInput.yOnlyMode() ? 0 : dFrame;
            const QPointF saved = mGPressedPoint->getSavedFrameAndValue();
            const qreal rawFrame = saved.x() + dFrameV;
            const qreal rawValue = saved.y() + dValue;
            const qreal newFrame = qBound(mMinMoveFrame, rawFrame, mMaxMoveFrame);
            const qreal newValue = qBound(mMinMoveVal, rawValue, mMaxMoveVal);
            mGPressedPoint->setFrameAndValue(newFrame, newValue);
        } else if(mMovingKeys) {
            if(mFirstMove) {
                if(mGraphViewed) {
                    for(const auto& anim : mGraphAnimators) {
                        anim->graph_startSelectedKeysTransform();
                    }
                } else {
                    for(const auto& anim : mSelectedKeysAnimators) {
                        anim->anim_startSelectedKeysTransform();
                    }
                }
            }
            if(mScalingKeys) {
                qreal keysScale;
                if(mValueInput.inputEnabled()) {
                    keysScale = mValueInput.getValue();
                } else {
                    keysScale = 1 + (posU.x() - mLastPressPos.x())/150.;
                    mValueInput.setDisplayedValue(keysScale);
                }
                const int absFrame = mCurrentScene->getCurrentFrame();
                if(mGraphViewed) {
                    for(const auto& anim : mGraphAnimators) {
                        anim->anim_scaleSelectedKeysFrame(absFrame, keysScale);
                    }
                } else {
                    for(const auto& anim : mSelectedKeysAnimators) {
                        anim->anim_scaleSelectedKeysFrame(absFrame, keysScale);
                    }
                }
            } else {
                if(mGraphViewed) {
                    const int dY = mLastPressPos.y() - posU.y();
                    const qreal dValue = mValueInput.xOnlyMode() ? 0 :
                                dY/mPixelsPerValUnit;
                    const qreal dFrameV = mValueInput.yOnlyMode() ? 0 : dFrame;
                    for(const auto& anim : mGraphAnimators) {
                        anim->graph_changeSelectedKeysFrameAndValue(
                                        {dFrameV, dValue});
                    }
                } else if(iDDFrame != 0) {
                    for(const auto& anim : mSelectedKeysAnimators) {
                        anim->anim_incSelectedKeysFrame(iDDFrame);
                    }
                }
            }
        } else if(mMovingRect) {
            if(mFirstMove) {
                if(mLastPressedMovable) {
                    const auto childProp = mLastPressedMovable->getParentProperty();
                    const bool shiftPressed = QApplication::keyboardModifiers() & Qt::SHIFT;
                    mMoveAllSelected = shiftPressed &&
                            (childProp->SWT_isBoundingBox() ||
                             childProp->SWT_isSingleSound());
                    if(mMoveAllSelected) {
                        if(mLastPressedMovable->isDurationRect()) {
                            mCurrentScene->startDurationRectPosTransformForAllSelected();
                        } else if(mLastPressedMovable->isMaxFrame()) {
                            mCurrentScene->startMaxFramePosTransformForAllSelected();
                        } else if(mLastPressedMovable->isMinFrame()) {
                            mCurrentScene->startMinFramePosTransformForAllSelected();
                        }
                        const auto ebs = static_cast<eBoxOrSound*>(childProp);
                        if(!ebs->isSelected()) mLastPressedMovable->startPosTransform();
                    } else {
                        mLastPressedMovable->startPosTransform();
                    }
                }
            }

            if(iDDFrame != 0) {
                if(mLastPressedMovable) {
                    const auto childProp = mLastPressedMovable->getParentProperty();
                    if(mMoveAllSelected) {
                        if(mLastPressedMovable->isDurationRect()) {
                            mCurrentScene->moveDurationRectForAllSelected(iDDFrame);
                        } else if(mLastPressedMovable->isMaxFrame()) {
                            mCurrentScene->moveMaxFrameForAllSelected(iDDFrame);
                        } else if(mLastPressedMovable->isMinFrame()) {
                            mCurrentScene->moveMinFrameForAllSelected(iDDFrame);
                        }
                        const auto ebs = static_cast<eBoxOrSound*>(childProp);
                        if(!ebs->isSelected()) mLastPressedMovable->changeFramePosBy(iDDFrame);
                    } else {
                        mLastPressedMovable->changeFramePosBy(iDDFrame);
                    }
                }
//                        mLastPressedDurationRectangleMovable->changeFramePosBy(
//                                    dDFrame);
            }
        } else if(mSelecting) {
            if(mGraphViewed) {
                qreal value;
                qreal frame;
                graphGetValueAndFrameFromPos(posU, value, frame);
                mSelectionRect.setBottomRight({frame, value});
            } else {
                const qreal posUXFrame = posU.x()/mPixelsPerFrame + mMinViewedFrame;
                const QPointF br(posUXFrame, posU.y() + mViewedTop);
                mSelectionRect.setBottomRight(br);
            }
        }
        mFirstMove = false;
    } else updateHovered(posU);

    if(mIsMouseGrabbing) Document::sInstance->actionFinished();
    else update();
    mLastMovePos = pos;
}

void KeysView::mouseMoveEvent(QMouseEvent *event) {
    handleMouseMove(event->pos(), event->buttons());
}

void KeysView::mouseReleaseEvent(QMouseEvent *e) {
    if(mScrollTimer->isActive()) {
        mScrollTimer->disconnect();
        mScrollTimer->stop();
    }
    if(mGraphViewed && mGPressedPoint) {
        if(mGPressedPoint->isKeyPt()) {
            if(mFirstMove) {
                if(!(e->modifiers() & Qt::SHIFT)) {
                    clearKeySelection();
                    addKeyToSelection(mGPressedPoint->getParentKey());
                }
            } else {
                for(const auto& anim : mGraphAnimators) {
                    if(!anim->anim_hasSelectedKeys()) continue;
                    anim->graph_finishSelectedKeysTransform();
                }
            }
        }
        mGPressedPoint->setSelected(false);
        mGPressedPoint = nullptr;

        graphConstrainAnimatorCtrlsFrameValues();
        //graphConstrainAnimatorCtrlsFrameValues();

        // needed ?
        graphUpdateDimensions();
    } else {
        if(e->button() == Qt::LeftButton) {
            if(mSelecting) {
                if(mSelectionRect.left() > mSelectionRect.right()) {
                    const qreal rightT = mSelectionRect.right();
                    const qreal leftT = mSelectionRect.left();
                    mSelectionRect.setLeft(rightT);
                    mSelectionRect.setRight(leftT);
                }
                if(mSelectionRect.top() > mSelectionRect.bottom()) {
                    const qreal bottomT = mSelectionRect.bottom();
                    const qreal topT = mSelectionRect.top();
                    mSelectionRect.setTop(bottomT);
                    mSelectionRect.setBottom(topT);
                }
                if(!(e->modifiers() & Qt::SHIFT)) {
                    clearKeySelection();
                }
                selectKeysInSelectionRect();
            } else if(mMovingKeys) {
                if(mFirstMove && mLastPressedKey) {
                    if(!(QApplication::keyboardModifiers() & Qt::ShiftModifier)) {
                        clearKeySelection();
                        addKeyToSelection(mLastPressedKey);
                    }
                }
                for(const auto& anim : mSelectedKeysAnimators) {
                    anim->anim_finishSelectedKeysTransform();
                }
            } else if(mMovingRect) {
                if(mFirstMove) {
                } else {
                    const auto childProp = mLastPressedMovable->getParentProperty();
                    if(mMoveAllSelected) {
                        if(mLastPressedMovable->isDurationRect()) {
                            mCurrentScene->finishDurationRectPosTransformForAllSelected();
                        } else if(mLastPressedMovable->isMinFrame()) {
                            mCurrentScene->finishMinFramePosTransformForAllSelected();
                        } else if(mLastPressedMovable->isMaxFrame()) {
                            mCurrentScene->finishMaxFramePosTransformForAllSelected();
                        }
                        const auto ebs = static_cast<eBoxOrSound*>(childProp);
                        if(!ebs->isSelected()) mLastPressedMovable->finishPosTransform();
                    } else {
                        mLastPressedMovable->finishPosTransform();
                    }
                }
            }
        }
    }
    updateHovered(e->pos() + QPoint(-MIN_WIDGET_DIM/2, 0));
    releaseMouseAndDontTrack();

    mValueInput.clearAndDisableInput();
    Document::sInstance->actionFinished();
}

void KeysView::setFramesRange(const FrameRange& range) {
    mMinViewedFrame = range.fMin;
    mMaxViewedFrame = range.fMax;
    updatePixelsPerFrame();
    if(mGraphViewed) graphUpdateDimensions();
    update();
}

int KeysView::getMinViewedFrame() {
    return mMinViewedFrame;
}

int KeysView::getMaxViewedFrame() {
    return mMaxViewedFrame;
}

qreal KeysView::getPixelsPerFrame() {
    return mPixelsPerFrame;
}

void KeysView::updatePixelsPerFrame() {
    const qreal animWidth = width() - 2*MIN_WIDGET_DIM;
    const qreal dFrame = mMaxViewedFrame - mMinViewedFrame + 1;
    mPixelsPerFrame = animWidth/dFrame;
}

void KeysView::addKeyToSelection(Key * const key) {
    if(!key) return;
    QList<Animator*> toSelect;
    key->addToSelection(toSelect);
    for(const auto& anim : toSelect) {
        connect(anim, &QObject::destroyed, this, [this, anim]() {
            mSelectedKeysAnimators.removeOne(anim);
        });
        mSelectedKeysAnimators << anim;
    }
}

void KeysView::removeKeyFromSelection(Key * const key) {
    QList<Animator*> toRemove;
    key->removeFromSelection(toRemove);
    for(const auto& anim : toRemove) {
        disconnect(anim, &QObject::destroyed, this, nullptr);
        mSelectedKeysAnimators.removeOne(anim);
    }
}

void KeysView::clearKeySelection() {
    for(const auto& anim : mSelectedKeysAnimators) {
        anim->anim_deselectAllKeys();
    }
    mSelectedKeysAnimators.clear();
}
