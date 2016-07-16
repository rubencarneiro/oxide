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

#include "qml_test_support.h"

#include <QClipboard>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QLatin1String>
#include <QList>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QString>
#include <QtGlobal>
#include <QtTest>
#include <QtQml>
#include <QVariant>

#include "qt/quick/api/oxideqquickwebcontext.h"
#include "qt/quick/api/oxideqquickwebcontext_p.h"
#include "qt/quick/api/oxideqquickwebview.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

ExternalProtocolHandler::ExternalProtocolHandler()
    : locked_(false) {}

ExternalProtocolHandler::~ExternalProtocolHandler() {
  if (!scheme_.isEmpty()) {
    QDesktopServices::unsetUrlHandler(scheme_);
  }
}

void ExternalProtocolHandler::classBegin() {}

void ExternalProtocolHandler::componentComplete() {
  locked_ = true;
  if (!scheme_.isEmpty()) {
    QDesktopServices::setUrlHandler(scheme_, this, "openUrl");
  }
}

QString ExternalProtocolHandler::scheme() const {
  return scheme_;
}

void ExternalProtocolHandler::setScheme(const QString& scheme) {
  if (locked_) {
    return;
  }

  scheme_ = scheme;
  Q_EMIT schemeChanged();
}

ClipboardTestUtils::ClipboardTestUtils() {}

bool ClipboardTestUtils::hasImage() const {
  return QGuiApplication::clipboard()->mimeData()->hasImage();
}

void ClipboardTestUtils::copyToClipboard(const QString& mimeType,
                                         const QString& data) {
  QMimeData * mime_data = new QMimeData();
  if (mimeType.startsWith("image/")) {
    mime_data->setData(mimeType, QByteArray::fromBase64(data.toUtf8()));
  } else {
    mime_data->setData(mimeType, data.toUtf8());
  }
  QGuiApplication::clipboard()->setMimeData(mime_data);
}

QString ClipboardTestUtils::getFromClipboard(const QString& mimeType) {
  const QMimeData * mime_data = QGuiApplication::clipboard()->mimeData();
  if (mime_data->hasFormat(mimeType)) {
    return QString(mime_data->data(mimeType));
  }
  return QString();
}

void ClipboardTestUtils::clearClipboard() {
  QGuiApplication::clipboard()->clear();
}

bool QObjectTestHelper::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::ChildRemoved &&
      static_cast<QChildEvent*>(event)->child() == object_) {
    Q_ASSERT(watched == parent_);
    parent_ = nullptr;
    Q_EMIT parentChanged();
  } else if (event->type() == QEvent::ChildAdded &&
             static_cast<QChildEvent*>(event)->child() == object_) {
    Q_ASSERT(!parent_);
    parent_ = object_->parent();
    Q_EMIT parentChanged();
  }
}

void QObjectTestHelper::onDestroyed() {
  Q_ASSERT(!destroyed_);

  destroyed_ = true;
  Q_EMIT destroyedChanged();
}

QObjectTestHelper::QObjectTestHelper(QObject* object)
    : object_(object),
      parent_(object ? object->parent() : nullptr),
      destroyed_(false) {
  Q_ASSERT(object);

  connect(object, SIGNAL(destroyed()), this, SLOT(onDestroyed()));
  QCoreApplication::instance()->installEventFilter(this);
}

QObjectTestHelper::~QObjectTestHelper() {
  QCoreApplication::instance()->removeEventFilter(this);
}

bool QObjectTestHelper::destroyed() const {
  return destroyed_;
}

QObject* QObjectTestHelper::parent() const {
  if (destroyed_) {
    return nullptr;
  }

  return parent_;
}

WebContextTestSupportAttached::WebContextTestSupportAttached(
    QObject* attachee)
    : QObject(attachee),
      context_(qobject_cast<OxideQQuickWebContext*>(attachee)) {}

void WebContextTestSupportAttached::clearTemporarySavedPermissionStatuses() {
  if (!context_) {
    qWarning() << "Object is not a WebContext";
    return;
  }

  OxideQQuickWebContextPrivate::get(context_)
      ->clearTemporarySavedPermissionStatuses();
}

// static
WebContextTestSupportAttached* WebContextTestSupport::qmlAttachedProperties(
    QObject* attachee) {
  return new WebContextTestSupportAttached(attachee);
}

WebViewTestSupportAttached::WebViewTestSupportAttached(QObject* attachee)
    : QObject(attachee),
      view_(qobject_cast<OxideQQuickWebView*>(attachee)) {}

void WebViewTestSupportAttached::killWebProcess(bool crash) {
  if (!view_) {
    qWarning() << "Object is not a WebView";
    return;
  }

  OxideQQuickWebViewPrivate::get(view_)->killWebProcess(crash);
}

// static
WebViewTestSupportAttached* WebViewTestSupport::qmlAttachedProperties(
    QObject* attachee) {
  return new WebViewTestSupportAttached(attachee);
}

TestWindowAttached::TestWindowAttached(QObject* attachee)
    : QObject(attachee),
      item_(nullptr) {
  QObject* o = attachee;
  while (o) {
    item_ = qobject_cast<QQuickItem*>(o);
    if (item_) {
      break;
    }
    o = o->parent();
  }
  if (!item_) {
    qWarning() << "Can't determine item from object";
  }
}

int TestWindowAttached::x() const {
  if (!item_ || !item_->window()) {
    return 0;
  }

  return item_->window()->x();
}

int TestWindowAttached::y() const {
  if (!item_ || !item_->window()) {
    return 0;
  }

  return item_->window()->y();
}

// static
TestWindowAttached* TestWindow::qmlAttachedProperties(QObject* attachee) {
  return new TestWindowAttached(attachee);
}

ItemTestSupportAttached::ItemTestSupportAttached(QObject* attachee)
    : QObject(attachee),
      item_(nullptr) {
  QObject* o = attachee;
  while (o) {
    item_ = qobject_cast<QQuickItem*>(o);
    if (item_) {
      break;
    }
    o = o->parent();
  }
  if (!item_) {
    qWarning() << "Can't determine item from object";
  }
}

QPointF ItemTestSupportAttached::mapToScene(const QPointF& point) const {
  if (!item_) {
    return QPointF();
  }

  return item_->mapToScene(point);
}

// static
ItemTestSupportAttached* ItemTestSupport::qmlAttachedProperties(QObject* attachee) {
  return new ItemTestSupportAttached(attachee);
}

TestSupport::TestSupport() {}

QObject* TestSupport::qObjectParent(QObject* object) {
  if (!object) {
    return nullptr;
  }

  return object->parent();
}

void TestSupport::destroyQObjectNow(QObject* object) {
  if (!object) {
    return;
  }

  delete object;
}

QObjectTestHelper* TestSupport::createQObjectTestHelper(QObject* object) {
  if (!object) {
    return nullptr;
  }

  return new QObjectTestHelper(object);
}

QVariant TestSupport::getAppProperty(const QString& property) {
  QVariant rv =
      QCoreApplication::instance()->property(property.toStdString().c_str());
  if (QObject* qobject = rv.value<QObject*>()) {
    QQmlEngine::setObjectOwnership(qobject, QQmlEngine::CppOwnership);
  }
  return rv;
}

void TestSupport::setAppProperty(const QString& property,
                                 const QVariant& value) {
  QCoreApplication::instance()->setProperty(property.toStdString().c_str(),
                                            value);
}

void TestSupport::removeAppProperty(const QString& property) {
  QCoreApplication::instance()->setProperty(property.toStdString().c_str(),
                                            QVariant());
}

void TestSupport::wait(int ms) {
  QTest::qWait(ms);
}
