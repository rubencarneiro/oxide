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

#include "qml_test_support.h"

#include <sys/types.h>
#include <unistd.h>

#include <QClipboard>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QLatin1String>
#include <QList>
#include <QQmlContext>
#include <QProcess>
#include <QString>
#include <QtGlobal>
#include <QtTest>
#include <QtQml>
#include <QVariant>

#include "qt/quick/api/oxideqquickwebcontext.h"
#include "qt/quick/api/oxideqquickwebcontext_p.h"

namespace {

QList<pid_t> getChildProcesses(pid_t pid) {
  QProcess pgrep;
  pgrep.start(QString("pgrep --parent %1").arg(pid));
  pgrep.waitForFinished();
  QList<QByteArray> output = pgrep.readAllStandardOutput().split('\n');
  QList<pid_t> children;
  Q_FOREACH (const QByteArray& child, output) {
    if (!child.isEmpty()) {
      children.append(child.toInt());
    }
  }
  return children;
}

QList<pid_t> getDescendantProcesses(pid_t pid) {
  QList<pid_t> descendants = getChildProcesses(pid);
  Q_FOREACH (pid_t descendant, descendants) {
    descendants << getDescendantProcesses(descendant);
  }
  return descendants;
}

} // namespace

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

WebContextTestSupport::WebContextTestSupport(OxideQQuickWebContext* context)
    : context_(context) {}

void WebContextTestSupport::clearTemporarySavedPermissionStatuses() {
  if (!context_) {
    qWarning() << "Associated context has already been deleted";
    return;
  }

  OxideQQuickWebContextPrivate::get(context_)
      ->clearTemporarySavedPermissionStatuses();
}

OxideTestingUtils::OxideTestingUtils() {}

QObject* OxideTestingUtils::qObjectParent(QObject* object) {
  if (!object) {
    return nullptr;
  }

  return object->parent();
}

void OxideTestingUtils::destroyQObjectNow(QObject* object) {
  if (!object) {
    return;
  }

  delete object;
}

QObjectTestHelper* OxideTestingUtils::createQObjectTestHelper(QObject* object) {
  if (!object) {
    return nullptr;
  }

  return new QObjectTestHelper(object);
}

WebContextTestSupport* OxideTestingUtils::createWebContextTestSupport(
    OxideQQuickWebContext* context) {
  if (!context) {
    qWarning() << "NULL WebContext";
    return nullptr;
  }

  return new WebContextTestSupport(context);
}

QVariant OxideTestingUtils::getAppProperty(const QString& property) {
  return QCoreApplication::instance()->property(property.toStdString().c_str());
}

void OxideTestingUtils::setAppProperty(const QString& property,
                                       const QVariant& value) {
  QCoreApplication::instance()->setProperty(property.toStdString().c_str(),
                                            value);
}

void OxideTestingUtils::removeAppProperty(const QString& property) {
  QCoreApplication::instance()->setProperty(property.toStdString().c_str(),
                                            QVariant());
}

void OxideTestingUtils::killWebProcesses(uint signal) {
  Q_FOREACH (pid_t descendant, getDescendantProcesses(getpid())) {
    QProcess ps;
    ps.start(QString("ps fhp %1").arg(descendant));
    ps.waitForFinished();
    QString output = ps.readAllStandardOutput();
    if (output.contains("oxide-renderer") &&
        output.contains("--type=renderer")) {
      QProcess kill;
      kill.start(QString("kill -%1 %2")
          .arg(QString::number(signal), QString::number(descendant)));
      kill.waitForFinished();
    }
  }
}

void OxideTestingUtils::wait(int ms) {
  QTest::qWait(ms);
}
