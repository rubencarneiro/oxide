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

#include "oxideqloadevent.h"

class OxideQLoadEventPrivate {
 public:
  ~OxideQLoadEventPrivate() {}
  OxideQLoadEventPrivate(const QUrl& url,
                         OxideQLoadEvent::Type type,
                         OxideQLoadEvent::ErrorDomain error_domain,
                         const QString& error_string,
                         int error_code,
                         const QUrl& redirection_url,
                         bool is_main_frame) :
      url(url), type(type), error_domain(error_domain),
      error_string(error_string), error_code(error_code),
      redirection_url(redirection_url), is_main_frame(is_main_frame) {}

  QUrl url;
  OxideQLoadEvent::Type type;
  OxideQLoadEvent::ErrorDomain error_domain;
  QString error_string;
  int error_code;
  QUrl redirection_url;
  bool is_main_frame;
};

OxideQLoadEvent::OxideQLoadEvent(const QUrl& url,
                                 Type type,
                                 ErrorDomain error_domain,
                                 const QString& error_string,
                                 int error_code,
                                 const QUrl& redirection_url,
                                 bool is_main_frame) :
     d_ptr(new OxideQLoadEventPrivate(
         url, type, error_domain, error_string,
         error_code, redirection_url, is_main_frame)) {}

OxideQLoadEvent::~OxideQLoadEvent() {}

QUrl OxideQLoadEvent::url() const {
  Q_D(const OxideQLoadEvent);

  return d->url;
}

OxideQLoadEvent::Type OxideQLoadEvent::type() const {
  Q_D(const OxideQLoadEvent);

  return d->type;
}

OxideQLoadEvent::ErrorDomain OxideQLoadEvent::errorDomain() const {
  Q_D(const OxideQLoadEvent);

  return d->error_domain;
}

QString OxideQLoadEvent::errorString() const {
  Q_D(const OxideQLoadEvent);

  return d->error_string;
}

int OxideQLoadEvent::errorCode() const {
  Q_D(const OxideQLoadEvent);

  return d->error_code;
}

QUrl OxideQLoadEvent::redirectedUrl() const {
  Q_D(const OxideQLoadEvent);

  return d->redirection_url;
}

bool OxideQLoadEvent::isMainFrame() const {
  Q_D(const OxideQLoadEvent);

  return d->is_main_frame;
}
