// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#include "oxideqquickscriptmessage.h"
#include "oxideqquickscriptmessage_p.h"

#include "qt/core/api/oxideqglobal_p.h"
#include "qt/core/glue/oxide_qt_script_message_proxy.h"

#include "oxideqquickwebframe.h"
#include "oxideqquickwebframe_p.h"

OxideQQuickScriptMessagePrivate::OxideQQuickScriptMessagePrivate(
    oxide::qt::ScriptMessageProxy* proxy,
    OxideQQuickScriptMessage* q)
    : q_ptr(q),
      proxy_(proxy) {}

// static
OxideQQuickScriptMessage* OxideQQuickScriptMessagePrivate::create(
    oxide::qt::ScriptMessageProxy* proxy) {
  OxideQQuickScriptMessage* message = new OxideQQuickScriptMessage();
  message->d_ptr.reset(new OxideQQuickScriptMessagePrivate(proxy, message));
  return message;
}

// static
OxideQQuickScriptMessagePrivate* OxideQQuickScriptMessagePrivate::get(
    OxideQQuickScriptMessage* q) {
  return q->d_func();
}

OxideQQuickScriptMessage::OxideQQuickScriptMessage() {}

OxideQQuickScriptMessage::~OxideQQuickScriptMessage() {}

OxideQQuickWebFrame* OxideQQuickScriptMessage::frame() const {
  Q_D(const OxideQQuickScriptMessage);

  QObject* f = d->proxy_->frame();
  if (!f) {
    return nullptr;
  }

  return qobject_cast<OxideQQuickWebFrame*>(f);
}

QUrl OxideQQuickScriptMessage::context() const {
  Q_D(const OxideQQuickScriptMessage);

  return d->proxy_->context();
}

QString OxideQQuickScriptMessage::msgId() const {
  Q_D(const OxideQQuickScriptMessage);

  return d->proxy_->msgId();
}

QVariant OxideQQuickScriptMessage::args() const {
  WARN_DEPRECATED_API_USAGE() <<
      "OxideQQuickScriptMessage: args is deprecated. Please use payload "
      "instead";

  return payload();
}

QVariant OxideQQuickScriptMessage::payload() const {
  Q_D(const OxideQQuickScriptMessage);

  return d->proxy_->payload();
}

void OxideQQuickScriptMessage::reply(const QVariant& payload) {
  Q_D(OxideQQuickScriptMessage);

  QVariant aux = payload;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  d->proxy_->reply(aux);
}

void OxideQQuickScriptMessage::error(const QVariant& payload) {
  Q_D(OxideQQuickScriptMessage);

  QVariant aux = payload;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  d->proxy_->error(aux);
}
