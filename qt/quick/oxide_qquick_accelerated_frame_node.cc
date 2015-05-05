// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxide_qquick_accelerated_frame_node.h"

#include <QPoint>
#include <QQuickWindow>
#include <QRect>
#include <QSize>
#include <QSGTexture>

#include "qt/core/glue/oxide_qt_web_view_proxy.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

namespace oxide {
namespace qquick {

AcceleratedFrameNode::AcceleratedFrameNode(OxideQQuickWebView* view) :
    view_(view) {
  setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
}

AcceleratedFrameNode::~AcceleratedFrameNode() {}

void AcceleratedFrameNode::updateNode(
    QSharedPointer<oxide::qt::CompositorFrameHandle> handle) {
  handle_ = handle;

  setRect(handle_->GetRect());

  texture_.reset(view_->window()->createTextureFromId(
      handle_->GetAcceleratedFrameTexture(),
      handle_->GetRect().size(),
      QQuickWindow::TextureHasAlphaChannel));
  setTexture(texture_.data());
}

} // namespace qquick
} // namespace oxide
