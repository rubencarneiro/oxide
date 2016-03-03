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

#ifndef OXIDE_QTCORE_GLOBAL
#define OXIDE_QTCORE_GLOBAL

#include <QtCore/QString>
#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE
class QThread;
QT_END_NAMESPACE

#if defined(OXIDE_QTCORE_IMPLEMENTATION)
# define OXIDE_QTCORE_EXPORT Q_DECL_EXPORT
#else
# define OXIDE_QTCORE_EXPORT Q_DECL_IMPORT
#endif

enum OXIDE_QTCORE_EXPORT OxideProcessModel {
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

OXIDE_QTCORE_EXPORT QString oxideGetNSSDbPath();
OXIDE_QTCORE_EXPORT void oxideSetNSSDbPath(const QString& path);

OXIDE_QTCORE_EXPORT QThread* oxideGetIOThread();

OXIDE_QTCORE_EXPORT OxideProcessModel oxideGetProcessModel();
OXIDE_QTCORE_EXPORT void oxideSetProcessModel(OxideProcessModel model);

OXIDE_QTCORE_EXPORT size_t oxideGetMaxRendererProcessCount();
OXIDE_QTCORE_EXPORT void oxideSetMaxRendererProcessCount(size_t count);

#endif // OXIDE_QTCORe_GLOBAL
