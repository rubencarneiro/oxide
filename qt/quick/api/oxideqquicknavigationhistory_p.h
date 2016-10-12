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

#ifndef _OXIDE_QT_QUICK_API_NAVIGATION_HISTORY_P_H_
#define _OXIDE_QT_QUICK_API_NAVIGATION_HISTORY_P_H_

#include <memory>

#include <QList>
#include <QtGlobal>

#include "qt/core/glue/navigation_history.h"
#include "qt/core/glue/navigation_history_client.h"
#include "qt/core/glue/web_contents_id.h"

#include "qt/quick/api/oxideqquicknavigationitem.h"

class OxideQQuickNavigationHistory;

class OxideQQuickNavigationHistoryPrivate
    : public oxide::qt::NavigationHistoryClient {
  Q_DECLARE_PUBLIC(OxideQQuickNavigationHistory)
  Q_DISABLE_COPY(OxideQQuickNavigationHistoryPrivate)

 public:
  ~OxideQQuickNavigationHistoryPrivate();

  static OxideQQuickNavigationHistoryPrivate* get(
      OxideQQuickNavigationHistory* q);

  void init(oxide::qt::WebContentsID web_contents_id);

 private:
  OxideQQuickNavigationHistoryPrivate(OxideQQuickNavigationHistory* q);

  void ensureModelItemsAreBuilt();

  OxideQQuickNavigationItem constructItemForIndex(int index);

  // oxide::qt::NavigationHistoryClient implementation
  void NavigationHistoryChanged() override;

  enum Roles {
    Url = Qt::UserRole + 1,
    Title,
    Timestamp
  };

  OxideQQuickNavigationHistory* q_ptr;

  std::unique_ptr<oxide::qt::NavigationHistory> navigation_history_;

  struct ModelHistoryItem;

  bool model_items_need_rebuilding_;
  QList<ModelHistoryItem> model_items_;
};

#endif // _OXIDE_QT_QUICK_API_NAVIGATION_HISTORY_P_H_
