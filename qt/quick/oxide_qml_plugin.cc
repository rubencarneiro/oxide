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

#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "qt/core/api/oxideqincomingmessage.h"
#include "qt/core/api/oxideqloadevent.h"
#include "qt/quick/api/oxideqquickmessagehandler_p.h"
#include "qt/quick/api/oxideqquickoutgoingmessagerequest_p.h"
#include "qt/quick/api/oxideqquickuserscript_p.h"
#include "qt/quick/api/oxideqquickwebcontext_p.h"
#include "qt/quick/api/oxideqquickwebframe_p.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

QT_USE_NAMESPACE

namespace {

class OxideQQuickDefaultWebContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString product READ product WRITE setProduct NOTIFY productChanged)
  Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
  Q_PROPERTY(QUrl dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged)
  Q_PROPERTY(QUrl cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
  Q_PROPERTY(QString acceptLangs READ acceptLangs WRITE setAcceptLangs NOTIFY acceptLangsChanged)
  Q_PROPERTY(QQmlListProperty<OxideQQuickUserScript> userScripts READ userScripts)

 public:
  OxideQQuickDefaultWebContext(QObject* parent = NULL);
  virtual ~OxideQQuickDefaultWebContext();

  QString product() const;
  void setProduct(const QString& product);

  QString userAgent() const;
  void setUserAgent(const QString& user_agent);

  QUrl dataPath() const;
  void setDataPath(const QUrl& data_path);

  QUrl cachePath() const;
  void setCachePath(const QUrl& cache_path);

  QString acceptLangs() const;
  void setAcceptLangs(const QString& accept_langs);

  QQmlListProperty<OxideQQuickUserScript> userScripts();

 Q_SIGNALS:
  void productChanged();
  void userAgentChanged();
  void dataPathChanged();
  void cachePathChanged();
  void acceptLangsChanged();

 public Q_SLOTS:
  void productChangedListener();
  void userAgentChangedListener();
  void dataPathChangedListener();
  void cachePathChangedListener();
  void acceptLangsChangedListener();

 private:
  QSharedPointer<OxideQQuickWebContext> context_;
};

OxideQQuickDefaultWebContext::OxideQQuickDefaultWebContext(
    QObject* parent) :
    QObject(parent),
    context_(OxideQQuickWebContext::defaultContext()) {
  QObject::connect(context_.data(), SIGNAL(productChanged()),
                   this, SLOT(productChangedListener()));
  QObject::connect(context_.data(), SIGNAL(userAgentChanged()),
                   this, SLOT(userAgentChangedListener()));
  QObject::connect(context_.data(), SIGNAL(dataPathChanged()),
                   this, SLOT(dataPathChangedListener()));
  QObject::connect(context_.data(), SIGNAL(cachePathChanged()),
                   this, SLOT(cachePathChangedListener()));
  QObject::connect(context_.data(), SIGNAL(acceptLangsChanged()),
                   this, SLOT(acceptLangsChangedListener()));
}

OxideQQuickDefaultWebContext::~OxideQQuickDefaultWebContext() {
  QObject::disconnect(context_.data(), SIGNAL(productChanged()),
                      this, SLOT(productChangedListener()));
  QObject::disconnect(context_.data(), SIGNAL(userAgentChanged()),
                      this, SLOT(userAgentChangedListener()));
  QObject::disconnect(context_.data(), SIGNAL(dataPathChanged()),
                      this, SLOT(dataPathChangedListener()));
  QObject::disconnect(context_.data(), SIGNAL(cachePathChanged()),
                      this, SLOT(cachePathChangedListener()));
  QObject::disconnect(context_.data(), SIGNAL(acceptLangsChanged()),
                      this, SLOT(acceptLangsChangedListener()));
}

QString OxideQQuickDefaultWebContext::product() const {
  return context_->product();
}

void OxideQQuickDefaultWebContext::setProduct(const QString& product) {
  context_->setProduct(product);
}

QString OxideQQuickDefaultWebContext::userAgent() const {
  return context_->userAgent();
}

void OxideQQuickDefaultWebContext::setUserAgent(
    const QString& user_agent) {
  context_->setUserAgent(user_agent);
}

QUrl OxideQQuickDefaultWebContext::dataPath() const {
  return context_->dataPath();
}

void OxideQQuickDefaultWebContext::setDataPath(
    const QUrl& data_path) {
  context_->setDataPath(data_path);
}

QUrl OxideQQuickDefaultWebContext::cachePath() const {
  return context_->cachePath();
}

void OxideQQuickDefaultWebContext::setCachePath(
    const QUrl& cache_path) {
  context_->setCachePath(cache_path);
}

QString OxideQQuickDefaultWebContext::acceptLangs() const {
  return context_->acceptLangs();
}

void OxideQQuickDefaultWebContext::setAcceptLangs(
    const QString& accept_langs) {
  context_->setAcceptLangs(accept_langs);
}

QQmlListProperty<OxideQQuickUserScript>
OxideQQuickDefaultWebContext::userScripts() {
  return context_->userScripts();
}

void OxideQQuickDefaultWebContext::productChangedListener() {
  emit productChanged();
}

void OxideQQuickDefaultWebContext::userAgentChangedListener() {
  emit userAgentChanged();
}

void OxideQQuickDefaultWebContext::dataPathChangedListener() {
  emit dataPathChanged();
}

void OxideQQuickDefaultWebContext::cachePathChangedListener() {
  emit cachePathChanged();
}

void OxideQQuickDefaultWebContext::acceptLangsChangedListener() {
  emit acceptLangsChanged();
}

QObject* DefaultWebContextSingletonFactory(QQmlEngine* engine,
                                           QJSEngine* script_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(script_engine);

  return new OxideQQuickDefaultWebContext();
}

}

class OxideQmlPlugin : public QQmlExtensionPlugin {
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface" FILE "oxide_qml_plugin.json")
  Q_OBJECT
 public:
  void registerTypes(const char* uri) {
    Q_ASSERT(QLatin1String(uri) == QLatin1String("com.canonical.Oxide"));

    qmlRegisterSingletonType<OxideQQuickDefaultWebContext>(
        uri, 0, 1, "DefaultWebContext", DefaultWebContextSingletonFactory);
    qmlRegisterUncreatableType<OxideQIncomingMessage>(uri, 0, 1, "IncomingMessage",
        "IncomingMessages are created automatically by Oxide");
    qmlRegisterUncreatableType<OxideQLoadEvent>(uri, 0, 1, "LoadEvent",
        "LoadEvent' are created automatically by Oxide");
    qmlRegisterUncreatableType<OxideQQuickOutgoingMessageRequest>(uri, 0, 1, "OutgoingMessageRequest",
        "OutgoingMessageRequests are created automatically by WebFrame.sendMessage");
    qmlRegisterType<OxideQQuickUserScript>(uri, 0, 1, "UserScript");
    qmlRegisterType<OxideQQuickMessageHandler>(uri, 0, 1, "MessageHandler");
    qmlRegisterUncreatableType<OxideQQuickWebFrame>(uri, 0, 1, "WebFrame",
        "Frames are created automatically by Oxide to represent frames in the renderer");
    qmlRegisterType<OxideQQuickWebContext>(uri, 0, 1, "WebContext");
    qmlRegisterType<OxideQQuickWebView>(uri, 0, 1, "WebView");
  }
};

#include "oxide_qml_plugin.moc"
