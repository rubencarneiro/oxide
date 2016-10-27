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

#include "navigation_history_impl.h"

#include <QDateTime>
#include <QString>
#include <QUrl>

#include "base/logging.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"

#include "qt/core/glue/navigation_history_client.h"

#include "web_contents_id_tracker.h"

namespace oxide {
namespace qt {

void NavigationHistoryImpl::init(WebContentsID web_contents_id) {
  DCHECK(!contents_);

  contents_ =
      WebContentsIDTracker::GetInstance()->GetWebContentsFromID(web_contents_id);
  DCHECK(contents_);

  Observe(&contents_->GetController());

  client_->NavigationHistoryChanged();
}

int NavigationHistoryImpl::getCurrentItemIndex() const {
  if (!contents_) {
    return -1;
  }

  return contents_->GetController().GetCurrentEntryIndex();
}

void NavigationHistoryImpl::goToIndex(int index) {
  contents_->GetController().GoToIndex(index);
}

void NavigationHistoryImpl::goToOffset(int offset) {
  contents_->GetController().GoToOffset(offset);
}

int NavigationHistoryImpl::getItemCount() const {
  if (!contents_) {
    return 0;
  }

  return contents_->GetController().GetEntryCount();
}

int NavigationHistoryImpl::getItemIndex(NavigationHistoryItem* item) const {
  if (item->history() != this) {
    return -1;
  }

  auto it = items_.find(item->id());
  if (it == items_.end() || it->second != item) {
    return -1;
  }

  content::NavigationController& controller = contents_->GetController();
  for (int i = 0; i < controller.GetEntryCount(); ++i) {
    content::NavigationEntry* entry = controller.GetEntryAtIndex(i);
    if (entry->GetUniqueID() == item->id()) {
      return i;
    }
  }

  return -1;
}

QExplicitlySharedDataPointer<NavigationHistoryItem>
NavigationHistoryImpl::getItemAtIndex(int index) {
  DCHECK(contents_);

  content::NavigationEntry* entry =
      contents_->GetController().GetEntryAtIndex(index);
  DCHECK(entry);

  QExplicitlySharedDataPointer<NavigationHistoryItem> item;

  auto it = items_.find(entry->GetUniqueID());
  if (it == items_.end()) {
    item = new NavigationHistoryItem(this, entry->GetUniqueID());
    items_.insert(std::make_pair(entry->GetUniqueID(), item.data()));
    item->UpdateFromEntry(entry);
  } else {
    item = it->second;
  }

  return item;
}

bool NavigationHistoryImpl::canGoBack() const {
  if (!contents_) {
    return false;
  }

  return contents_->GetController().CanGoBack();
}

bool NavigationHistoryImpl::canGoForward() const {
  if (!contents_) {
    return false;
  }

  return contents_->GetController().CanGoForward();
}

bool NavigationHistoryImpl::canGoToOffset(int offset) const {
  if (!contents_) {
    return false;
  }

  return contents_->GetController().CanGoToOffset(offset);
}

void NavigationHistoryImpl::goBack() {
  contents_->GetController().GoBack();
}

void NavigationHistoryImpl::goForward() {
  contents_->GetController().GoForward();
}

void NavigationHistoryImpl::NavigationHistoryChanged() {
  content::NavigationController& controller = contents_->GetController();

  size_t remaining = items_.size();
  for (int i = 0; i < controller.GetEntryCount() && remaining > 0; ++i) {
    content::NavigationEntry* entry = controller.GetEntryAtIndex(i);
    auto it = items_.find(entry->GetUniqueID());
    if (it == items_.end()) {
      continue;
    }
    --remaining;
    it->second->UpdateFromEntry(entry);
  }

  client_->NavigationHistoryChanged();
}

NavigationHistoryImpl::NavigationHistoryImpl(NavigationHistoryClient* client,
                                             QObject* handle)
    : client_(client),
      contents_(nullptr) {
  setHandle(handle);
}

NavigationHistoryImpl::~NavigationHistoryImpl() {
  for (auto& kv : items_) {
    DCHECK_EQ(kv.second->history(), this);
    kv.second->DetachFromHistory();
  }
}

void NavigationHistoryImpl::RemoveItem(NavigationHistoryItem* item) {
  DCHECK_EQ(item->history(), this);

  auto it = items_.find(item->id());
  if (it == items_.end() || it->second != item) {
    return;
  }

  items_.erase(it);
}

} // namespace qt
} // namespace oxide
