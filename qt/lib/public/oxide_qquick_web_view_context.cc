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

#include "oxide_qquick_web_view_context_p.h"

#include "oxide_q_user_script.h"
#include "oxide_qquick_web_view_context_p_p.h"

QT_USE_NAMESPACE

namespace {
OxideQQuickWebViewContext* g_default_context;
}

OxideQQuickWebViewContextPrivate::OxideQQuickWebViewContextPrivate(
    OxideQQuickWebViewContext* q) :
    oxide::qt::WebViewContextPrivate(q) {}

OxideQQuickWebViewContextPrivate::~OxideQQuickWebViewContextPrivate() {}

OxideQQuickWebViewContextPrivate* OxideQQuickWebViewContextPrivate::get(
    OxideQQuickWebViewContext* context) {
  return context->d_func();
}

void OxideQQuickWebViewContextPrivate::userScript_append(
    QQmlListProperty<OxideQUserScript>* prop,
    OxideQUserScript* user_script) {
  if (!user_script) {
    return;
  }

  OxideQQuickWebViewContext* context =
      static_cast<OxideQQuickWebViewContext *>(prop->object);
  OxideQQuickWebViewContextPrivate* p =
      OxideQQuickWebViewContextPrivate::get(context);

  if (!p->user_scripts().isEmpty() && p->user_scripts().contains(user_script)) {
    p->user_scripts().removeOne(user_script);
    p->user_scripts().append(user_script);
  } else {
    QObject::connect(user_script, SIGNAL(scriptLoaded()),
                     context, SLOT(scriptUpdated()));
    QObject::connect(user_script, SIGNAL(scriptPropertyChanged()),
                     context, SLOT(scriptUpdated()));
    p->user_scripts().append(user_script);
  }

  p->updateUserScripts();
}

int OxideQQuickWebViewContextPrivate::userScript_count(
    QQmlListProperty<OxideQUserScript>* prop) {
  OxideQQuickWebViewContextPrivate* p =
      OxideQQuickWebViewContextPrivate::get(
        static_cast<OxideQQuickWebViewContext *>(prop->object));

  return p->user_scripts().size();
}

OxideQUserScript* OxideQQuickWebViewContextPrivate::userScript_at(
    QQmlListProperty<OxideQUserScript>* prop,
    int index) {
  OxideQQuickWebViewContextPrivate* p =
      OxideQQuickWebViewContextPrivate::get(
        static_cast<OxideQQuickWebViewContext *>(prop->object));

  if (index >= p->user_scripts().size()) {
    return NULL;
  }

  return qobject_cast<OxideQUserScript *>(p->user_scripts().at(index));
}

void OxideQQuickWebViewContextPrivate::userScript_clear(
    QQmlListProperty<OxideQUserScript>* prop) {
  OxideQQuickWebViewContext* context =
      static_cast<OxideQQuickWebViewContext *>(prop->object);
  OxideQQuickWebViewContextPrivate* p =
      OxideQQuickWebViewContextPrivate::get(context);

  while (p->user_scripts().size() > 0) {
    OxideQUserScript* script = p->user_scripts().first();
    p->user_scripts().removeFirst();
    QObject::disconnect(script, SIGNAL(scriptLoaded()),
                        context, SLOT(scriptUpdated()));
    QObject::disconnect(script, SIGNAL(scriptPropertyChanged()),
                        context, SLOT(scriptUpdated()));
    delete script;
  }

  p->updateUserScripts();
}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(bool is_default) :
    oxide::qt::WebViewContext(*new OxideQQuickWebViewContextPrivate(this)) {
  if (is_default) {
    Q_ASSERT(!g_default_context);
    g_default_context = this;
  }
}

OxideQQuickWebViewContext::OxideQQuickWebViewContext(QObject* parent) :
    oxide::qt::WebViewContext(*new OxideQQuickWebViewContextPrivate(this),
                              parent) {}

OxideQQuickWebViewContext::~OxideQQuickWebViewContext() {
  if (g_default_context == this) {
    g_default_context = NULL;
  }
}

// static
OxideQQuickWebViewContext* OxideQQuickWebViewContext::defaultContext() {
  if (g_default_context) {
    return g_default_context;
  }

  return new OxideQQuickWebViewContext(true);
}

QQmlListProperty<OxideQUserScript>
OxideQQuickWebViewContext::userScripts() {
  return QQmlListProperty<OxideQUserScript>(
      this, NULL,
      OxideQQuickWebViewContextPrivate::userScript_append,
      OxideQQuickWebViewContextPrivate::userScript_count,
      OxideQQuickWebViewContextPrivate::userScript_at,
      OxideQQuickWebViewContextPrivate::userScript_clear);
}

