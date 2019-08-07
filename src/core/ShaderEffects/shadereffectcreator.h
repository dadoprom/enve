#ifndef SHADEREFFECTCREATOR_H
#define SHADEREFFECTCREATOR_H
#include "shadereffectprogram.h"

enum PropertyType {
    PTYPE_FLOAT,
    PTYPE_INT
};

class ShaderEffect;

struct ShaderEffectCreator : public StdSelfRef {
    e_OBJECT
public:
    ShaderEffectCreator(const QString& grePath, const QString& name,
                        const QList<stdsptr<ShaderPropertyCreator>>& propCs,
                        const ShaderEffectProgram& program) :
        fName(name), fGrePath(grePath),
        fProperties(propCs), fProgram(program) {}

    struct Identifier {
        Identifier(const QString& grePath, const QString& name,
                   const QList<PropertyType>& types) :
        fGrePath(grePath), fName(name), fTypes(types) {}

        const QString fGrePath;
        const QString fName;
        const QList<PropertyType> fTypes;
    };

    const QString fName;
    const QString fGrePath;
    const QList<stdsptr<ShaderPropertyCreator>> fProperties;
    ShaderEffectProgram fProgram;

    bool compatible(const QList<PropertyType>& props) const {
        if(props.count() != fProperties.count()) return false;
        for(int i = 0; i < props.count(); i++) {
            const auto& iType = props.at(i);
            const auto prop = fProperties.at(i).get();
            if(iType == PTYPE_FLOAT) {
                const bool iCompatible =
                        dynamic_cast<QrealAnimatorCreator*>(prop);
                if(!iCompatible) return false;
            } else if(iType == PTYPE_INT) {
                const bool iCompatible =
                        dynamic_cast<IntAnimatorCreator*>(prop);
                if(!iCompatible) return false;
            } else return false;
        }
        return true;
    }

    void reloadProgram(QGL33 * const gl, const QString& fragPath) {
        if(!QFile(fragPath).exists()) return;
        ShaderEffectProgram program;
        try {
            program = ShaderEffectProgram::sCreateProgram(
                        gl, fragPath,
                        fProgram.fMarginScript, fProperties,
                        fProgram.fPropUniCreators,
                        fProgram.fValueHandlers);
        } catch(...) {
            RuntimeThrow("Failed to load a new version of '" + fragPath + "'");
        }
        gl->glDeleteProgram(fProgram.fId);
        fProgram = program;
    }

    qsptr<ShaderEffect> create() const;

    void writeIdentifier(QIODevice * const dst) const {
        gWrite(dst, fName);
        gWrite(dst, fGrePath);
        const int nChildren = fProperties.count();
        dst->write(rcConstChar(&nChildren), sizeof(int));
        for(const auto& anim : fProperties) {
            PropertyType type;
            if(dynamic_cast<QrealAnimatorCreator*>(anim.get())) {
                type = PTYPE_FLOAT;
            } else if(dynamic_cast<IntAnimatorCreator*>(anim.get())) {
                type = PTYPE_INT;
            } else RuntimeThrow("Only QrealAnimator and IntAnimator supported");
            dst->write(rcConstChar(&type), sizeof(PropertyType));
        }
    }

    static Identifier sReadIdentifier(QIODevice * const src) {
        const QString name = gReadString(src);
        const QString grePath = gReadString(src);
        QList<PropertyType> props;
        int nChildren;
        src->read(rcChar(&nChildren), sizeof(int));
        for(int i = 0; i < nChildren; i++) {
            PropertyType type;
            src->read(rcChar(&type), sizeof(PropertyType));
            props << type;
        }
        return Identifier(grePath, name, props);
    }

    static stdsptr<ShaderEffectCreator> sLoadFromFile(
            QGL33 * const gl, const QString& grePath);

    static stdsptr<ShaderEffectCreator> sWithGrePath(
            const QString& grePath);

    static stdsptr<ShaderEffectCreator> sWithGrePathAndCompatible(
            const QString& grePath,
            const QList<PropertyType>& props);

    static QList<stdsptr<ShaderEffectCreator>> sWithName(
            const QString &name);

    static QList<stdsptr<ShaderEffectCreator>> sWithNameAndCompatible(
            const QString &name,
            const QList<PropertyType>& props);

    static QList<stdsptr<ShaderEffectCreator>> sWithCompatibleProps(
            const QList<PropertyType>& props);

    static QList<stdsptr<ShaderEffectCreator>> sGetBestCompatibleEffects(
            const Identifier& id);

    static QList<stdsptr<ShaderEffectCreator>> sEffectCreators;
};

#endif // SHADEREFFECTCREATOR_H
