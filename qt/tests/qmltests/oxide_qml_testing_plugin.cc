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

#include <QCoreApplication>
#include <QDesktopServices>
#include <QLatin1String>
#include <QQmlContext>
#include <QQmlExtensionPlugin>
#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <QVariant>

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
      return NULL;
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
      return NULL;
    }

    return new DestructionObserver(object);
  }

  Q_INVOKABLE QVariant getAppProperty(const QString& property) {
    QCoreApplication::instance()->property(property.toStdString().c_str());
  }

  Q_INVOKABLE void setAppProperty(const QString& property, const QVariant& value) {
    QCoreApplication::instance()->setProperty(property.toStdString().c_str(), value);
  }

  Q_INVOKABLE void removeAppProperty(const QString& property) {
    QCoreApplication::instance()->setProperty(property.toStdString().c_str(), QVariant());
  }

  Q_INVOKABLE void setUrlHandler(const QString& scheme, bool doHandle) {
    if (doHandle) {
      QDesktopServices::setUrlHandler(scheme, this, "urlHandled");
    } else {
      // Register an inexistent handler for the scheme, to ensure that
      // QDesktopServices::openUrl(…) returns false (its current implementation
      // ignores the return value of the custom handler method, so returning
      // false from a valid handler wouldn’t help).
      QDesktopServices::setUrlHandler(scheme, this, "doNotHandleUrl");
    }
  }

  Q_INVOKABLE void unsetUrlHandler(const QString& scheme) {
    QDesktopServices::unsetUrlHandler(scheme);
  }

 Q_SIGNALS:
  void urlHandled(const QUrl& url);
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
    Q_ASSERT(QLatin1String(uri) == QLatin1String("com.canonical.Oxide.Testing"));

    qmlRegisterSingletonType<OxideTestingUtils>(
        uri, 1, 0, "OxideTestingUtils", UtilsFactory);
    qmlRegisterUncreatableType<DestructionObserver>(
        uri, 1, 0, "DestructionObserver",
        "Create this with OxideTestingUtils.createDestructionObserver()");
  }
};

#include "oxide_qml_testing_plugin.moc"
