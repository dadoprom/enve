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

#include "filesourcelist.h"
#include "GUI/global.h"
#include <QPainter>
#include <QScrollBar>
#include <QMenu>
#include "mainwindow.h"
#include <QApplication>
#include <QDrag>
#include "FileCacheHandlers/videocachehandler.h"
#include "FileCacheHandlers/imagecachehandler.h"

FileSourceWidget::FileSourceWidget(FileSourceListVisibleWidget *parent) :
    QWidget(parent) {
    mParentVisibleWidget = parent;
    setFixedHeight(MIN_WIDGET_DIM);
}

void FileSourceWidget::setTargetCache(FileCacheHandlerAbstraction *target) {
    mTargetCache = target;
    if(!mTargetCache) {
        setToolTip("");
    } else {
        setToolTip(mTargetCache->getName());
    }
}

void FileSourceWidget::mouseMoveEvent(QMouseEvent *event) {
    if(!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    if((event->pos() - mDragStartPos).manhattanLength()
         < QApplication::startDragDistance()) {
        return;
    }
    QDrag *drag = new QDrag(this);

    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls(QList<QUrl>() <<
                      QUrl::fromLocalFile(mTargetCache->getName()));
    drag->setMimeData(mimeData);

    drag->installEventFilter(MainWindow::sGetInstance());
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}


void FileSourceWidget::paintEvent(QPaintEvent *) {
    if(!mTargetCache || width() <= 2*MIN_WIDGET_DIM) return;
    QPainter p(this);

    QString pathString = mTargetCache->getName();
    if(mFileNameOnly) pathString = pathString.split("/").last();
    const auto fm = p.fontMetrics();
    const int spaceForPath = int(width() - 1.5*MIN_WIDGET_DIM);
    pathString = fm.elidedText(pathString, Qt::ElideLeft, spaceForPath);
    const int pathWidth = fm.width(pathString);

    if(mTargetCache->selected) {
        p.fillRect(QRect(0.5*MIN_WIDGET_DIM, 0,
                         pathWidth + MIN_WIDGET_DIM, MIN_WIDGET_DIM),
                   QColor(180, 180, 180));
        p.setPen(Qt::black);
    }
    if(mTargetCache->isFileMissing()) p.setPen(Qt::red);

    p.drawText(rect().adjusted(MIN_WIDGET_DIM, 0,
                               -0.5*MIN_WIDGET_DIM, 0),
               Qt::AlignVCenter | Qt::AlignLeft,
               pathString);

    p.end();
}

void FileSourceWidget::switchFileNameOnly() {
    mFileNameOnly = !mFileNameOnly;
}

void FileSourceWidget::mousePressEvent(QMouseEvent *event) {
    mDragStartPos = event->pos();
}

void FileSourceWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(!mTargetCache) return;
    if(event->button() == Qt::LeftButton) {
        if(event->modifiers() & Qt::ShiftModifier) {
            mTargetCache->switchSelected();
        } else {
            mParentVisibleWidget->clearSelected();
            mTargetCache->setSelected(true);
        }
    } else {
        if(!mTargetCache->selected) {
            mParentVisibleWidget->clearSelected();
            mTargetCache->setSelected(true);
        }
        mParentVisibleWidget->showContextMenu(event->globalPos());
    }

    update();
}

FileSourceListScrollWidget::FileSourceListScrollWidget(ScrollArea *parent) :
    MinimalScrollWidget(parent) {
    createVisiblePartWidget();
    updateHeight();
}

void FileSourceListScrollWidget::updateHeight() {
    FileSourceListVisibleWidget *visWid =
            ((FileSourceListVisibleWidget*)mMinimalVisiblePartWidget);
    setFixedHeight((visWid->getCacheListCount() + 0.5) * MIN_WIDGET_DIM);
}

void FileSourceListScrollWidget::createVisiblePartWidget() {
    mMinimalVisiblePartWidget = new FileSourceListVisibleWidget(this);
}

FileSourceListVisibleWidget::FileSourceListVisibleWidget(MinimalScrollWidget *parent) :
    MinimalScrollWidgetVisiblePart(parent) {
    connect(FilesHandler::sInstance, &FilesHandler::addedCacheHandler,
            this, &FileSourceListVisibleWidget::addCacheHandlerToList);
    connect(FilesHandler::sInstance, &FilesHandler::removedCacheHandler,
            this, &FileSourceListVisibleWidget::removeCacheHandlerFromList);
}

void FileSourceListVisibleWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    int currY = MIN_WIDGET_DIM;
    p.setPen(QPen(QColor(40, 40, 40), 1.));
    while(currY < height()) {
        p.drawLine(0, currY, width(), currY);

        currY += MIN_WIDGET_DIM;
    }

//    if(mDragging) {
//        p.setPen(QPen(Qt::white, 3.));
//        p.drawLine(0, mCurrentDragPosId*MIN_WIDGET_HEIGHT,
//                   width(), mCurrentDragPosId*MIN_WIDGET_HEIGHT);
//    }

    p.end();
}

void FileSourceListVisibleWidget::updateVisibleWidgetsContent() {
    int firstVisibleId = mVisibleTop/MIN_WIDGET_DIM;

    for(const auto& wid : mSingleWidgets) wid->hide();

    int idP = 0;
    for(int i = firstVisibleId;
        i < mCacheList.count() &&
        idP < mSingleWidgets.count(); i++) {
        const auto fsw = static_cast<FileSourceWidget*>(mSingleWidgets.at(idP));
        fsw->setTargetCache(mCacheList.at(i).get());
        fsw->show();
        idP++;
    }
}

QWidget *FileSourceListVisibleWidget::createNewSingleWidget() {
    return new FileSourceWidget(this);
}

void FileSourceListVisibleWidget::addCacheHandlerToList(FileCacheHandler * const handler) {
    mCacheList << std::make_shared<FileCacheHandlerAbstraction>(handler, this);
    scheduleContentUpdate();
}

void FileSourceListVisibleWidget::removeCacheHandlerFromList(
        FileCacheHandler * const handler) {
    for(int i = 0; i < mCacheList.count(); i++) {
        const auto& abs = mCacheList.at(i);
        if(abs->fTarget == handler) {
            if(abs->selected) removeFromSelectedList(abs.get());
            mCacheList.removeAt(i);
            scheduleContentUpdate();
            return;
        }
    }
}

void FileSourceListVisibleWidget::showContextMenu(const QPoint &globalPos) {
    QMenu menu;
    menu.addAction("Reload");
    if(mSelectedList.count() == 1) menu.addAction("Replace...");
    const auto selected_action = menu.exec(globalPos);
    if(selected_action) {
        if(selected_action->text() == "Reload") {
            for(const auto& abs : mSelectedList) {
                abs->fTarget->reloadAction();
            }
        } else if(selected_action->text() == "Replace...") {
            mSelectedList.first()->fTarget->replace(MainWindow::sGetInstance());
        }

        Document::sInstance->actionFinished();
    } else {

    }
}

FileSourceList::FileSourceList(QWidget *parent) : ScrollArea(parent) {
    mScrollWidget = new FileSourceListScrollWidget(this);
    setWidget(mScrollWidget);

    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            mScrollWidget, &FileSourceListScrollWidget::changeVisibleTop);
    connect(this, &FileSourceList::heightChanged,
            mScrollWidget, &FileSourceListScrollWidget::changeVisibleHeight);
    connect(this, &FileSourceList::widthChanged,
            mScrollWidget, &FileSourceListScrollWidget::setWidth);

    verticalScrollBar()->setSingleStep(MIN_WIDGET_DIM);
    setAcceptDrops(true);
}

void FileSourceList::dropEvent(QDropEvent *event) {
    if(event->mimeData()->hasUrls()) {
        const QList<QUrl> urlList = event->mimeData()->urls();
        for(const QUrl &url : urlList) {
            if(url.isLocalFile()) {
                const QString urlStr = url.toLocalFile();
                const QString ext = urlStr.split(".").last();
                if(isVideoExt(ext)) {
                    //FileCacheHandler::sGetHandlerForFilePath<VideoFrameCacheHandler>(urlStr);
                } else if(isImageExt(ext)) {
                    //FileCacheHandler::sGetHandlerForFilePath<ImageCacheHandler>(urlStr);
                }
            }
        }
        event->acceptProposedAction();
    }
    Document::sInstance->actionFinished();
}

void FileSourceList::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

void FileSourceList::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void FileCacheHandlerAbstraction::setSelected(const bool bT) {
    if(bT == selected) return;
    selected = bT;
    if(selected) {
        parentVisibleWidget->addToSelectedList(this);
    } else {
        parentVisibleWidget->removeFromSelectedList(this);
    }
}
