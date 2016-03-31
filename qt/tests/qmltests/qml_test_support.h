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
#include <QPointer>
#include <QQmlParserStatus>
#include <QString>
#include <QVariant>
#include <signal.h>

class OxideQQuickWebContext;
class OxideQQuickWebView;

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

class ClipboardTestUtils : public QObject {
  Q_OBJECT

 public:
  ClipboardTestUtils();

  Q_INVOKABLE void copyToClipboard(const QString& mimeType,
                                   const QString& data);

  Q_INVOKABLE QString getFromClipboard(const QString& mimeType);

  Q_INVOKABLE void clearClipboard();
};

class QObjectTestHelper : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool destroyed READ destroyed NOTIFY destroyedChanged)
  Q_PROPERTY(QObject* parent READ parent NOTIFY parentChanged)

 public:
  QObjectTestHelper(QObject* object);
  ~QObjectTestHelper() override;

  bool destroyed() const;

  QObject* parent() const;

 private:
  bool eventFilter(QObject* watched, QEvent* event) override;

 Q_SIGNALS:
  void destroyedChanged();
  void parentChanged();

 private Q_SLOTS:
  void onDestroyed();

 private:
  QPointer<QObject> object_;
  QPointer<QObject> parent_;
  bool destroyed_;
};

class WebContextTestSupport : public QObject {
  Q_OBJECT

 public:
  WebContextTestSupport(OxideQQuickWebContext* context);

  Q_INVOKABLE void clearTemporarySavedPermissionStatuses();

 private:
  QPointer<OxideQQuickWebContext> context_;
};

class WebViewTestSupport : public QObject {
  Q_OBJECT

 public:
  WebViewTestSupport(OxideQQuickWebView* view);

  Q_INVOKABLE void killWebProcess(bool crash);

 private:
  QPointer<OxideQQuickWebView> view_;
};

class TestSupport : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool skipTestCase READ skipTestCase WRITE setSkipTestCase NOTIFY skipTestCaseChanged)

 public:
  TestSupport();

  static TestSupport* instance();

  bool skipTestCase() const;
  void setSkipTestCase(bool skip);

  void reset();

  Q_INVOKABLE QObject* qObjectParent(QObject* object);

  Q_INVOKABLE void destroyQObjectNow(QObject* object);

  Q_INVOKABLE QObjectTestHelper* createQObjectTestHelper(QObject* object);

  Q_INVOKABLE WebContextTestSupport* createWebContextTestSupport(
      OxideQQuickWebContext* context);

  Q_INVOKABLE WebViewTestSupport* createWebViewTestSupport(
      OxideQQuickWebView* view);

  Q_INVOKABLE QVariant getAppProperty(const QString& property);
  Q_INVOKABLE void setAppProperty(const QString& property,
                                  const QVariant& value);
  Q_INVOKABLE void removeAppProperty(const QString& property);

  Q_INVOKABLE void wait(int ms);

 Q_SIGNALS:
  void skipTestCaseChanged();

 private:
  bool skip_test_case_;
};

#endif // _OXIDE_QT_TESTS_QMLTEST_QML_TEST_SUPPORT_H_
