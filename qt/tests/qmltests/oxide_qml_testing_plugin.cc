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
#include <QClipboard>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QLatin1String>
#include <QQmlContext>
#include <QQmlExtensionPlugin>
#include <QQmlParserStatus>
#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <QVariant>

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

  Q_INVOKABLE void copyToClipboard(const QString& mimeType, const QString& data) {
    QMimeData * mime_data = new QMimeData();
    mime_data->setData(mimeType, data.toUtf8());
    QGuiApplication::clipboard()->setMimeData(mime_data);
  }

  Q_INVOKABLE QString getClipboardImageData() const {
    return QString("iVBORw0KGgoAAAANSUhEUgAAABAAAA"
                   "AQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL"
                   "2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9gHHRY6HX"
                   "zuCtIAAAAZdEVYdENvbW1lbnQAQ3JlYXRlZCB3aXRoIEdJTVBXgQ"
                   "4XAAAB6ElEQVQ4y33TT2jPcRzH8cfn9/1uMxsbWmsi1jS05cJuyF"
                   "r7g4t22QgnWg6Sw5aLGwdJymFFTZKk0JTFKIkLByItsvIvSi2HsZ"
                   "/N2n77OuxbfrF51/vw+fR5vnt/3q/XO/hPjLUKIShHlMwY+3RfCy"
                   "5hBNvr+RjPB2fbbEUHahHhJjZjaZotuPBPganT1qNq8oEq7MMiCB"
                   "mhqMytye824SceQfgLLsFdrJge1pz77Bw2YgwVyYyBHx88jorFpc"
                   "u9TxKDcR5chBPYAnGtXbkvOpIZ2UX3yLapChldZTXOYiHehaAzk9"
                   "fAmvRf8A0DhY2zMJQO+oqT6MMMqrEjZNu0YGco8bywQZPIfhwv6H"
                   "ZinuGux0Wsw4sYvahJfnqT++JMtEonXv9H3fc4jCWYiDGOBJNiEW"
                   "Isno/+OKgMe1J5P8c4iMZMueGoSicy2JkaZq7YgL2owO2Qp0IlLu"
                   "cNcneSuFbYM3sYmjVTLa6jDlkc+GOkxIigF80IEhPDPdqHKE190J"
                   "B2VpcSj/AwzOHEo1j7tlsfrqbywkSI9C9r8nB6TDT6VH99YmSuXT"
                   "iPIhzC6rz7Zyu7PC2udgy/Kts90T1HgYJu4xgf4hRupFZeECIvi6"
                   "sdQU36dBtezbuN9eQwnCY5priCVoziDvwGNw6PMb/zL+4AAAAASU"
                   "VORK5CYII=");
  }

  Q_INVOKABLE void copyImageToClipboard() {
    QMimeData * mime_data = new QMimeData();
    mime_data->setData("image/png"
        , QByteArray::fromBase64(getClipboardImageData().toUtf8()));
    QGuiApplication::clipboard()->setMimeData(mime_data);
  }

  Q_INVOKABLE QString copyFromClipboard(const QString& mimeType) {
    const QMimeData * mime_data = QGuiApplication::clipboard()->mimeData();
    if (mime_data->hasFormat(mimeType)) {
      return QString(mime_data->data(mimeType));
    }
    return QString();
  }

  Q_INVOKABLE void clearClipboard(const QString& data) {
    QGuiApplication::clipboard()->clear();
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
        uri, 1, 0, "OxideTestingUtils", UtilsFactory);
    qmlRegisterUncreatableType<DestructionObserver>(
        uri, 1, 0, "DestructionObserver",
        "Create this with OxideTestingUtils.createDestructionObserver()");
    qmlRegisterType<ExternalProtocolHandler>(uri, 1, 0, "ExternalProtocolHandler");
  }
};

#include "oxide_qml_testing_plugin.moc"
