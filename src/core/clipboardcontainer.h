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

#ifndef CLIPBOARDCONTAINER_H
#define CLIPBOARDCONTAINER_H
#include <QList>
#include <QWeakPointer>
#include <typeindex>
#include "smartPointers/ememory.h"
#include "ReadWrite/basicreadwrite.h"
#include "Animators/SmartPath/smartpath.h"
class QrealAnimator;
class BoundingBox;
class ContainerBox;
class Key;
class Animator;
class Property;

typedef QPair<qptr<Animator>, QByteArray> AnimatorKeyDataPair;

enum class ClipboardType : short {
    misc,
    boxes,
    keys,
    property,
    dynamicProperties,
    smartPath
};

class Clipboard : public StdSelfRef {
public:
    Clipboard(const ClipboardType type);
    ClipboardType getType() const;
protected:
    QByteArray mData;
private:
    const ClipboardType mType;
};

class SmartPathClipboard : public Clipboard {
public:
    SmartPathClipboard(const SmartPath& path) :
        Clipboard(ClipboardType::smartPath), mPath(path) {}

    const SmartPath& path() const { return mPath; }
private:
    SmartPath mPath;
};

class BoxesClipboard : public Clipboard {
    e_OBJECT
protected:
    BoxesClipboard(const QList<BoundingBox*> &src);
public:
    void pasteTo(ContainerBox * const parent);
};

class KeysClipboard : public Clipboard {
    e_OBJECT
protected:
    KeysClipboard();
public:
    void paste(const int pasteFrame, const bool merge,
               const std::function<void(Key*)> &selectAction = nullptr);

    void addTargetAnimator(Animator *anim, const QByteArray& keyData);
private:
    QList<AnimatorKeyDataPair> mAnimatorData;
};


class PropertyClipboard : public Clipboard {
    e_OBJECT
protected:
    PropertyClipboard(const Property * const source);
public:
    bool paste(Property * const target);

    template<typename T>
    bool hasType() {
        return mContentType == std::type_index(typeid(T));
    }

    template<typename T>
    bool compatibleTarget(T* const obj) const {
        return mContentType == std::type_index(typeid(*obj));
    }
private:
    const std::type_index mContentType;
};

template <typename T>
class DynamicComplexAnimatorBase;

class DynamicPropsClipboard : public Clipboard {
    e_OBJECT
protected:
    template<typename T>
    DynamicPropsClipboard(const QList<T*>& source) :
        Clipboard(ClipboardType::dynamicProperties),
        mContentBaseType(std::type_index(typeid(T))) {
        QBuffer buffer(&mData);
        buffer.open(QIODevice::WriteOnly);
        eWriteStream dst(&buffer);
        dst << source.count();
        for(const auto& src : source)
            src->prp_writeProperty(dst);
        buffer.close();
    }
public:
    template<typename T>
    bool paste(DynamicComplexAnimatorBase<T> * const target) {
        if(!compatibleTarget(target)) return false;
        QBuffer buffer(&mData);
        buffer.open(QIODevice::ReadOnly);
        eReadStream src(&buffer);
        target->prp_readProperty(src);
        buffer.close();
        return true;
    }

    template<typename T>
    bool compatibleTarget(DynamicComplexAnimatorBase<T> * const obj) {
        Q_UNUSED(obj)
        return mContentBaseType == std::type_index(typeid(T));
    }
private:
    const std::type_index mContentBaseType;
};

#endif // CLIPBOARDCONTAINER_H
