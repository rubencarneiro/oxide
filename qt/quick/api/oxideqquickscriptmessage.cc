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

#include "oxideqquickscriptmessage_p.h"
#include "oxideqquickscriptmessage_p_p.h"

#include "oxideqquickwebframe_p.h"
#include "oxideqquickwebframe_p_p.h"

OxideQQuickScriptMessagePrivate::OxideQQuickScriptMessagePrivate(
    OxideQQuickScriptMessage* q) :
    oxide::qt::ScriptMessageAdapter(q) {}

// static
OxideQQuickScriptMessagePrivate* OxideQQuickScriptMessagePrivate::get(
    OxideQQuickScriptMessage* q) {
  return q->d_func();
}

OxideQQuickScriptMessage::OxideQQuickScriptMessage() :
    d_ptr(new OxideQQuickScriptMessagePrivate(this)) {}

OxideQQuickScriptMessage::~OxideQQuickScriptMessage() {}

OxideQQuickWebFrame* OxideQQuickScriptMessage::frame() const {
  Q_D(const OxideQQuickScriptMessage);

  return adapterToQObject<OxideQQuickWebFrame>(d->frame());
}

QUrl OxideQQuickScriptMessage::context() const {
  Q_D(const OxideQQuickScriptMessage);

  return d->context();
}

QString OxideQQuickScriptMessage::msgId() const {
  Q_D(const OxideQQuickScriptMessage);

  return d->msgId();
}

QVariant OxideQQuickScriptMessage::args() const {
  Q_D(const OxideQQuickScriptMessage);

  return d->args();
}

void OxideQQuickScriptMessage::reply(const QVariant& args) {
  Q_D(OxideQQuickScriptMessage);

  d->reply(args);
}

void OxideQQuickScriptMessage::error(const QString& msg) {
  Q_D(OxideQQuickScriptMessage);

  d->error(msg);
}
