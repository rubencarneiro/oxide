// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_SOFTWARE_FRAME_NODE_H_
#define _OXIDE_QT_QUICK_SOFTWARE_FRAME_NODE_H_

#include <QScopedPointer>
#include <QSGSimpleTextureNode>
#include <QSharedPointer>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QImage;
class QQuickItem;
class QSGTexture;
QT_END_NAMESPACE

namespace oxide {

namespace qt {
class CompositorFrameHandle;
}

namespace qquick {

class SoftwareFrameNode final : public QSGSimpleTextureNode {
 public:
  SoftwareFrameNode(QQuickItem* item);

  void updateNode(QSharedPointer<oxide::qt::CompositorFrameHandle> handle);
  void setImage(const QImage& image);

 private:
  QQuickItem* item_;

  QSharedPointer<oxide::qt::CompositorFrameHandle> handle_;
  QScopedPointer<QSGTexture> texture_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_SOFTWARE_FRAME_NODE_H_
