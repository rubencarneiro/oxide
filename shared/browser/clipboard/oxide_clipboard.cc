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

#include "oxide_clipboard.h"

#include "base/logging.h"

#include "oxide_clipboard_observer.h"

namespace oxide {

Clipboard::Clipboard() {
  DCHECK(CalledOnValidThread());
  for (int i = 0; i < ui::CLIPBOARD_TYPE_LAST; ++i) {
    cached_info_->push_back(CachedInfo());
  }
}

void Clipboard::NotifyClipboardDataChanged(ui::ClipboardType type) {
  DCHECK(CalledOnValidThread());
  cached_info_[type].needs_update = true;
  for (auto& observer : observers_) {
    observer.ClipboardDataChanged(type);
  }
}

void Clipboard::AddObserver(ClipboardObserver* observer) {
  DCHECK(CalledOnValidThread());
  observers_.AddObserver(observer);
}

void Clipboard::RemoveObserver(ClipboardObserver* observer) {
  DCHECK(CalledOnValidThread());
  observers_.RemoveObserver(observer);
}

Clipboard::~Clipboard() {
  DCHECK(CalledOnValidThread());
  for (auto& observer : observers_) {
    observer.OnClipboardDestruction();
  }
}

// static
Clipboard* Clipboard::GetForCurrentThread() {
  return static_cast<Clipboard*>(ui::Clipboard::GetForCurrentThread());
}

bool Clipboard::HasData(ui::ClipboardType type) {
  DCHECK(CalledOnValidThread());

  if (cached_info_[type].needs_update) {
    base::string16 text;
    ReadText(type, &text);
    cached_info_[type].has_data = !text.empty();
    cached_info_[type].needs_update = false;
  }

  return cached_info_[type].has_data;
}

} // namespace oxide
