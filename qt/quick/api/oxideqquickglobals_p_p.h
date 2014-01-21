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

#ifndef _OXIDE_QT_QUICK_API_GLOBALS_P_P_H_
#define _OXIDE_QT_QUICK_API_GLOBALS_P_P_H_

#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>

class OxideQQuickGlobals;

class OxideQQuickGlobalsPrivate Q_DECL_FINAL {
  Q_DECLARE_PUBLIC(OxideQQuickGlobals)

 public:
  OxideQQuickGlobalsPrivate(OxideQQuickGlobals* q);

  bool has_default_context() const { return has_default_context_; }

  void defaultContextCreated();
  void defaultContextDestroyed();

  void defaultContextProductChanged();
  void defaultContextUserAgentChanged();
  void defaultContextDataPathChanged();
  void defaultContextCachePathChanged();
  void defaultContextAcceptLangsChanged();

  static OxideQQuickGlobalsPrivate* get(OxideQQuickGlobals* q);

  QString product;
  QString user_agent;
  QUrl data_path;
  QUrl cache_path;
  QString accept_langs;

 private:
  OxideQQuickGlobals* q_ptr;
  bool has_default_context_;
};

#endif // _OXIDE_QT_QUICK_API_GLOBALS_P_P_H_
