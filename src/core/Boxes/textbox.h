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

#ifndef TEXTBOX_H
#define TEXTBOX_H
#include "Boxes/pathbox.h"
#include "skia/skiaincludes.h"
class QStringAnimator;

class TextBox : public PathBox {
public:
    TextBox();

    bool SWT_isTextBox() const { return true; }
    void setupCanvasMenu(PropertyMenu * const menu);
    SkPath getPathAtRelFrameF(const qreal relFrame);

    void writeBoundingBox(eWriteStream& dst);
    void readBoundingBox(eReadStream& src);

    bool differenceInEditPathBetweenFrames(
                const int frame1, const int frame2) const;

    void setFont(const QFont &font);
    void setSelectedFontSize(const qreal size);
    void setSelectedFontFamilyAndStyle(const QString &fontFamily,
                                       const QString &fontStyle);

    qreal getFontSize();
    QString getFontFamily();
    QString getFontStyle();
    QString getCurrentValue();

    void openTextEditor(QWidget* dialogParent);

    void setTextAlignment(const Qt::Alignment &alignment) {
        mAlignment = alignment;
        planUpdate(UpdateReason::userChange);
    }

    void setCurrentValue(const QString &text);
private:
    Qt::Alignment mAlignment = Qt::AlignLeft;
    QFont mFont;

    qsptr<QrealAnimator> mLinesDist;
    qsptr<QStringAnimator> mText;
};

#endif // TEXTBOX_H
