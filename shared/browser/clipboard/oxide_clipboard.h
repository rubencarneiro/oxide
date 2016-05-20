// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_CLIPBOARD_CLIPBOARD_H_
#define _OXIDE_SHARED_BROWSER_CLIPBOARD_CLIPBOARD_H_

#include "base/containers/stack_container.h"
#include "base/macros.h"
#include "base/observer_list.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/clipboard/clipboard_types.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

class ClipboardObserver;

class OXIDE_SHARED_EXPORT Clipboard : public ui::Clipboard {
 public:
  ~Clipboard() override;

  static Clipboard* GetForCurrentThread();

  bool HasData(ui::ClipboardType type);

 protected:
  Clipboard();

  void NotifyClipboardDataChanged(ui::ClipboardType type);

 private:
  friend class ClipboardObserver;

  void AddObserver(ClipboardObserver* observer);
  void RemoveObserver(ClipboardObserver* observer);

  base::ObserverList<ClipboardObserver> observers_;

  struct CachedInfo {
    bool needs_update = true;
    bool has_data = false;
  };

  base::StackVector<CachedInfo, ui::CLIPBOARD_TYPE_LAST + 1> cached_info_;

  DISALLOW_COPY_AND_ASSIGN(Clipboard);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_CLIPBOARD_CLIPBOARD_H_
