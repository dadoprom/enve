#ifndef CANVASWINDOW_H
#define CANVASWINDOW_H

#include <QWidget>
#include "glwindow.h"
#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"
#include "keyfocustarget.h"
class Canvas;
typedef QSharedPointer<Canvas> CanvasQSPtr;
enum ColorMode : short;
enum CanvasMode : short;
class Color;
class Gradient;
class BoundingBox;
class BoxesGroup;
class PaintControler;
class SoundComposition;
class PaintSetting;
class CanvasWidget;
class RenderInstanceSettings;
class Updatable;
class ImageBox;
class SingleSound;
class VideoBox;

#include <QAudioOutput>

class CanvasWindow : public GLWindow,
        public SingleWidgetTarget,
        public KeyFocusTarget {
    Q_OBJECT
public:
    explicit CanvasWindow(QWidget *parent);
    ~CanvasWindow();

    Canvas *getCurrentCanvas();
    const QList<CanvasQSPtr> &getCanvasList() {
        return mCanvasList;
    }

    void setCurrentCanvas(Canvas *canvas);
    void addCanvasToList(Canvas *canvas);
    void removeCanvas(const int &id);
    void addCanvasToListAndSetAsCurrent(Canvas *canvas);
    void renameCanvas(Canvas *canvas, const QString &newName);
    void renameCanvas(const int &id, const QString &newName);
    bool hasNoCanvas();
    void setCanvasMode(const CanvasMode &mode);

    void callUpdateSchedulers();
    bool KFT_handleKeyEventForTarget(QKeyEvent *event);
    void KFT_setFocusToWidget() {
        setFocus();
    }

    void KFT_clearFocus() {
        clearFocus();
    }

    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void strokeCapStyleChanged(const Qt::PenCapStyle &capStyle);
    void strokeJoinStyleChanged(const Qt::PenJoinStyle &joinStyle);
    void strokeWidthChanged(const qreal &strokeWidth,
                            const bool &finish);

    void pickPathForSettings(const bool &pickFill,
                             const bool &pickStroke);
    void updateDisplayedFillStrokeSettings();

    void setResolutionFraction(const qreal &percent);
    void updatePivotIfNeeded();
    void schedulePivotUpdate();

    BoxesGroup *getCurrentGroup();
    SingleWidgetAbstraction *SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget);
    void applyPaintSettingToSelected(const PaintSetting &setting);
    void setSelectedFillColorMode(const ColorMode &mode);
    void setSelectedStrokeColorMode(const ColorMode &mode);

    void updateAfterFrameChanged(const int &currentFrame);
    void clearAll();
    void createAnimationBoxForPaths(const QStringList &importPaths);
    void createLinkToFileWithPath(const QString &path);
    VideoBox *createVideoForPath(const QString &path);
    int getCurrentFrame();
    int getMaxFrame();
    void addUpdatableAwaitingUpdate(Updatable *updatable);
    void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction *abstraction,
            ScrollWidgetVisiblePart *visiblePartWidget);
    ImageBox *createImageForPath(const QString &path);
    SingleSound *createSoundForPath(const QString &path);
    void updateHoveredElements();

    void switchLocalPivot();
    bool getLocalPivot();
    void setLocalPivot(const bool &bT);

    void importFile(const QString &path,
                    const QPointF &relDropPos = QPointF(0., 0.));

    QWidget *getCanvasWidget();

    void grabMouse();

    bool hasFocus() {
        return mHasFocus;
    }

    void setFocus() {
        mHasFocus = true;
    }

    void clearFocus() {
        mHasFocus = false;
    }

    void repaint();

    QRect rect();

    void releaseMouse();

    bool isMouseGrabber();

    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void openSettingsWindowForCurrentCanvas();

    void rotate90CCW();
    void rotate90CW();
    void processSchedulers();
    bool noBoxesAwaitUpdate();
    void afterAllSavesFinished();
    void writeCanvases(std::fstream *file);
    void readCanvases(std::fstream *file);
protected:
    RenderInstanceSettings *mCurrentRenderSettings = NULL;
    QList<int> mFreeThreads;
    bool mMouseGrabber = false;
    bool mHasFocus = false;
    QWidget *mCanvasWidget;
    void setRendering(const bool &bT);
    void setPreviewing(const bool &bT);

    QTimer *mPreviewFPSTimer = NULL;
    QList<QThread*> mPaintControlerThreads;
    QList<PaintControler*> mPaintControlers;

    qreal mSavedResolutionFraction = 100.;
    int mSavedCurrentFrame = 0;
    bool mPreviewing = false;
    bool mRendering = false;
    bool mNoBoxesAwaitUpdate = true;
    bool mCancelLastBoxUpdate = false;
    QList<std::shared_ptr<Updatable> > mUpdatablesAwaitingUpdate;

    QString mOutputString;
    int mCurrentRenderFrame;

    bool mClearBeingUpdated = false;

    void (CanvasWindow::*mBoxesUpdateFinishedFunction)(void) = NULL;
    CanvasQSPtr mCurrentCanvas;
    QList<CanvasQSPtr> mCanvasList;

    //void paintEvent(QPaintEvent *);

    void nextCurrentRenderFrame();


    // AUDIO
    void initializeAudio();
    void startAudio();
    void stopAudio();
    void volumeChanged(int value);

    QAudioDeviceInfo mAudioDevice;
    SoundComposition *mCurrentSoundComposition;
    QAudioOutput *mAudioOutput;
    QIODevice *mAudioIOOutput; // not owned
    QAudioFormat mAudioFormat;

    QByteArray mAudioBuffer;
    // AUDIO

    void qRender(QPainter *p);
    void renderSk(SkCanvas *canvas);
    void tabletEvent(QTabletEvent *e);
signals:
    void updateUpdatable(Updatable*, int);
    void changeCurrentFrame(int);
    void changeFrameRange(int, int);
public slots:
    void setMovePathMode();
    void setMovePointMode();
    void setAddPointMode();
    void setRectangleMode();
    void setCircleMode();
    void setTextMode();
    void setParticleBoxMode();
    void setParticleEmitterMode();
    void setPaintBoxMode();
    void setPaintMode();

    void raiseAction();
    void lowerAction();
    void raiseToTopAction();
    void lowerToBottomAction();

    void objectsToPathAction();
    void strokeToPathAction();

    void setFontFamilyAndStyle(QString family, QString style);
    void setFontSize(qreal size);

    void connectPointsSlot();
    void disconnectPointsSlot();
    void mergePointsSlot();

    void makePointCtrlsSymmetric();
    void makePointCtrlsSmooth();
    void makePointCtrlsCorner();

    void makeSegmentLine();
    void makeSegmentCurve();

    void pathsUnionAction();
    void pathsDifferenceAction();
    void pathsIntersectionAction();
    void pathsDivisionAction();
    void pathsExclusionAction();

    void renameCurrentCanvas(const QString &newName);
    void setCurrentCanvas(const int &id);

    void setClipToCanvas(const bool &bT);

    void interruptPreview();
    void outOfMemory();
    void interruptRendering();

    void playPreview();
    void stopPreview();
    void pausePreview();
    void resumePreview();
    void renderPreview();
    void renderOutput();
    void renderFromSettings(RenderInstanceSettings *settings);

    void importFile();

    void startSelectedStrokeWidthTransform();

    void deleteAction();
    void copyAction();
    void pasteAction();
    void cutAction();
    void duplicateAction();
    void selectAllAction();
    void clearSelectionAction();

    void groupSelectedBoxes();
    void ungroupSelectedBoxes();
    void rotate90CWAction();
    void rotate90CCWAction();
private slots:
    void sendNextUpdatableForUpdate(const int &threadId,
                                    Updatable *lastUpdatable = NULL);
    void nextSaveOutputFrame();
    void nextPreviewRenderFrame();
    void saveOutput(const QString &renderDest,
                    const qreal &resolutionFraction);

    void pushTimerExpired();
};

#endif // CANVASWINDOW_H