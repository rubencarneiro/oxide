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

#include <queue>

#include <QClipboard>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QJSValue>
#include <QLatin1String>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWindow>
#include <QScreen>
#include <QString>
#include <QtGlobal>
#include <QtTest>
#include <QtQml>
#include <QVariant>

#include "qt/core/glue/screen_utils.h"
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

int TestWindowAttached::width() const {
  if (!item_ || !item_->window()) {
    return 0;
  }

  return item_->window()->width();
}

int TestWindowAttached::height() const {
  if (!item_ || !item_->window()) {
    return 0;
  }

  return item_->window()->height();
}

QScreen* TestWindowAttached::screen() const {
  if (!item_ || !item_->window()) {
    return nullptr;
  }

  return item_->window()->screen();
}

void TestWindowAttached::setScreen(QScreen* screen) {
  if (!item_ || !item_->window()) {
    qWarning() << "Can't set screen on item with no window";
    return;
  }

  item_->window()->setScreen(screen);
}

QQuickItem* TestWindowAttached::rootItem() const {
  if (!item_ || !item_->window()) {
    return nullptr;
  }

  QQuickView* view = qobject_cast<QQuickView*>(item_->window());
  if (view) {
    return view->rootObject();
  }

  QQuickItem* root = item_;
  while (root->parentItem()) {
    root = root->parentItem();
  }

  Q_ASSERT(root == item_->window()->contentItem());
  return root->childItems()[0];
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

TestSupport::TestSupport()
    : test_loaded_(false) {}

// static
TestSupport* TestSupport::instance() {
  static QPointer<TestSupport> object = new TestSupport();
  Q_ASSERT(object);
  return object;
}

void TestSupport::reset() {
  test_loaded_ = false;
}

bool TestSupport::testLoaded() const {
  return test_loaded_;
}

void TestSupport::setTestLoaded(bool loaded) {
  test_loaded_ = loaded;
  Q_EMIT testLoadedChanged();
}

QVariantList TestSupport::screens() const {
  QVariantList rv;
  for (auto screen : QGuiApplication::screens()) {
    rv.push_back(QVariant::fromValue(screen));
  }
  return rv;
}

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

QVariant TestSupport::toQtPixels(QQuickItem* item, const QVariant& v) {
  if (!item->window() || !item->window()->screen()) {
    qWarning() << "Can't determine scale factor for item";
    return v;
  }

  float scale =
      oxide::qt::GetScreenScaleFactor(item->window()->screen()) /
      item->window()->screen()->devicePixelRatio();

  QVariant value = v;
  if (value.userType() == qMetaTypeId<QJSValue>()) {
    value = value.value<QJSValue>().toVariant();
  }

  if (value.type() != QVariant::Map) {
    qWarning() << "Invalid type";
    return QVariant();
  }

  QMap<QString, QVariant> map = value.toMap();
  if (map.contains("x") && map.contains("y")) {
    map["x"] = map["x"].toReal() * scale;
    map["y"] = map["y"].toReal() * scale;
  }

  if (map.contains("width") && map.contains("height")) {
    map["width"] = map["width"].toReal() * scale;
    map["height"] = map["height"].toReal() * scale;
  }

  return map;
}

QQuickItem* TestSupport::findItemInScene(QQuickItem* root,
                                         const QString& name) {
  if (!root) {
    qWarning() << "No root item specified";
    return nullptr;
  }

  std::queue<QQuickItem*> stack;
  stack.push(root);

  while (!stack.empty()) {
    QQuickItem* i = stack.front();
    stack.pop();

    if (i->objectName() == name) {
      return i;
    }

    for (auto* child : i->childItems()) {
      stack.push(child);
    }
  }

  return nullptr;
}

QVariantList TestSupport::findItemsInScene(QQuickItem* root,
                                           const QString& namePrefix) {
  QVariantList rv;

  if (!root) {
    qWarning() << "No root item specified";
    return rv;
  }

  std::queue<QQuickItem*> stack;
  stack.push(root);

  while (!stack.empty()) {
    QQuickItem* i = stack.front();
    stack.pop();

    if (i->objectName().startsWith(namePrefix)) {
      rv.push_back(QVariant::fromValue(i));
    }

    for (auto* child : i->childItems()) {
      stack.push(child);
    }
  }

  return rv;
}
