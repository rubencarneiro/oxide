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

#ifndef OXIDE_QTQUICK_USER_SCRIPT
#define OXIDE_QTQUICK_USER_SCRIPT

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QUrl>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QtQml>

#include <OxideQtQuick/oxideqquickglobal.h>

class OxideQQuickUserScriptPrivate;

class OXIDE_QTQUICK_EXPORT OxideQQuickUserScript : public QObject,
                                                   public QQmlParserStatus {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY scriptPropertyChanged)
  Q_PROPERTY(bool emulateGreasemonkey READ emulateGreasemonkey WRITE setEmulateGreasemonkey NOTIFY scriptPropertyChanged)
  Q_PROPERTY(bool matchAllFrames READ matchAllFrames WRITE setMatchAllFrames NOTIFY scriptPropertyChanged)
  Q_PROPERTY(bool incognitoEnabled READ incognitoEnabled WRITE setIncognitoEnabled NOTIFY scriptPropertyChanged)
  Q_PROPERTY(QUrl context READ context WRITE setContext NOTIFY scriptPropertyChanged)

  Q_DECLARE_PRIVATE(OxideQQuickUserScript)
  Q_DISABLE_COPY(OxideQQuickUserScript)

  Q_INTERFACES(QQmlParserStatus)

 public:
  OxideQQuickUserScript(QObject* parent = nullptr);
  ~OxideQQuickUserScript() Q_DECL_OVERRIDE;

  QUrl url() const;
  void setUrl(const QUrl& url);

  bool emulateGreasemonkey() const;
  void setEmulateGreasemonkey(bool emulate_greasemonkey);

  bool matchAllFrames() const;
  void setMatchAllFrames(bool match_all_frames);

  bool incognitoEnabled() const;
  void setIncognitoEnabled(bool incognito_enabled);

  QUrl context() const;
  void setContext(const QUrl& context);

 Q_SIGNALS:
  void scriptLoaded();
  void scriptLoadFailed();
  void scriptPropertyChanged();

 protected:
  // QQmlParserStatus implementation
  void classBegin() Q_DECL_OVERRIDE;
  void componentComplete() Q_DECL_OVERRIDE;

 private:
  QScopedPointer<OxideQQuickUserScriptPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickUserScript)

#endif // OXIDE_QTQUICK_USER_SCRIPT
