// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef OXIDE_Q_GLOBAL
#define OXIDE_Q_GLOBAL

#include <QString>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QThread;
QT_END_NAMESPACE

enum Q_DECL_EXPORT OxideProcessModel {
  OxideProcessModelMultiProcess, // Uses the default multiprocess model
  OxideProcessModelSingleProcess,

  // Unsupported process models - these could change or go away at any
  // time (the enums won't change, but the behaviour might), so applications
  // shouldn't rely on these by default. They exist so that applications
  // could, eg, expose a command-line interface to enable them in the same
  // way that Chrome provides
  OxideProcessModelProcessPerSiteInstance,
  OxideProcessModelProcessPerView,
  OxideProcessModelProcessPerSite,
  OxideProcessModelSitePerProcess
};

Q_DECL_EXPORT QString oxideGetNSSDbPath();
Q_DECL_EXPORT void oxideSetNSSDbPath(const QString& path);

Q_DECL_EXPORT QThread* oxideGetIOThread();

Q_DECL_EXPORT OxideProcessModel oxideGetProcessModel();
Q_DECL_EXPORT void oxideSetProcessModel(OxideProcessModel model);

Q_DECL_EXPORT size_t oxideGetMaxRendererProcessCount();
Q_DECL_EXPORT void oxideSetMaxRendererProcessCount(size_t count);

#endif // OXIDE_Q_GLOBAL
