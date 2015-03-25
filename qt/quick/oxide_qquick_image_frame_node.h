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

#ifndef _OXIDE_QQUICK_IMAGE_FRAME_NODE_H_
#define _OXIDE_QQUICK_IMAGE_FRAME_NODE_H_

#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QSharedPointer>
#include <QtGlobal>

namespace oxide {

namespace qt{
class CompositorFrameHandle;
}

namespace qquick {

class ImageFrameNode : public QSGSimpleTextureNode,
                       public QSGTexture {
 public:
  ImageFrameNode();
  ~ImageFrameNode() override;

  void updateNode(QSharedPointer<oxide::qt::CompositorFrameHandle> handle);

 private:
  // QSGTexture implementation
  int textureId() const override;
  QSize textureSize() const override;
  bool hasAlphaChannel() const override;
  bool hasMipmaps() const override;
  void bind() override;

  QSharedPointer<oxide::qt::CompositorFrameHandle> handle_;
  bool dirty_bind_options_;
  GLuint texture_id_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QQUICK_IMAGE_FRAME_NODE_H_
