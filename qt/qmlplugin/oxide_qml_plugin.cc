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

#include "oxide_q_incoming_message.h"
#include "oxide_q_load_status.h"
#include "oxide_q_user_script.h"
#include "oxide_qquick_message_handler_p.h"
#include "oxide_qquick_outgoing_message_request_p.h"
#include "oxide_qquick_web_frame_p.h"
#include "oxide_qquick_web_view_p.h"
#include "oxide_qquick_web_view_context_p.h"

QT_USE_NAMESPACE

namespace {

class OxideQQuickDefaultWebViewContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString product READ product WRITE setProduct NOTIFY productChanged)
  Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent NOTIFY userAgentChanged)
  Q_PROPERTY(QUrl dataPath READ dataPath WRITE setDataPath NOTIFY dataPathChanged)
  Q_PROPERTY(QUrl cachePath READ cachePath WRITE setCachePath NOTIFY cachePathChanged)
  Q_PROPERTY(QString acceptLangs READ acceptLangs WRITE setAcceptLangs NOTIFY acceptLangsChanged)
  Q_PROPERTY(QQmlListProperty<OxideQUserScript> userScripts READ userScripts)

 public:
  OxideQQuickDefaultWebViewContext(QObject* parent = NULL);
  virtual ~OxideQQuickDefaultWebViewContext();

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

  QQmlListProperty<OxideQUserScript> userScripts();

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
  QSharedPointer<OxideQQuickWebViewContext> context_;
};

OxideQQuickDefaultWebViewContext::OxideQQuickDefaultWebViewContext(
    QObject* parent) :
    QObject(parent),
    context_(OxideQQuickWebViewContext::defaultContext()) {
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

OxideQQuickDefaultWebViewContext::~OxideQQuickDefaultWebViewContext() {
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

QString OxideQQuickDefaultWebViewContext::product() const {
  return context_->product();
}

void OxideQQuickDefaultWebViewContext::setProduct(const QString& product) {
  context_->setProduct(product);
}

QString OxideQQuickDefaultWebViewContext::userAgent() const {
  return context_->userAgent();
}

void OxideQQuickDefaultWebViewContext::setUserAgent(
    const QString& user_agent) {
  context_->setUserAgent(user_agent);
}

QUrl OxideQQuickDefaultWebViewContext::dataPath() const {
  return context_->dataPath();
}

void OxideQQuickDefaultWebViewContext::setDataPath(
    const QUrl& data_path) {
  context_->setDataPath(data_path);
}

QUrl OxideQQuickDefaultWebViewContext::cachePath() const {
  return context_->cachePath();
}

void OxideQQuickDefaultWebViewContext::setCachePath(
    const QUrl& cache_path) {
  context_->setCachePath(cache_path);
}

QString OxideQQuickDefaultWebViewContext::acceptLangs() const {
  return context_->acceptLangs();
}

void OxideQQuickDefaultWebViewContext::setAcceptLangs(
    const QString& accept_langs) {
  context_->setAcceptLangs(accept_langs);
}

QQmlListProperty<OxideQUserScript>
OxideQQuickDefaultWebViewContext::userScripts() {
  return context_->userScripts();
}

void OxideQQuickDefaultWebViewContext::productChangedListener() {
  emit productChanged();
}

void OxideQQuickDefaultWebViewContext::userAgentChangedListener() {
  emit userAgentChanged();
}

void OxideQQuickDefaultWebViewContext::dataPathChangedListener() {
  emit dataPathChanged();
}

void OxideQQuickDefaultWebViewContext::cachePathChangedListener() {
  emit cachePathChanged();
}

void OxideQQuickDefaultWebViewContext::acceptLangsChangedListener() {
  emit acceptLangsChanged();
}

QObject* DefaultWebViewContextSingletonFactory(QQmlEngine* engine,
                                               QJSEngine* script_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(script_engine);

  return new OxideQQuickDefaultWebViewContext();
}

}

class OxideQmlPlugin : public QQmlExtensionPlugin {
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface" FILE "oxide_qml_plugin.json")
  Q_OBJECT
 public:
  void registerTypes(const char* uri) {
    Q_ASSERT(QLatin1String(uri) == QLatin1String("com.canonical.Oxide"));

    qmlRegisterSingletonType<OxideQQuickDefaultWebViewContext>(
        uri, 0, 1, "DefaultWebViewContext", DefaultWebViewContextSingletonFactory);
    qmlRegisterUncreatableType<OxideQIncomingMessage>(uri, 0, 1, "IncomingMessage",
        "IncomingMessages are created automatically by Oxide");
    qmlRegisterUncreatableType<OxideQLoadStatus>(uri, 0, 1, "LoadStatus",
        "LoadStatus' are created automatically by Oxide");
    qmlRegisterUncreatableType<OxideQQuickOutgoingMessageRequest>(uri, 0, 1, "OutgoingMessageRequest",
        "OutgoingMessageRequests are created automatically by WebFrame.sendMessage");
    qmlRegisterType<OxideQUserScript>(uri, 0, 1, "UserScript");
    qmlRegisterType<OxideQQuickMessageHandler>(uri, 0, 1, "MessageHandler");
    qmlRegisterUncreatableType<OxideQQuickWebFrame>(uri, 0, 1, "WebFrame",
        "Frames are created automatically by Oxide to represent frames in the renderer");
    qmlRegisterType<OxideQQuickWebViewContext>(uri, 0, 1, "WebViewContext");
    qmlRegisterType<OxideQQuickWebView>(uri, 0, 1, "WebView");
  }
};

#include "oxide_qml_plugin.moc"
