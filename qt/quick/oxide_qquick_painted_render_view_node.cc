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

PaintedRenderViewNode::PaintedRenderViewNode(
    RenderViewItem* item) :
    item_(item),
    backing_store_(NULL),
    geometry_(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4) {
  setGeometry(&geometry_);
  setMaterial(&material_);
  setOpaqueMaterial(&material_o_);

  material_.setTexture(&texture_);
  material_o_.setTexture(&texture_);

  texture_.setOwnsTexture(true);
  texture_.setHasAlphaChannel(true);
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

  setDirtyRect(QRect(QPoint(0, 0), this->size()));

  markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
}

void PaintedRenderViewNode::setDirtyRect(const QRect& rect) {
  dirty_rect_ = rect;
}

void PaintedRenderViewNode::setBackingStore(const QPixmap* pixmap) {
  if (pixmap == backing_store_) {
    return;
  }

  backing_store_ = pixmap;

  setDirtyRect(QRect(QPoint(0, 0), size()));
}

void PaintedRenderViewNode::update() {
  if (dirty_rect_.isEmpty()) {
    return;
  }

  markDirty(QSGNode::DirtyMaterial);

  if (image_.isNull()) {
    return;
  }

  QPainter painter;
  painter.begin(&image_);

  if (!dirty_rect_.isNull()) {
    painter.setClipRect(dirty_rect_);
  }

  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.fillRect(dirty_rect_, Qt::transparent);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

  if (backing_store_) {
    QRectF rect(0, 0, size_.width(), size_.height());
    painter.drawPixmap(rect, *backing_store_, rect);
  }

  painter.end();

  texture_.setImage(image_);

  dirty_rect_ = QRect();
}

} // namespace qquick
} // namespace oxide
