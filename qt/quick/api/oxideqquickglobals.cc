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

#include "oxideqquickglobals_p.h"

#include <QCoreApplication>

#include "oxideqquickwebcontext_p.h"

namespace {
class OxideQQuickGlobals* g_instance;
}

class OxideQQuickGlobalsPrivate {
 public:
  ~OxideQQuickGlobalsPrivate() {}
  OxideQQuickGlobalsPrivate() {}
};

OxideQQuickGlobals::OxideQQuickGlobals() :
    QObject(QCoreApplication::instance()),
    d_ptr(new OxideQQuickGlobalsPrivate()) {}

// static
OxideQQuickGlobals* OxideQQuickGlobals::instance() {
  if (!g_instance) {
    g_instance = new OxideQQuickGlobals();
  }

  Q_ASSERT(g_instance);

  return g_instance;
}

OxideQQuickGlobals::~OxideQQuickGlobals() {
  Q_ASSERT(this == g_instance);
  g_instance = NULL;
}

OxideQQuickWebContext* OxideQQuickGlobals::defaultWebContext() {
  return OxideQQuickWebContext::defaultContext(true);
}
