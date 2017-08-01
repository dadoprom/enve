#include "Boxes/imagebox.h"
#include <QFileDialog>
#include "mainwindow.h"
#include "filesourcescache.h"

ImageBox::ImageBox() :
    BoundingBox(TYPE_IMAGE) {
    setName("Image");
}

ImageBox::ImageBox(const QString &filePath) :
    ImageBox() {
    setFilePath(filePath);
}

ImageBox::~ImageBox() {
    if(mImgCacheHandler != NULL) {
        mImgCacheHandler->removeDependentBox(this);
    }
}

#include <QSqlError>
int ImageBox::saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = BoundingBox::saveToSql(query, parentId);

    if(!query->exec(QString("INSERT INTO imagebox (boundingboxid, "
                           "imagefilepath) "
                "VALUES (%1, '%2')").
                    arg(boundingBoxId).
                    arg(mImageFilePath) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return boundingBoxId;
}


void ImageBox::loadFromSql(const int &boundingBoxId) {
    BoundingBox::loadFromSql(boundingBoxId);

    QSqlQuery query;
    QString queryStr = "SELECT * FROM imagebox WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int ImageFilePathId = query.record().indexOf("imagefilepath");

        setFilePath(query.value(ImageFilePathId).toString());
    } else {
        qDebug() << "Could not load imagebox with id " << boundingBoxId;
    }
}


void ImageBox::makeDuplicate(Property *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    ImageBox *imgTarget = (ImageBox*)targetBox;
    imgTarget->setFilePath(mImageFilePath);
}

BoundingBox *ImageBox::createNewDuplicate() {
    return new ImageBox();
}

void ImageBox::setFilePath(const QString &path) {
    mImageFilePath = path;
    if(mImgCacheHandler != NULL) {
        mImgCacheHandler->removeDependentBox(this);
    }
    mImgCacheHandler = (ImageCacheHandler*)
                                FileSourcesCache::getHandlerForFilePath(
                                                        path);
    if(mImgCacheHandler == NULL) {
        QFile file(path);
        if(file.exists()) {
            mImgCacheHandler = new ImageCacheHandler(path);
        }
    }
    mImgCacheHandler->addDependentBox(this);
    prp_updateInfluenceRangeAfterChanged();
}

void ImageBox::addActionsToMenu(QMenu *menu) {
    menu->addAction("Reload")->setObjectName("ib_reload");
    menu->addAction("Set Source File...")->
            setObjectName("ib_set_src_file");
}

void ImageBox::changeSourceFile() {
    MainWindow::getInstance()->disableEventFilter();
    QString importPath = QFileDialog::getOpenFileName(
                                            MainWindow::getInstance(),
                                            "Change Source", "",
                                            "Image Files (*.png *.jpg)");
    MainWindow::getInstance()->enableEventFilter();
    if(!importPath.isEmpty()) {
        setFilePath(importPath);
    }
}

void ImageBox::setupBoundingBoxRenderDataForRelFrame(
                                    const int &relFrame,
                                    BoundingBoxRenderData *data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    ImageBoxRenderData *imgData = (ImageBoxRenderData*)data;
    imgData->image = mImgCacheHandler->getImage();
    if(imgData->image == NULL) {
        mImgCacheHandler->addScheduler();
        mImgCacheHandler->addDependent(imgData);
    }
}

BoundingBoxRenderData *ImageBox::createRenderData() {
    return new ImageBoxRenderData(mImgCacheHandler, this);
}

bool ImageBox::handleSelectedCanvasAction(QAction *selectedAction) {
    if(selectedAction->objectName() == "ib_set_src_file") {
        changeSourceFile();
    } else if(selectedAction->objectName() == "ib_reload") {
        if(mImgCacheHandler != NULL) {
            mImgCacheHandler->clearCache();
        }
    } else {
        return false;
    }
    return true;
}
#include "filesourcescache.h"
void ImageBoxRenderData::loadImageFromHandler() {
    image = ((ImageCacheHandler*)srcCacheHandler)->getImage();
}
