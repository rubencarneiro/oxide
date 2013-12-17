// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "oxide_qquick_accelerated_render_view_node.h"

#include <QQuickWindow>
#include <QSGTexture>

#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate.h"

#include "oxide_qquick_render_view_item.h"

namespace oxide {
namespace qquick {

AcceleratedRenderViewNode::AcceleratedRenderViewNode(RenderViewItem* item) :
    item_(item),
    front_texture_(NULL),
    back_texture_(NULL) {
  setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
}

AcceleratedRenderViewNode::~AcceleratedRenderViewNode() {
  delete front_texture_;
  delete back_texture_;
}

void AcceleratedRenderViewNode::updateFrontTexture(
    const oxide::qt::TextureInfo& tex_info) {
  if (front_texture_ &&
      static_cast<unsigned int>(front_texture_->textureId()) == tex_info.id() &&
      front_texture_->textureSize() == tex_info.size_in_pixels()) {
    markDirty(QSGNode::DirtyMaterial);
    return;
  }

  if (!back_texture_ ||
      static_cast<unsigned int>(back_texture_->textureId()) != tex_info.id() ||
      back_texture_->textureSize() != tex_info.size_in_pixels()) {
    delete back_texture_;
    back_texture_ = NULL;

    back_texture_ =
        item_->window()->createTextureFromId(
          tex_info.id(),
          tex_info.size_in_pixels(),
          QQuickWindow::TextureHasAlphaChannel);
  }

  std::swap(front_texture_, back_texture_);
  setTexture(front_texture_);
}

} // namespace qquick
} // namespace oxide
