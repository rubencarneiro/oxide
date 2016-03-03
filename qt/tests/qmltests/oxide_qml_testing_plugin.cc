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

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <QClipboard>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QLatin1String>
#include <QList>
#include <QQmlContext>
#include <QQmlExtensionPlugin>
#include <QQmlParserStatus>
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

class ExternalProtocolHandler : public QObject,
                                public QQmlParserStatus {
  Q_OBJECT
  Q_PROPERTY(QString scheme READ scheme WRITE setScheme NOTIFY schemeChanged)

 public:
  ExternalProtocolHandler()
      : locked_(false) {}

  virtual ~ExternalProtocolHandler() {
    if (!scheme_.isEmpty()) {
      QDesktopServices::unsetUrlHandler(scheme_);
    }
  }

  void classBegin() final {}
  void componentComplete() final {
    locked_ = true;
    if (!scheme_.isEmpty()) {
      QDesktopServices::setUrlHandler(scheme_, this, "openUrl");
    }
  }

  QString scheme() const { return scheme_; }
  void setScheme(const QString& scheme) {
    if (locked_) {
      return;
    }

    scheme_ = scheme;
    Q_EMIT schemeChanged();
  }

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
  DestructionObserver(QObject* object) :
      destroyed_(false) {
    connect(object, SIGNAL(destroyed()),
            this, SLOT(onDestroyed()));
  }

  virtual ~DestructionObserver() {
    disconnect(this, SLOT(onDestroyed()));
  }

  bool destroyed() const { return destroyed_; }

 Q_SIGNALS:
  void destroyedChanged();

 private Q_SLOTS:
  void onDestroyed() {
    Q_ASSERT(!destroyed_);

    destroyed_ = true;
    Q_EMIT destroyedChanged();
  }

 private:
  bool destroyed_;
};

class OxideTestingUtils : public QObject {
  Q_OBJECT

 public:
  OxideTestingUtils() {}

  Q_INVOKABLE QObject* qObjectParent(QObject* object) {
    if (!object) {
      return nullptr;
    }

    return object->parent();
  }

  Q_INVOKABLE void destroyQObjectNow(QObject* object) {
    if (!object) {
      return;
    }

    delete object;
  }

  Q_INVOKABLE DestructionObserver* createDestructionObserver(QObject* object) {
    if (!object) {
      return nullptr;
    }

    return new DestructionObserver(object);
  }

  Q_INVOKABLE QVariant getAppProperty(const QString& property) {
    return QCoreApplication::instance()->property(property.toStdString().c_str());
  }

  Q_INVOKABLE void setAppProperty(const QString& property, const QVariant& value) {
    QCoreApplication::instance()->setProperty(property.toStdString().c_str(), value);
  }

  Q_INVOKABLE void removeAppProperty(const QString& property) {
    QCoreApplication::instance()->setProperty(property.toStdString().c_str(), QVariant());
  }

  Q_INVOKABLE void killWebProcesses(uint signal=SIGKILL) {
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

  Q_INVOKABLE void copyToClipboard(const QString& mimeType, const QString& data) {
    QMimeData * mime_data = new QMimeData();
    if (mimeType.startsWith("image/")) {
      mime_data->setData(mimeType, QByteArray::fromBase64(data.toUtf8()));
    } else {
      mime_data->setData(mimeType, data.toUtf8());
    }
    QGuiApplication::clipboard()->setMimeData(mime_data);
  }

  Q_INVOKABLE QString getFromClipboard(const QString& mimeType) {
    const QMimeData * mime_data = QGuiApplication::clipboard()->mimeData();
    if (mime_data->hasFormat(mimeType)) {
      return QString(mime_data->data(mimeType));
    }
    return QString();
  }

  Q_INVOKABLE void clearClipboard() {
    QGuiApplication::clipboard()->clear();
  }

  Q_INVOKABLE void clearTemporarySavedPermissionStatuses(
      OxideQQuickWebContext* context) {
    OxideQQuickWebContextPrivate::get(context)
        ->clearTemporarySavedPermissionStatuses();
  }

  Q_INVOKABLE void wait(int ms) {
    QTest::qWait(ms);
  }
};

QObject* UtilsFactory(QQmlEngine* engine, QJSEngine* script_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(script_engine);

  return new OxideTestingUtils();
}

class OxideQmlTestingPlugin : public QQmlExtensionPlugin {
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface" FILE "oxide_qml_testing_plugin.json")
  Q_OBJECT

 public:
  void registerTypes(const char* uri) {
    qmlRegisterSingletonType<OxideTestingUtils>(
        uri, 1, 0, "Utils", UtilsFactory);
    qmlRegisterUncreatableType<DestructionObserver>(
        uri, 1, 0, "DestructionObserver",
        "Create this with OxideTestingUtils.createDestructionObserver()");
    qmlRegisterType<ExternalProtocolHandler>(
        uri, 1, 0, "ExternalProtocolHandler");
  }
};

#include "oxide_qml_testing_plugin.moc"
