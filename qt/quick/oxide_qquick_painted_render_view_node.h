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

#ifndef _OXIDE_QT_QUICK_PAINTED_RENDER_VIEW_NODE_H_
#define _OXIDE_QT_QUICK_PAINTED_RENDER_VIEW_NODE_H_

#include <QImage>
#include <QRect>
#include <QScopedPointer>
#include <QSize>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGOpaqueTextureMaterial>
#include <QSGTextureMaterial>
#include <QtGlobal>
#include <QtQuick/private/qsgtexture_p.h>

QT_BEGIN_NAMESPACE
class QPixmap;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {

class PaintedRenderViewNode Q_DECL_FINAL : public QSGGeometryNode {
 public:
  PaintedRenderViewNode();

  QSize size() const;
  void setSize(const QSize& size);

  void markDirtyRect(const QRect& rect);
  void setBackingStore(const QPixmap* pixmap);

  virtual void preprocess() Q_DECL_FINAL;

 private:
  QSize size_;
  QRect dirty_rect_;

  const QPixmap* backing_store_;
  QImage image_;

  QSGOpaqueTextureMaterial material_o_;
  QSGTextureMaterial material_;
  QSGGeometry geometry_;
  QSGPlainTexture texture_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_PAINTED_RENDER_VIEW_NODE_H_
