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

#include <QDir>
#include <QLatin1String>
#include <QQmlContext>
#include <QQmlExtensionPlugin>
#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <QUrl>

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
  Q_PROPERTY(QUrl DATA_PATH READ dataPath CONSTANT)

 public:
  OxideTestingUtils() {}

  QUrl dataPath() const {
    static QUrl url;
    static bool initialized = false;

    if (initialized) {
      return url;
    }

    initialized = true;

    QString path(QString(qgetenv("OXIDE_TESTING_DATA_PATH")));
    if (!path.isEmpty()) {
      QDir dir(path);
      url = QUrl::fromLocalFile(dir.absolutePath());
    }

    return url;
  }

  Q_INVOKABLE DestructionObserver* createDestructionObserver(QObject* object) {
    if (!object) {
      return NULL;
    }

    return new DestructionObserver(object);
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
    Q_ASSERT(QLatin1String(uri) == QLatin1String("com.canonical.Oxide.Testing"));

    qmlRegisterSingletonType<OxideTestingUtils>(
        uri, 0, 1, "Utils", UtilsFactory);
    qmlRegisterUncreatableType<DestructionObserver>(
        uri, 0, 1, "DestructionObserver",
        "Create this with OxideTestingUtils.createDestructionObserver()");
  }
};

#include "oxide_qml_testing_plugin.moc"
