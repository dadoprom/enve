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

#ifndef BOXTARGETWIDGET_H
#define BOXTARGETWIDGET_H

#include <QWidget>
class BoundingBox;
class BoxTargetProperty;
#include "smartPointers/ememory.h"

class BoxTargetWidget : public QWidget {
    Q_OBJECT
public:
    explicit BoxTargetWidget(QWidget *parent = nullptr);

    void setTargetProperty(BoxTargetProperty *property);
private:
    bool mDragging = false;
    qptr<BoxTargetProperty> mProperty;
signals:

public slots:
protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void paintEvent(QPaintEvent *);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
};

#endif // BOXTARGETWIDGET_H
