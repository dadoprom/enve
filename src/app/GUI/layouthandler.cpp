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

#include "layouthandler.h"
#include "Private/document.h"
#include <QPushButton>

LayoutHandler::LayoutHandler(Document& document,
                             AudioHandler& audioHandler) :
    mDocument(document), mAudioHandler(audioHandler) {
    const auto canvasComboLayout = new QHBoxLayout;
    canvasComboLayout->setSpacing(0);
    canvasComboLayout->setContentsMargins(0, 0, 0, 0);

    mComboWidget = new QWidget;
    mComboWidget->setContentsMargins(0, 0, 0, 0);
    mComboWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mComboWidget->setLayout(canvasComboLayout);
    mComboWidget->setObjectName("invisWid");
    mComboWidget->setStyleSheet("QWidget#invisWid { background: transparent; }");

    mComboBox = new QComboBox;
    mComboBox->setMinimumContentsLength(20);
    mComboBox->setEditable(true);
    mComboBox->setObjectName("currentLayoutComboBox");
    mComboBox->setLayoutDirection(Qt::RightToLeft);
    mComboBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    const auto newLayPush = new QPushButton("+", mComboWidget);
    newLayPush->setObjectName("addCanvasButton");
    newLayPush->setFixedWidth(MIN_WIDGET_DIM);
    newLayPush->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    const auto removeLayPush = new QPushButton("x", mComboWidget);
    removeLayPush->setObjectName("removeCanvasButton");
    removeLayPush->setFixedWidth(MIN_WIDGET_DIM);
    removeLayPush->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    canvasComboLayout->addWidget(mComboBox);
    canvasComboLayout->addWidget(newLayPush);
    canvasComboLayout->addWidget(removeLayPush);

    mSceneLayout = new QStackedWidget();
    mTimelineLayout = new QStackedWidget();

    connect(mComboBox, qOverload<int>(&QComboBox::activated),
            this, &LayoutHandler::setCurrent);
    connect(mComboBox->lineEdit(), &QLineEdit::textEdited,
            this, &LayoutHandler::renameCurrent);

    connect(newLayPush, &QPushButton::pressed,
            this, &LayoutHandler::newLayoutAction);
    connect(removeLayPush, &QPushButton::pressed,
            this, &LayoutHandler::removeCurrent);

    connect(&document, &Document::sceneCreated,
            this, &LayoutHandler::newForSceneAction);
    connect(&document, qOverload<Canvas*>(&Document::sceneRemoved),
            this, &LayoutHandler::removeForScene);
}

void LayoutHandler::clear() {
    for(int i = 0; i < mComboBox->count(); i++)
        removeAt(0);
}
