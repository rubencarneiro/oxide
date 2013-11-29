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

namespace oxide {
namespace qquick {

PaintedRenderViewNode::PaintedRenderViewNode() :
    backing_store_(NULL),
    geometry_(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4) {
  setFlag(QSGNode::UsePreprocess);

  setGeometry(&geometry_);
  setMaterial(&material_);
  setOpaqueMaterial(&material_o_);

  texture_.setOwnsTexture(true);
  texture_.setHasAlphaChannel(true);

  material_.setTexture(&texture_);
  material_o_.setTexture(&texture_);
}

QSize PaintedRenderViewNode::size() const {
  return size_;
}

void PaintedRenderViewNode::setSize(const QSize& size) {
  if (size == size_) {
    return;
  }

  size_ = size;

  QSGGeometry::updateTexturedRectGeometry(
      &geometry_,
      QRectF(0, 0, size_.width(), size_.height()),
      QRectF(0, 0, 1, 1));

  texture_.setTextureSize(size_);

  image_ = QImage(size_, QImage::Format_ARGB32_Premultiplied);
  image_.fill(Qt::transparent);

  markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);

  markDirtyRect(QRect(QPoint(0, 0), size_));
}

void PaintedRenderViewNode::markDirtyRect(const QRect& rect) {
  dirty_rect_ |= rect;
  markDirty(QSGNode::DirtyMaterial);
}

void PaintedRenderViewNode::setBackingStore(const QPixmap* pixmap) {
  if (pixmap == backing_store_) {
    return;
  }

  backing_store_ = pixmap;

  markDirtyRect(QRect(QPoint(0, 0), size()));
}

void PaintedRenderViewNode::preprocess() {
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
    QRectF rect(0, 0, size().width(), size().height());
    painter.drawPixmap(rect, *backing_store_, rect);
  }

  painter.end();

  texture_.setImage(image_);
}

} // namespace qquick
} // namespace oxide
