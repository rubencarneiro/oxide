// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "oxide_qquick_image_frame_node.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include "qt/core/glue/contents_view.h"

namespace oxide {
namespace qquick {

int ImageFrameNode::textureId() const {
  if (texture_id_ == 0) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    QOpenGLContext::currentContext()->functions()->glGenTextures(
        1,
        &const_cast<ImageFrameNode*>(this)->texture_id_);
#else
    typedef void (*glGenTexturesProc)(GLsizei n, GLuint* textures);
    static glGenTexturesProc glGenTexturesFn =
        reinterpret_cast<glGenTexturesProc>(
            QOpenGLContext::currentContext()->getProcAddress("glGenTextures"));
    Q_ASSERT(glGenTexturesFn);

    glGenTexturesFn(1, &const_cast<ImageFrameNode*>(this)->texture_id_);
#endif
  }

  return texture_id_;
}

QSize ImageFrameNode::textureSize() const {
  if (!handle_) {
    return QSize();
  }

  return handle_->GetSizeInPixels();
}

bool ImageFrameNode::hasAlphaChannel() const {
  return true;
}

bool ImageFrameNode::hasMipmaps() const {
  return false;
}

void ImageFrameNode::bind() {
  QOpenGLContext* context = QOpenGLContext::currentContext();
  QOpenGLFunctions* functions = context->functions();

#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
  functions->glBindTexture(GL_TEXTURE_2D, textureId());
#else
  typedef void (*glBindTextureProc)(GLenum target, GLuint texture);
  static glBindTextureProc glBindTextureFn =
      reinterpret_cast<glBindTextureProc>(
          context->getProcAddress("glBindTexture"));
  Q_ASSERT(glBindTextureFn);

  glBindTextureFn(GL_TEXTURE_2D, textureId());
#endif

  updateBindOptions(dirty_bind_options_);
  dirty_bind_options_ = false;

  typedef void (*glEGLImageTargetTexture2DOESProc)(GLenum target,
                                                   GLeglImageOES image);
  static glEGLImageTargetTexture2DOESProc glEGLImageTargetTexture2DOESFn =
      reinterpret_cast<glEGLImageTargetTexture2DOESProc>(
        context->getProcAddress("glEGLImageTargetTexture2DOES"));
  Q_ASSERT(glEGLImageTargetTexture2DOESFn);

  glEGLImageTargetTexture2DOESFn(GL_TEXTURE_2D,
                                 handle_->GetImageFrame());
  Q_ASSERT(static_cast<GLenum>(GL_NO_ERROR) == functions->glGetError());
}

ImageFrameNode::ImageFrameNode()
    : dirty_bind_options_(true),
      texture_id_(0) {
  QSGTexture::setFiltering(QSGTexture::Linear);
  setHorizontalWrapMode(QSGTexture::ClampToEdge);
  setVerticalWrapMode(QSGTexture::ClampToEdge);
  setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
}

ImageFrameNode::~ImageFrameNode() {
  QOpenGLContext* context = QOpenGLContext::currentContext();
  if (context && texture_id_ != 0) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    context->functions()->glDeleteTextures(1, &texture_id_);
#else
    typedef void (*glDeleteTexturesProc)(GLsizei n, const GLuint* textures);
    static glDeleteTexturesProc glDeleteTexturesFn =
        reinterpret_cast<glDeleteTexturesProc>(
            context->getProcAddress("glDeleteTextures"));
    Q_ASSERT(glDeleteTexturesFn);

    glDeleteTexturesFn(1, &texture_id_);
#endif
  }
}

void ImageFrameNode::updateNode(
    QSharedPointer<oxide::qt::CompositorFrameHandle> handle) {
  handle_ = handle;
  setRect(handle_->GetRect());
  setTexture(this);
}

} // namespace qquick
} // namespace oxide
