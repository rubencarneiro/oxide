// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#ifndef _OXIDE_QT_QUICK_API_GLOBAL_P_H_
#define _OXIDE_QT_QUICK_API_GLOBAL_P_H_

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QVariant>

class OxideQQuickGlobalPrivate;
class OxideQQuickWebContext;

class Q_DECL_EXPORT OxideQQuickGlobal : public QObject {
  Q_OBJECT

  Q_PROPERTY(ProcessModel processModel READ processModel WRITE setProcessModel NOTIFY processModelChanged)
  Q_PROPERTY(int maxRendererProcessCount READ maxRendererProcessCount WRITE setMaxRendererProcessCount NOTIFY maxRendererProcessCountChanged)

  Q_ENUMS(ProcessModel)

  Q_DECLARE_PRIVATE(OxideQQuickGlobal)
  Q_DISABLE_COPY(OxideQQuickGlobal)

 public:

  enum ProcessModel {
    ProcessModelMultiProcess,
    ProcessModelSingleProcess,

    ProcessModelProcessPerSiteInstance,
    ProcessModelProcessPerView,
    ProcessModelProcessPerSite,
    ProcessModelSitePerProcess
  };

  OxideQQuickGlobal();
  virtual ~OxideQQuickGlobal();

  ProcessModel processModel() const;
  void setProcessModel(ProcessModel model);

  int maxRendererProcessCount() const;
  void setMaxRendererProcessCount(int count);

  Q_INVOKABLE OxideQQuickWebContext* defaultWebContext();

  Q_INVOKABLE QVariant availableAudioCaptureDevices();
  Q_INVOKABLE QVariant availableVideoCaptureDevices();

 Q_SIGNALS:
  void processModelChanged();
  void maxRendererProcessCountChanged();

 private:
  QScopedPointer<OxideQQuickGlobalPrivate> d_ptr;
};

#endif // _OXIDE_QT_QUICK_API_GLOBAL_P_H_
