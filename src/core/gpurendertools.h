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

#ifndef GPURENDERTOOLS_H
#define GPURENDERTOOLS_H
#include "glhelpers.h"
#include "switchablecontext.h"
#include "etextureframebuffer.h"

class GpuRenderTools {
public:
    GpuRenderTools(QGL33* const gl,
                   SwitchableContext& context,
                   sk_sp<SkImage> img);

    ~GpuRenderTools();

    GLuint getSquareVAO() const;

    //! @brief Swaps the source and the target texture if valid.
    void swapTextures();

    void switchToOpenGL(QGL33* const gl);

    void switchToSkia();

    //! @brief Returns SkCanvas associated with the target texture.
    //! If there is no SkCanvas new SkCanvas is created.
    SkCanvas* requestTargetCanvas();

    //! @brief Returned texture may be used as one wishes,
    //!  but has to be valid.
    eTexture& getSrcTexture();

    //! @brief Returns TextureFrameBuffer associated with the target texture.
    //! If there is no SkCanvas new SkCanvas is created.
    eTextureFrameBuffer& requestTargetFbo();

    sk_sp<SkImage> requestSrcTextureImageWrapper();

    bool validTargetCanvas() const;
    bool validTargetFbo() const;
private:
    GrBackendTexture sourceBackedTexture();

    QGL33* const mGL;
    SwitchableContext& mContext;

    eTexture mSrcTexture;
    eTextureFrameBuffer mTargetTextureFbo;

    sk_sp<SkSurface> mSurface;
    SkCanvas* mCanvas = nullptr;
};


#endif // GPURENDERTOOLS_H