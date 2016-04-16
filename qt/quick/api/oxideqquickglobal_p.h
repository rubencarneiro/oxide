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

#ifndef _OXIDE_QT_QUICK_API_GLOBAL_P_H_
#define _OXIDE_QT_QUICK_API_GLOBAL_P_H_

#include <QObject>
#include <QScopedPointer>
#include <QtGlobal>
#include <QVariant>

#include "qt/quick/api/oxideqquickglobal.h"

class OxideQQuickGlobalPrivate;
class OxideQQuickWebContext;

class OXIDE_QTQUICK_EXPORT OxideQQuickGlobal : public QObject {
  Q_OBJECT

  Q_PROPERTY(ProcessModel processModel READ processModel WRITE setProcessModel NOTIFY processModelChanged)
  Q_PROPERTY(int maxRendererProcessCount READ maxRendererProcessCount WRITE setMaxRendererProcessCount NOTIFY maxRendererProcessCountChanged)

  Q_PROPERTY(QVariant availableAudioCaptureDevices READ availableAudioCaptureDevices NOTIFY availableAudioCaptureDevicesChanged)
  Q_PROPERTY(QVariant availableVideoCaptureDevices READ availableVideoCaptureDevices NOTIFY availableVideoCaptureDevicesChanged)

  Q_PROPERTY(QString chromiumVersion READ chromiumVersion CONSTANT)
  Q_PROPERTY(QString version READ oxideVersion CONSTANT)

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
  ~OxideQQuickGlobal() Q_DECL_OVERRIDE;

  ProcessModel processModel() const;
  void setProcessModel(ProcessModel model);

  int maxRendererProcessCount() const;
  void setMaxRendererProcessCount(int count);

  QVariant availableAudioCaptureDevices();
  QVariant availableVideoCaptureDevices();

  QString chromiumVersion() const;
  QString oxideVersion() const;

  Q_INVOKABLE OxideQQuickWebContext* defaultWebContext();

 Q_SIGNALS:
  void processModelChanged();
  void maxRendererProcessCountChanged();
  void availableAudioCaptureDevicesChanged();
  void availableVideoCaptureDevicesChanged();

 private:
  Q_PRIVATE_SLOT(d_func(), void availableAudioCaptureDevicesDidChange());
  Q_PRIVATE_SLOT(d_func(), void availableVideoCaptureDevicesDidChange());

  QScopedPointer<OxideQQuickGlobalPrivate> d_ptr;
};

#endif // _OXIDE_QT_QUICK_API_GLOBAL_P_H_
