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

#ifndef COMPLEXANIMATOR_H
#define COMPLEXANIMATOR_H
#include "animator.h"
#include "key.h"

class ComplexKey;
class KeysClipboard;
class QrealAnimator;

class ComplexAnimator : public Animator {
    e_OBJECT
    Q_OBJECT
protected:
    ComplexAnimator(const QString& name);
public:
    virtual void ca_childIsRecordingChanged();
    virtual void ca_removeAllChildren();

    bool SWT_isComplexAnimator() const
    { return true; }

    void SWT_setupAbstraction(SWT_Abstraction *abstraction,
                              const UpdateFuncs &updateFuncs,
                              const int visiblePartWidgetId);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool parentSatisfies,
                             const bool parentMainTarget) const;


    void SWT_setChildrenAncestorDisabled(const bool bT);

    void anim_setAbsFrame(const int frame);

    void prp_startTransform();
    void prp_finishTransform();
    void prp_cancelTransform();

    QString prp_getValueText();

    void prp_afterFrameShiftChanged(const FrameRange& oldAbsRange,
                                    const FrameRange& newAbsRange);

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;

    stdsptr<Key> anim_createKey() final
    { return nullptr; }

    bool anim_isDescendantRecording() const;
    void anim_addKeyAtRelFrame(const int relFrame);
    void anim_setRecording(const bool rec);
    void anim_shiftAllKeys(const int shift);

    void ca_swapChildren(Property * const child1,
                         Property * const child2);
    void ca_moveChildInList(Property *child, const int to);
    void ca_moveChildInList(Property *child, const int from, const int to);
    void ca_moveChildBelow(Property *move, Property *below);
    void ca_moveChildAbove(Property *move, Property *above);

    bool ca_hasChildren() const;

    int ca_getNumberOfChildren() const;

    template <typename T = Property>
    T *ca_getChildAt(const int i) const {
        if(i < 0 || i >= ca_getNumberOfChildren())
            RuntimeThrow("Index outside of range");
        return static_cast<T*>(ca_mChildren.at(i).data());
    }

    int ca_getChildPropertyIndex(Property * const child);

    void ca_updateDescendatKeyFrame(Key* key);

    template <class T = Property>
    T *ca_getFirstDescendant(const std::function<bool(T*)>& tester) const {
        for(const auto &prop : ca_mChildren) {
            const auto target = dynamic_cast<T*>(prop.data());
            if(target && tester(target)) return target;
            else if(prop->SWT_isComplexAnimator()) {
                const auto ca = static_cast<ComplexAnimator*>(prop.get());
                const auto desc = ca->ca_getFirstDescendant<T>(tester);
                if(desc) return desc;
            }
        }
        return nullptr;
    }

    template <class T = Property>
    T *ca_getFirstDescendant() const {
        return ca_getFirstDescendant<T>([](T* const prop) {
            Q_UNUSED(prop)
            return true;
        });
    }

    template <class T = Property>
    T *ca_getFirstDescendantWithName(const QString &name) {
        return ca_getFirstDescendant<T>([name](T* const prop) {
            return prop->prp_getName() == name;
        });
    }

    void ca_execOnDescendants(const std::function<void(Property*)>& op) const;

    Property* ca_getGUIProperty() const
    { return ca_mGUIProperty; }

    void ca_setGUIProperty(Property * const prop)
    { ca_mGUIProperty = prop; }

    void ca_addDescendantsKey(Key * const key);
    void ca_removeDescendantsKey(Key * const key);
signals:
    void ca_childAdded(Property*);
    void ca_childRemoved(Property*);
    void ca_childMoved(Property*);
protected:
    void ca_addChild(const qsptr<Property> &child);
    void ca_insertChild(const qsptr<Property> &child, const int id);
    void ca_removeChild(const qsptr<Property> child);

    template <typename T = Property>
    qsptr<T> ca_takeChildAt(const int i) {
        const auto result = ca_mChildren.at(i);
        ca_removeChild(result);
        return result->ref<T>();
    }

    void ca_prependChild(Property *child, const qsptr<Property>& prependWith);
    void ca_replaceChild(const qsptr<Property> &child,
                         const qsptr<Property>& replaceWith);


    void ca_setHiddenWhenEmpty();
    void ca_setDisabledWhenEmpty(const bool disabled);
    const QList<qsptr<Property>>& ca_getChildren() const
    { return ca_mChildren; }
private:
    bool ca_mDisabledEmpty = true;
    bool ca_mHiddenEmpty = false;
    bool ca_mChildRecording = false;
    qptr<Property> ca_mGUIProperty;
    QList<qsptr<Property>> ca_mChildren;
};

#endif // COMPLEXANIMATOR_H
