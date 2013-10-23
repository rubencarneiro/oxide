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

#ifndef _OXIDE_QT_LIB_BROWSER_BACKING_STORE_H_
#define _OXIDE_QT_LIB_BROWSER_BACKING_STORE_H_

#include <QPixmap>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/browser/renderer_host/backing_store.h"

QT_USE_NAMESPACE

namespace oxide {
namespace qt {

class BackingStore FINAL : public content::BackingStore {
 public:
  BackingStore(content::RenderWidgetHost* widget, const gfx::Size& size);

  void PaintToBackingStore(
      content::RenderProcessHost* process,
      TransportDIB::Id bitmap,
      const gfx::Rect& bitmap_rect,
      const std::vector<gfx::Rect>& copy_rects,
      float scale_factor,
      const base::Closure& completion_callback,
      bool* scheduled_completion_callback) FINAL;

  bool CopyFromBackingStore(const gfx::Rect& rect,
                            skia::PlatformBitmap* output) FINAL;

  void ScrollBackingStore(const gfx::Vector2d& delta,
                          const gfx::Rect& clip_rect,
                          const gfx::Size& view_size) FINAL;

  const QPixmap* pixmap() const { return &pixmap_; }

 private:
  QPixmap pixmap_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(BackingStore);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_BROWSER_BACKING_STORE_H_
