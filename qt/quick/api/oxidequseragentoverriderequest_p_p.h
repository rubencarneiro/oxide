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

#ifndef _OXIDE_QT_QUICK_API_USER_AGENT_OVERRIDE_REQUEST_P_P_H_
#define _OXIDE_QT_QUICK_API_USER_AGENT_OVERRIDE_REQUEST_P_P_H_

#include <QString>
#include <QtGlobal>
#include <QUrl>

class OxideQUserAgentOverrideRequest;

class OxideQUserAgentOverrideRequestPrivate final {
 public:
  ~OxideQUserAgentOverrideRequestPrivate();

  static OxideQUserAgentOverrideRequestPrivate* get(
      OxideQUserAgentOverrideRequest* q);

  QString user_agent;

 private:
  friend class OxideQUserAgentOverrideRequest;

  OxideQUserAgentOverrideRequestPrivate(const QUrl& url);

  QUrl url_;
};

#endif // _OXIDE_QT_QUICK_API_USER_AGENT_OVERRIDE_REQUEST_P_P_H_
