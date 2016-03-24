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

#ifndef _OXIDE_QT_TESTS_QMLTEST_QML_TEST_SUPPORT_H_
#define _OXIDE_QT_TESTS_QMLTEST_QML_TEST_SUPPORT_H_

#include <QObject>
#include <QQmlParserStatus>
#include <QString>
#include <QVariant>
#include <signal.h>

class OxideQQuickWebContext;

class ExternalProtocolHandler : public QObject,
                                public QQmlParserStatus {
  Q_OBJECT
  Q_PROPERTY(QString scheme READ scheme WRITE setScheme NOTIFY schemeChanged)

 public:
  ExternalProtocolHandler();
  ~ExternalProtocolHandler() override;

  void classBegin() override;
  void componentComplete() override;

  QString scheme() const;
  void setScheme(const QString& scheme);

 Q_SIGNALS:
  void schemeChanged();
  void openUrl(const QUrl& url);

 private:
  bool locked_;
  QString scheme_;
};

class DestructionObserver : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool destroyed READ destroyed NOTIFY destroyedChanged)

 public:
  DestructionObserver(QObject* object);
  ~DestructionObserver() override;

  bool destroyed() const;

 Q_SIGNALS:
  void destroyedChanged();

 private Q_SLOTS:
  void onDestroyed();

 private:
  bool destroyed_;
};

class OxideTestingUtils : public QObject {
  Q_OBJECT

 public:
  OxideTestingUtils();

  Q_INVOKABLE QObject* qObjectParent(QObject* object);

  Q_INVOKABLE void destroyQObjectNow(QObject* object);

  Q_INVOKABLE DestructionObserver* createDestructionObserver(QObject* object);

  Q_INVOKABLE QVariant getAppProperty(const QString& property);
  Q_INVOKABLE void setAppProperty(const QString& property,
                                  const QVariant& value);
  Q_INVOKABLE void removeAppProperty(const QString& property);

  Q_INVOKABLE void killWebProcesses(uint signal=SIGKILL);

  Q_INVOKABLE void copyToClipboard(const QString& mimeType,
                                   const QString& data);
  Q_INVOKABLE QString getFromClipboard(const QString& mimeType);
  Q_INVOKABLE void clearClipboard();

  Q_INVOKABLE void clearTemporarySavedPermissionStatuses(
      OxideQQuickWebContext* context);

  Q_INVOKABLE void wait(int ms);
};
#endif // _OXIDE_QT_TESTS_QMLTEST_QML_TEST_SUPPORT_H_
