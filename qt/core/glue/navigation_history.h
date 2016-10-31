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

#ifndef _OXIDE_QT_CORE_GLUE_NAVIGATION_HISTORY_H_
#define _OXIDE_QT_CORE_GLUE_NAVIGATION_HISTORY_H_

#include <memory>

#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QSharedData>
#include <QString>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/api/oxideqglobal.h"
#include "qt/core/glue/oxide_qt_proxy_base.h"
#include "qt/core/glue/web_contents_id.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace content {
class NavigationEntry;
}

namespace oxide {
namespace qt {

class NavigationHistoryClient;
class NavigationHistoryImpl;

class OXIDE_QTCORE_EXPORT NavigationHistoryItem : public QSharedData {
 public:
  NavigationHistoryItem();
  NavigationHistoryItem(NavigationHistoryImpl* history,
                        int id);
  NavigationHistoryItem(const QUrl& url,
                        const QUrl& original_url,
                        const QString& title,
                        const QDateTime& timestamp);
  ~NavigationHistoryItem();

  QUrl url() const { return url_; }
  QUrl original_url() const { return original_url_; }
  QString title() const { return title_; }
  QDateTime timestamp() const { return timestamp_; }

#if defined(OXIDE_QTCORE_IMPLEMENTATION)
  NavigationHistoryImpl* history() const { return history_; }
  int id() const { return id_; }

  void DetachFromHistory() { history_ = nullptr; }
  void UpdateFromEntry(content::NavigationEntry* entry);
#endif

 private:
  NavigationHistoryImpl* history_ = nullptr;

  int id_ = 0;
  QUrl url_;
  QUrl original_url_;
  QString title_;
  QDateTime timestamp_;
};

class OXIDE_QTCORE_EXPORT NavigationHistory
    : public ProxyBase<NavigationHistory> {
 public:
  static std::unique_ptr<NavigationHistory> create(
      NavigationHistoryClient* client,
      QObject* handle);

  virtual ~NavigationHistory();

  virtual void init(WebContentsID web_contents_id) = 0;

  virtual int getCurrentItemIndex() const = 0;

  virtual void goToIndex(int index) = 0;
  virtual void goToOffset(int offset) = 0;

  virtual int getItemCount() const = 0;

  virtual int getItemIndex(NavigationHistoryItem* item) const = 0;
  virtual QExplicitlySharedDataPointer<NavigationHistoryItem> getItemAtIndex(
      int index) = 0;

  virtual bool canGoBack() const = 0;
  virtual bool canGoForward() const = 0;
  virtual bool canGoToOffset(int offset) const = 0;

  virtual void goBack() = 0;
  virtual void goForward() = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_NAVIGATION_HISTORY_H_
