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

#include "oxideqquicknetworkdelegateworker_p.h"
#include "oxideqquicknetworkdelegateworker_p_p.h"

#include <QByteArray>
#include <QFile>
#include <QGuiApplication>
#include <QIODevice>
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueList>
#include <QString>
#include <QtDebug>
#include <QThread>
#include <QVariant>

#include "qt/core/api/oxideqnetworkcallbackevents.h"

#include "oxideqquickwebcontext_p.h"
#include "oxideqquickwebcontext_p_p.h"

using namespace oxide::qquick;

namespace oxide {
namespace qquick {

class NetworkDelegateHelperThread : public QThread {
 public:
  NetworkDelegateHelperThread(QObject* parent) :
      QThread(parent) {}

  virtual ~NetworkDelegateHelperThread() {
    Q_ASSERT(s_instance == this);
    s_instance = NULL;

    quit();
    wait();
  }

  static NetworkDelegateHelperThread* instance() {
    if (!s_instance) {
      s_instance = new NetworkDelegateHelperThread(QGuiApplication::instance());
      // XXX(chrisccoulson): Should we set a priority here? Remember, this will
      //  be blocking the IO thread
      s_instance->setObjectName("Oxide_NetworkDelegateHelperThread");
      s_instance->start();
    }

    return s_instance;
  }

 private:
  static NetworkDelegateHelperThread* s_instance;
};

NetworkDelegateHelperThread* NetworkDelegateHelperThread::s_instance;

class NetworkDelegateWorkerApi : public QObject {
  Q_OBJECT

  Q_PROPERTY(QJSValue onMessage READ onMessage WRITE setOnMessage)

 public:
  NetworkDelegateWorkerApi(
      NetworkDelegateWorkerHelperThreadController* controller);
  virtual ~NetworkDelegateWorkerApi() {}

  QJSValue onMessage() const {
    return on_message_handler_;
  }
  void setOnMessage(const QJSValue& func) {
    on_message_handler_ = func;
  }

  Q_INVOKABLE void sendMessage(const QVariant& message);

 private:
  NetworkDelegateWorkerHelperThreadController* controller_;
  QJSValue on_message_handler_;
};

class NetworkDelegateWorkerHelperThreadController : public QObject {
  Q_OBJECT

 public:
  NetworkDelegateWorkerHelperThreadController() :
      running_(false),
      api_(this) {}
  virtual ~NetworkDelegateWorkerHelperThreadController() {}

 Q_SIGNALS:
  void sendMessage(const QVariant& message);
  void error(const QString& error);

 public Q_SLOTS:
  void runScript(const QUrl& url);
  void receiveMessage(const QVariant& message);

  void beforeURLRequest(OxideQBeforeURLRequestEvent* event);
  void beforeSendHeaders(OxideQBeforeSendHeadersEvent* event);

 private:
  bool running_;
  NetworkDelegateWorkerApi api_;
  QScopedPointer<QJSEngine> engine_;
  QJSValue exports_;
};

class NetworkDelegateWorkerUIThreadController : public QObject {
  Q_OBJECT

 public:
  NetworkDelegateWorkerUIThreadController() {}
  virtual ~NetworkDelegateWorkerUIThreadController() {}

 Q_SIGNALS:
  void runScript(const QUrl& source);
  void sendMessage(const QVariant& message);
};

NetworkDelegateWorkerIOThreadController::NetworkDelegateWorkerIOThreadController() {}

NetworkDelegateWorkerIOThreadController::~NetworkDelegateWorkerIOThreadController() {}

NetworkDelegateWorkerApi::NetworkDelegateWorkerApi(
    NetworkDelegateWorkerHelperThreadController* controller) :
    QObject(controller),
    controller_(controller) {}
   
void NetworkDelegateWorkerApi::sendMessage(const QVariant& message) {
  if (message.type() != QVariant::Map &&
      message.type() != QVariant::List &&
      message.type() != QVariant::StringList) {
    return;
  }

  Q_EMIT controller_->sendMessage(message);
}

void NetworkDelegateWorkerHelperThreadController::runScript(const QUrl& source) {
  Q_ASSERT(!running_);

  // QJSEngine is created here instead of the constructor, as it must be
  // created in the thread it executes code in
  engine_.reset(new QJSEngine(this));
  exports_ = engine_->newObject();

  QFile f(source.toLocalFile());
  if (!f.open(QIODevice::ReadOnly)) {
    Q_EMIT error("Failed to open script");
    return;
  }

  QString code("(function(oxide, exports) {\n");
  code += f.readAll();
  code += "\n})";

  QJSValue func = engine_->evaluate(code);
  if (func.isError()) {
    Q_EMIT error(QString("Script evaluation threw error: ") + func.toString());
    return;
  }

  Q_ASSERT(func.isCallable());

  QJSValueList argv;
  argv.append(engine_->newQObject(&api_));
  argv.append(exports_);

  QJSValue rv = func.call(argv);
  if (rv.isError()) {
    Q_EMIT error(QString("Script running threw error: ") + rv.toString());
    return;
  }

  running_ = true;
}

void NetworkDelegateWorkerHelperThreadController::receiveMessage(
    const QVariant& message) {
  if (!running_) {
    return;
  }

  QJSValue func = api_.onMessage();
  if (!func.isCallable()) {
    return;
  }

  QJSValueList argv;
  argv.append(engine_->toScriptValue(message));

  func.call(argv);
}

void NetworkDelegateWorkerHelperThreadController::beforeURLRequest(
    OxideQBeforeURLRequestEvent* event) {
  if (!running_) {
    return;
  }

  QJSValue func = exports_.property("onBeforeURLRequest");
  if (!func.isCallable()) {
    return;
  }

  QJSValueList argv;
  argv.append(engine_->newQObject(event));

  func.call(argv);

  delete event;
}

void NetworkDelegateWorkerHelperThreadController::beforeSendHeaders(
    OxideQBeforeSendHeadersEvent* event) {
  if (!running_) {
    return;
  }

  QJSValue func = exports_.property("onBeforeSendHeaders");
  if (!func.isCallable()) {
    return;
  }

  QJSValueList argv;
  argv.append(engine_->newQObject(event));

  func.call(argv);

  delete event;
}

} // namespace qquick
} // namespace oxide

OxideQQuickNetworkDelegateWorkerPrivate::OxideQQuickNetworkDelegateWorkerPrivate() :
    io_thread_controller(new NetworkDelegateWorkerIOThreadController()),
    constructed_(false),
    helper_thread_controller_(new NetworkDelegateWorkerHelperThreadController()),
    ui_thread_controller_(new NetworkDelegateWorkerUIThreadController()) {}

OxideQQuickNetworkDelegateWorkerPrivate::~OxideQQuickNetworkDelegateWorkerPrivate() {
  helper_thread_controller_->deleteLater();
}

// static
OxideQQuickNetworkDelegateWorkerPrivate*
OxideQQuickNetworkDelegateWorkerPrivate::get(OxideQQuickNetworkDelegateWorker* q) {
  return q->d_func();
}

OxideQQuickNetworkDelegateWorker::OxideQQuickNetworkDelegateWorker() :
    d_ptr(new OxideQQuickNetworkDelegateWorkerPrivate()) {
  Q_D(OxideQQuickNetworkDelegateWorker);

  d->helper_thread_controller_->moveToThread(NetworkDelegateHelperThread::instance());

  connect(d->ui_thread_controller_.data(), SIGNAL(runScript(const QUrl&)),
          d->helper_thread_controller_, SLOT(runScript(const QUrl&)));
  connect(d->ui_thread_controller_.data(), SIGNAL(sendMessage(const QVariant&)),
          d->helper_thread_controller_, SLOT(receiveMessage(const QVariant&)));

  connect(d->helper_thread_controller_, SIGNAL(error(const QString&)),
          this, SIGNAL(error(const QString&)));
  connect(d->helper_thread_controller_, SIGNAL(sendMessage(const QVariant&)),
          this, SIGNAL(message(const QVariant&)));

  connect(d->io_thread_controller.data(), SIGNAL(beforeURLRequest(OxideQBeforeURLRequestEvent*)),
          d->helper_thread_controller_, SLOT(beforeURLRequest(OxideQBeforeURLRequestEvent*)),
          Qt::BlockingQueuedConnection);
  connect(d->io_thread_controller.data(), SIGNAL(beforeSendHeaders(OxideQBeforeSendHeadersEvent*)),
          d->helper_thread_controller_, SLOT(beforeSendHeaders(OxideQBeforeSendHeadersEvent*)),
          Qt::BlockingQueuedConnection);
}

OxideQQuickNetworkDelegateWorker::~OxideQQuickNetworkDelegateWorker() {
  Q_D(OxideQQuickNetworkDelegateWorker);

  OxideQQuickWebContext* context = qobject_cast<OxideQQuickWebContext *>(parent());
  if (context) {
    OxideQQuickWebContextPrivate::get(context)->networkDelegateWorkerDestroyed(this);
  }

  disconnect(d->ui_thread_controller_.data(), SIGNAL(runScript(const QUrl&)),
             d->helper_thread_controller_, SLOT(runScript(const QUrl&)));
  disconnect(d->ui_thread_controller_.data(), SIGNAL(sendMessage(const QVariant&)),
             d->helper_thread_controller_, SLOT(receiveMessage(const QVariant&)));

  disconnect(d->helper_thread_controller_, SIGNAL(error(const QString&)),
             this, SIGNAL(error(const QString&)));
  disconnect(d->helper_thread_controller_, SIGNAL(sendMessage(const QVariant&)),
             this, SIGNAL(message(const QVariant&)));

  disconnect(d->io_thread_controller.data(), SIGNAL(beforeURLRequest(OxideQBeforeURLRequestEvent*)),
             d->helper_thread_controller_, SLOT(beforeURLRequest(OxideQBeforeURLRequestEvent*)));
  disconnect(d->io_thread_controller.data(), SIGNAL(beforeSendHeaders(OxideQBeforeSendHeadersEvent*)),
             d->helper_thread_controller_, SLOT(beforeSendHeaders(OxideQBeforeSendHeadersEvent*)));
}

void OxideQQuickNetworkDelegateWorker::classBegin() {}

void OxideQQuickNetworkDelegateWorker::componentComplete() {
  Q_D(OxideQQuickNetworkDelegateWorker);

  Q_ASSERT(!d->constructed_);
  d->constructed_ = true;

  if (d->source_.isEmpty()) {
    qWarning() << "NetworkDelegateWorker.source not set";
    return;
  }

  Q_EMIT d->ui_thread_controller_->runScript(d->source_);
}

QUrl OxideQQuickNetworkDelegateWorker::source() const {
  Q_D(const OxideQQuickNetworkDelegateWorker);

  return d->source_;
}

void OxideQQuickNetworkDelegateWorker::setSource(const QUrl& source) {
  Q_D(OxideQQuickNetworkDelegateWorker);

  if (!source.isLocalFile() && !source.isEmpty()) {
    qWarning() << "NetworkDelegateWorker.source only supports local file URL's";
    return;
  }

  d->source_ = source;
}

void OxideQQuickNetworkDelegateWorker::sendMessage(const QVariant& message) {
  Q_D(OxideQQuickNetworkDelegateWorker);

  if (message.type() != QVariant::Map &&
      message.type() != QVariant::List &&
      message.type() != QVariant::StringList) {
    qWarning() << "Called NetworkDelegateWorker.sendMessage with an invalid argument";
    return;
  }

  Q_EMIT d->ui_thread_controller_->sendMessage(message);
}

#include "oxideqquicknetworkdelegateworker.moc"
