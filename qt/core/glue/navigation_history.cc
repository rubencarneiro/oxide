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

#include "navigation_history.h"

#include "base/memory/ptr_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "content/public/browser/navigation_entry.h"

#include "qt/core/browser/navigation_history_impl.h"

namespace oxide {
namespace qt {

NavigationHistoryItem::NavigationHistoryItem() = default;

NavigationHistoryItem::NavigationHistoryItem(NavigationHistoryImpl* history,
                                             int id)
    : history_(history),
      id_(id) {}

NavigationHistoryItem::NavigationHistoryItem(const QUrl& url,
                                             const QUrl& original_url,
                                             const QString& title,
                                             const QDateTime& timestamp)
    : url_(url),
      original_url_(original_url),
      title_(title),
      timestamp_(timestamp) {}

NavigationHistoryItem::~NavigationHistoryItem() {
  if (history_) {
    history_->RemoveItem(this);
  }
}

void NavigationHistoryItem::UpdateFromEntry(content::NavigationEntry* entry) {
  DCHECK_EQ(entry->GetUniqueID(), id_);

  url_ = QUrl(QString::fromStdString(entry->GetURL().spec()));
  original_url_ =
      QUrl(QString::fromStdString(entry->GetOriginalRequestURL().spec()));
  title_ = QString::fromStdString(base::UTF16ToUTF8(entry->GetTitle()));
  timestamp_ = QDateTime::fromMSecsSinceEpoch(entry->GetTimestamp().ToJsTime());
}

// static
std::unique_ptr<NavigationHistory> NavigationHistory::create(
    NavigationHistoryClient* client,
    QObject* handle) {
  return base::MakeUnique<NavigationHistoryImpl>(client, handle);
}

NavigationHistory::~NavigationHistory() = default;

} // namespace qt
} // namespace oxide
