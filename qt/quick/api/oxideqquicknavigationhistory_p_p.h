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

#ifndef _OXIDE_QT_QUICK_API_NAVIGATION_HISTORY_P_P_H_
#define _OXIDE_QT_QUICK_API_NAVIGATION_HISTORY_P_P_H_

#include <QMap>

class OxideQQuickNavigationHistory;
class OxideQQuickWebView;

struct NavigationEntry;

class OxideQQuickNavigationHistoryPrivate {
  Q_DECLARE_PUBLIC(OxideQQuickNavigationHistory)

  enum Roles {
    Url = Qt::UserRole + 1,
    Title,
    Timestamp
  };

  OxideQQuickNavigationHistory* q_ptr;
  OxideQQuickWebView* webview_;
  int entry_count_;
  int current_index_;
  QMap<int, NavigationEntry*> entry_cache_;
};

#endif // _OXIDE_QT_QUICK_API_NAVIGATION_HISTORY_P_P_H_
