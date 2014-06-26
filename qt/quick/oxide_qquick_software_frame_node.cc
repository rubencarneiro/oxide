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

#include "oxide_qquick_software_frame_node.h"

#include <QImage>
#include <QPoint>
#include <QQuickWindow>
#include <QRect>

#include "qt/core/glue/oxide_qt_render_widget_host_view_delegate.h"

#include "oxide_qquick_render_view_item.h"

namespace oxide {
namespace qquick {

SoftwareFrameNode::SoftwareFrameNode(RenderViewItem* item)
    : item_(item) {}

void SoftwareFrameNode::updateNode(
    QSharedPointer<oxide::qt::CompositorFrameHandle> handle) {
  handle_ = handle;

  setRect(QRect(QPoint(0, 0), handle_->GetSize()));

  texture_.reset(item_->window()->createTextureFromImage(
      handle_->GetSoftwareFrame(),
      QQuickWindow::TextureHasAlphaChannel));
  setTexture(texture_.data());
}

void SoftwareFrameNode::setImage(const QImage& image) {
  handle_.reset();

  setRect(QRect(QPoint(0, 0), image.size()));

  texture_.reset(item_->window()->createTextureFromImage(
      image, QQuickWindow::TextureHasAlphaChannel));
  setTexture(texture_.data());
}

} // namespace qquick
} // namespace oxide
