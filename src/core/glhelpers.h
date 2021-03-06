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

#ifndef GLHELPERS_H
#define GLHELPERS_H
#include <QOpenGLFunctions_3_3_Core>
#include "skia/skiaincludes.h"
#include "exceptions.h"

typedef QOpenGLFunctions_3_3_Core QGL33;
#define BUFFER_OFFSET(i) ((void*)(i))
#define checkGlErrors \
    { \
        const GLenum glError = glGetError(); \
        if(glError != GL_NO_ERROR) \
            RuntimeThrow("OpenGL error " + std::to_string(glError)); \
    }

//! @brief Creates a program, compiles, and attaches associated shaders.
extern void iniProgram(QGL33 * const gl,
                       GLuint& program,
                       const QString &vShaderPath,
                       const QString &fShaderPath);

extern void iniTexturedVShaderVBO(QGL33 * const gl);
extern void iniTexturedVShaderVAO(QGL33 * const gl, GLuint& VAO);
extern void iniPlainVShaderVBO(QGL33 * const gl);
extern void iniPlainVShaderVAO(QGL33 * const gl, GLuint& VAO);

extern QString GL_PLAIN_VERT;
extern GLuint GL_PLAIN_SQUARE_VBO;

extern QString GL_TEXTURED_VERT;
extern GLuint GL_TEXTURED_SQUARE_VBO;

enum class HardwareSupport : short {
    cpuOnly,
    cpuPreffered,
    gpuPreffered,
    gpuOnly
};

enum class Hardware : short {
    cpu, gpu, hdd
};

struct CpuRenderData {
    //! @brief Tile rect in texture coordinates
    SkIRect fTexTile;

    //! @brief Pixel {0, 0} position in scene coordinates
    int fPosX;
    int fPosY;

    //! @brief Texture size
    uint fWidth;
    uint fHeight;
};

#include <QJSEngine>
struct GpuRenderData : public CpuRenderData {
    //! @brief Used for shader based effects
    QJSEngine fJSEngine;
};

#endif // GLHELPERS_H
