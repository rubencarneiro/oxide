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

#ifndef _OXIDE_QT_CORE_GLUE_RENDER_WIDGET_HOST_VIEW_DELEGATE_P_H_
#define _OXIDE_QT_CORE_GLUE_RENDER_WIDGET_HOST_VIEW_DELEGATE_P_H_

#include <QSize>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"

namespace oxide {

class TextureHandle;

namespace qt {

class RenderWidgetHostView;

class TextureHandleImpl FINAL : public TextureHandle {
 public:
  TextureHandleImpl();
  ~TextureHandleImpl();

  unsigned int GetID() const FINAL;
  QSize GetSize() const FINAL;

  void SetHandle(oxide::TextureHandle* handle);

 private:
  scoped_refptr<oxide::TextureHandle> handle_;
};

class RenderWidgetHostViewDelegatePrivate FINAL {
 public:
  RenderWidgetHostViewDelegatePrivate();

  static RenderWidgetHostViewDelegatePrivate* get(
      RenderWidgetHostViewDelegate* delegate);

  RenderWidgetHostView* rwhv;
  TextureHandleImpl current_texture_handle_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_RENDER_WIDGET_HOST_VIEW_DELEGATE_P_H_
