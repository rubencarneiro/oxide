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
#include <QtQml>
#include <QVariant>
#include <QVariantList>
#include <signal.h>

QT_BEGIN_NAMESPACE
class QQuickItem;
class QScreen;
QT_END_NAMESPACE

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

  Q_INVOKABLE bool hasImage() const;

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

class WebContextTestSupportAttached : public QObject {
  Q_OBJECT

 public:
  WebContextTestSupportAttached(QObject* attachee);

  Q_INVOKABLE void clearTemporarySavedPermissionStatuses();

 private:
  OxideQQuickWebContext* context_;
};

class WebContextTestSupport : public QObject {
  Q_OBJECT

 public:
  static WebContextTestSupportAttached* qmlAttachedProperties(QObject* attachee);
};

QML_DECLARE_TYPE(WebContextTestSupport)
QML_DECLARE_TYPEINFO(WebContextTestSupport, QML_HAS_ATTACHED_PROPERTIES)

class ItemTestSupportAttached : public QObject {
  Q_OBJECT

 public:
  ItemTestSupportAttached(QObject* attachee);

  Q_INVOKABLE QPointF mapToScene(const QPointF& point) const;

 private:
  QQuickItem* item_;
};

class ItemTestSupport : public QObject {
  Q_OBJECT

 public:
  static ItemTestSupportAttached* qmlAttachedProperties(QObject* attachee);
};

QML_DECLARE_TYPE(ItemTestSupport)
QML_DECLARE_TYPEINFO(ItemTestSupport, QML_HAS_ATTACHED_PROPERTIES)

// This exists because the Window.window attached property does not exist
// until Qt5.7, and the other attached Window properties available in
// QtQuick.Window don't provide the functionality we require
class TestWindowAttached : public QObject {
  Q_OBJECT
  Q_PROPERTY(int x READ x)
  Q_PROPERTY(int y READ y)
  Q_PROPERTY(int width READ width)
  Q_PROPERTY(int height READ height)
  Q_PROPERTY(QScreen* screen READ screen WRITE setScreen)
  Q_PROPERTY(QQuickItem* rootItem READ rootItem)

 public:
  TestWindowAttached(QObject* attachee);

  int x() const;
  int y() const;
  int width() const;
  int height() const;

  QScreen* screen() const;
  void setScreen(QScreen* screen);

  QQuickItem* rootItem() const;

 private:
  QQuickItem* item_;
};

class TestWindow : public QObject {
  Q_OBJECT

 public:
  static TestWindowAttached* qmlAttachedProperties(QObject* attachee);
};

QML_DECLARE_TYPE(TestWindow)
QML_DECLARE_TYPEINFO(TestWindow, QML_HAS_ATTACHED_PROPERTIES)

class TestSupport : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool testLoaded READ testLoaded NOTIFY testLoadedChanged)
  Q_PROPERTY(QVariantList screens READ screens)

 public:
  TestSupport();

  static TestSupport* instance();

  void reset();

  bool testLoaded() const;
  void setTestLoaded(bool loaded);

  QVariantList screens() const;

  Q_INVOKABLE QObject* qObjectParent(QObject* object);

  Q_INVOKABLE void destroyQObjectNow(QObject* object);

  Q_INVOKABLE QObjectTestHelper* createQObjectTestHelper(QObject* object);

  Q_INVOKABLE QVariant getAppProperty(const QString& property);
  Q_INVOKABLE void setAppProperty(const QString& property,
                                  const QVariant& value);
  Q_INVOKABLE void removeAppProperty(const QString& property);

  Q_INVOKABLE void wait(int ms);

  Q_INVOKABLE QVariant toQtPixels(QQuickItem* item, const QVariant& v);

  Q_INVOKABLE QQuickItem* findItemInScene(QQuickItem* root,
                                          const QString& name);

  Q_INVOKABLE QVariantList findItemsInScene(QQuickItem* root,
                                            const QString& namePrefix);

 Q_SIGNALS:
  void testLoadedChanged();

 private:
  bool test_loaded_;
};

#endif // _OXIDE_QT_TESTS_QMLTEST_QML_TEST_SUPPORT_H_
