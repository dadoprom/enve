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

#ifndef IMPORTHANDLER_H
#define IMPORTHANDLER_H
#include <QFileInfo>
#include <memory>
#include "Boxes/boundingbox.h"

class eImporter {
public:
    virtual ~eImporter();

    virtual bool supports(const QFileInfo& fileInfo) const = 0;
    //! @brief Can return nullptr.
    virtual qsptr<BoundingBox> import(const QFileInfo& fileInfo) const = 0;
private:
};

class ImportHandler {
public:
    ImportHandler();

    static ImportHandler* sInstance;

    template<typename T>
    void addImporter() {
        mImporters << std::make_shared<T>();
    }

    qsptr<BoundingBox> import(const QString& path) const;
private:
    QList<std::shared_ptr<eImporter>> mImporters;
};

#endif // IMPORTHANDLER_H
