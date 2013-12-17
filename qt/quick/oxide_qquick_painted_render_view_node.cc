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

#include "oxide_qquick_painted_render_view_node.h"

#include <QPainter>
#include <QPixmap>

namespace oxide {
namespace qquick {

PaintedRenderViewNode::PaintedRenderViewNode() :
    backing_store_(NULL) {
  texture_.setOwnsTexture(true);
  texture_.setHasAlphaChannel(true);

  setTexture(&texture_);
}

void PaintedRenderViewNode::markDirtyRect(const QRect& rect) {
  dirty_rect_ |= rect;
  dirty_rect_ &= this->rect().toRect();
  markDirty(QSGNode::DirtyMaterial);
}

void PaintedRenderViewNode::setBackingStore(const QPixmap* pixmap) {
  if (pixmap == backing_store_) {
    return;
  }

  backing_store_ = pixmap;

  markDirtyRect(rect().toRect());
}

void PaintedRenderViewNode::setSize(const QSize& size) {
  QRect rect(QPoint(0, 0), size);
  if (rect == this->rect()) {
    return;
  }

  setRect(rect);

  texture_.setTextureSize(size);

  image_ = QImage(size, QImage::Format_ARGB32_Premultiplied);
  image_.fill(Qt::transparent);

  markDirtyRect(rect);
}

void PaintedRenderViewNode::update() {
  if (dirty_rect_.isEmpty()) {
    return;
  }

  QRect dirty_rect = dirty_rect_;
  dirty_rect_ = QRect();

  if (image_.isNull()) {
    return;
  }

  QPainter painter;
  painter.begin(&image_);

  if (!dirty_rect.isNull()) {
    painter.setClipRect(dirty_rect);
  }

  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.fillRect(dirty_rect, Qt::transparent);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

  if (backing_store_) {
    painter.drawPixmap(rect(), *backing_store_, rect());
  }

  painter.end();

  texture_.setImage(image_);
}

} // namespace qquick
} // namespace oxide
