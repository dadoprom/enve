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

#include "envesplash.h"

#include <QPainter>

#include "Private/esettings.h"

EnveSplash::EnveSplash() {
    mText = "Welcome to enve - open-source 2D animation software.\n\n"
            "Further development will only be possible with your support. "
            "Press 'Support enve' on the menu bar for more information.\n\n"
            "Please note that major version zero 0.x.y is for initial development; "
            "hence numerous bugs should be expected.\n\n"
            "Remember - enve is not a video editor.";
    const auto splashPath = eSettings::sIconsDir() + "/splash.png";
    const QPixmap pixmap(splashPath);
    mTextRect = QRect(qRound(0.03*pixmap.width()), qRound(0.20*pixmap.height()),
                      qRound(0.94*pixmap.width()), qRound(0.6*pixmap.height()));
    mMessageRect = QRect(mTextRect.x(), qRound(0.84*pixmap.height()),
                         qRound(0.94*pixmap.width()), qRound(0.09*pixmap.height()));
    mBottomRect = QRect(mTextRect.x(), qRound(0.93*pixmap.height()),
                        qRound(0.94*pixmap.width()), qRound(0.07*pixmap.height()));
    setPixmap(pixmap);
    setFixedSize(pixmap.width(), pixmap.height());
//    setWindowFlag(Qt::WindowStaysOnTopHint);
}

void EnveSplash::drawContents(QPainter * const p) {
    p->setPen(QColor(125, 125, 125));
    p->drawRect(mTextRect);
    p->drawRect(mMessageRect);

    p->setPen(Qt::white);

    QFont font = p->font();
    font.setPointSizeF(font.pointSizeF()*1.5);
    font.setFamily("FreeMono");
    p->setFont(font);
    const int w = width();
    const int marg = w/80;
    p->drawText(mTextRect.adjusted(marg, marg, -marg, -marg),
                Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, mText);
    p->drawText(mMessageRect, Qt::AlignCenter, message());
    p->setPen(QColor(Qt::gray));
    p->drawText(mBottomRect, Qt::AlignVCenter | Qt::AlignLeft, "Maurycy Liebner");
    QString rightTxt;
#if defined(LATEST_COMMIT_HASH) && defined(LATEST_COMMIT_DATE)
    const QString date(LATEST_COMMIT_DATE);
    rightTxt = QString(LATEST_COMMIT_HASH) + " " + date.split(" ").first();
#else
    rightTxt = "0.0.0";
#endif
    p->drawText(mBottomRect, Qt::AlignVCenter | Qt::AlignRight, rightTxt);
}

void EnveSplash::mousePressEvent(QMouseEvent *) {
    if(parent()) deleteLater();
}
