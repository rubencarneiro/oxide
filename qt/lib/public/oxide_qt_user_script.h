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

#ifndef _OXIDE_QT_LIB_PUBLIC_USER_SCRIPT_H_
#define _OXIDE_QT_LIB_PUBLIC_USER_SCRIPT_H_

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QUrl>

#include "shared/common/oxide_export.h"

namespace oxide {
namespace qt {

class UserScriptPrivate;

class OXIDE_EXPORT UserScript : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url WRITE setUrl)
  Q_PROPERTY(bool emulateGreasemonkey READ emulateGreasemonkey WRITE setEmulateGreasemonkey NOTIFY scriptPropertyChanged)
  Q_PROPERTY(bool matchAllFrames READ matchAllFrames WRITE setMatchAllFrames NOTIFY scriptPropertyChanged)
  Q_PROPERTY(bool incognitoEnabled READ incognitoEnabled WRITE setIncognitoEnabled NOTIFY scriptPropertyChanged)
  Q_PROPERTY(QString worldId READ worldId WRITE setWorldId NOTIFY scriptPropertyChanged)

  Q_DECLARE_PRIVATE(UserScript)

 public:

  enum State {
    Constructing,
    Loading,
    Ready,
    Failed
  };

  UserScript(UserScriptPrivate& dd, QObject* parent = NULL);
  virtual ~UserScript();

  void startLoading();

  State state() const;

  QUrl url() const;
  void setUrl(const QUrl& url);

  bool emulateGreasemonkey() const;
  void setEmulateGreasemonkey(bool emulate_greasemonkey);

  bool matchAllFrames() const;
  void setMatchAllFrames(bool match_all_frames);

  bool incognitoEnabled() const;
  void setIncognitoEnabled(bool incognito_enabled);

  QString worldId() const;
  void setWorldId(const QString& world_id);

 Q_SIGNALS:
  void scriptLoaded();
  void scriptLoadFailed();
  void scriptPropertyChanged();

 protected:
  QScopedPointer<UserScriptPrivate> d_ptr;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_PUBLIC_USER_SCRIPT_H_
