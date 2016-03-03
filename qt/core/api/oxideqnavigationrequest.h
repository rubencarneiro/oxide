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

#ifndef OXIDE_QTCORE_NAVIGATION_REQUEST
#define OXIDE_QTCORE_NAVIGATION_REQUEST

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>

#include <OxideQtCore/oxideqglobal.h>

#if defined(OXIDE_QTCORE_IMPLEMENTATION)
namespace oxide {
namespace qt {
class WebView;
}
}
#endif

class OxideQNavigationRequestPrivate;

class OXIDE_QTCORE_EXPORT OxideQNavigationRequest : public QObject {
  Q_OBJECT

  Q_PROPERTY(QUrl url READ url CONSTANT)
  Q_PROPERTY(Disposition disposition READ disposition CONSTANT)
  Q_PROPERTY(bool userGesture READ userGesture CONSTANT)
  Q_PROPERTY(Action action READ action WRITE setAction NOTIFY actionChanged)

  Q_ENUMS(Disposition)
  Q_ENUMS(Action)

  Q_DECLARE_PRIVATE(OxideQNavigationRequest)
  Q_DISABLE_COPY(OxideQNavigationRequest)

 public:

  enum Disposition {
    DispositionCurrentTab,
    DispositionNewForegroundTab,
    DispositionNewBackgroundTab,
    DispositionNewPopup,
    DispositionNewWindow
  };

  enum Action {
    ActionAccept,
    ActionReject = 0xFF
  };

  ~OxideQNavigationRequest() Q_DECL_OVERRIDE;

  QUrl url() const;
  Disposition disposition() const;
  bool userGesture() const;

  Action action() const;
  void setAction(Action action);

 Q_SIGNALS:
  void actionChanged();

 private:
#if defined(OXIDE_QTCORE_IMPLEMENTATION)
  friend class oxide::qt::WebView;
#endif
  Q_DECL_HIDDEN OxideQNavigationRequest(const QUrl& url,
                                        Disposition disposition,
                                        bool user_gesture);

  QScopedPointer<OxideQNavigationRequestPrivate> d_ptr;
};

#endif // OXIDE_QTCORE_NAVIGATION_REQUEST
