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

#include "oxideqquickwebcontextdelegateworker_p.h"
#include "oxideqquickwebcontextdelegateworker_p_p.h"

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

#include "oxideqquickwebcontext_p.h"
#include "oxideqquickwebcontext_p_p.h"

// WebContextDelegateWorker runs a script on a dedicated worker thread. It
// can exchange messages with the UI thread, and provide entry points
// for handling events from Chromium's IO thread. For handling events from
// Chromium, we post an event to the helper thread and block the IO thread
// until this is completed.

// There are several classes involved to make WebContextDelegateWorker function:
// - WebContextDelegateWorkerHelperThreadController:
//    This object lives on the helper thread, and is responsible for
//    evaluating JS code. It receives events from the other objects via
//    several slots
//
// - WebContextDelegateWorkerUIThreadController:
//    This lives on the UI thread. It delivers signals to slots on
//    WebContextDelegateWorkerHelperThreadController for starting
//    the worker and sending messages. It has slots for receiving signals
//    from WebContextDelegateWorkerHelperThreadController, for receiving
//    messages and errors
//
// - WebContextDelegateWorkerIOThreadController:
//    This lives on the UI thread but is accessed from Chromium's IO thread.
//    It delivers blocking signals to slots on NetworkDelegateHelperThreadController
//    for processing network related events from Chromium

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

class WebContextDelegateWorkerApi : public QObject {
  Q_OBJECT

  Q_PROPERTY(QJSValue onMessage READ onMessage WRITE setOnMessage)

 public:
  WebContextDelegateWorkerApi(
      WebContextDelegateWorkerHelperThreadController* controller);
  virtual ~WebContextDelegateWorkerApi() {}

  QJSValue onMessage() const {
    return on_message_handler_;
  }
  void setOnMessage(const QJSValue& func) {
    on_message_handler_ = func;
  }

  Q_INVOKABLE void sendMessage(const QVariant& message);

 private:
  WebContextDelegateWorkerHelperThreadController* controller_;
  QJSValue on_message_handler_;
};

class WebContextDelegateWorkerHelperThreadController : public QObject {
  Q_OBJECT

 public:
  WebContextDelegateWorkerHelperThreadController() :
      running_(false),
      api_(this) {}
  virtual ~WebContextDelegateWorkerHelperThreadController() {}

 Q_SIGNALS:
  void sendMessage(const QVariant& message);
  void error(const QString& error);

 public Q_SLOTS:
  void runScript(const QUrl& url);
  void receiveMessage(const QVariant& message);

  void callEntryPointInWorker(const QString& entry, QObject* data);

 private:
  bool running_;
  WebContextDelegateWorkerApi api_;
  QScopedPointer<QJSEngine> engine_;
  QJSValue exports_;
};

class WebContextDelegateWorkerUIThreadController : public QObject {
  Q_OBJECT

 public:
  WebContextDelegateWorkerUIThreadController() {}
  virtual ~WebContextDelegateWorkerUIThreadController() {}

 Q_SIGNALS:
  void runScript(const QUrl& source);
  void sendMessage(const QVariant& message);
};

WebContextDelegateWorkerIOThreadController::WebContextDelegateWorkerIOThreadController() {}

WebContextDelegateWorkerIOThreadController::~WebContextDelegateWorkerIOThreadController() {}

WebContextDelegateWorkerApi::WebContextDelegateWorkerApi(
    WebContextDelegateWorkerHelperThreadController* controller) :
    QObject(controller),
    controller_(controller) {}
   
void WebContextDelegateWorkerApi::sendMessage(const QVariant& message) {
  if (message.type() != QVariant::Map &&
      message.type() != QVariant::List &&
      message.type() != QVariant::StringList) {
    return;
  }

  Q_EMIT controller_->sendMessage(message);
}

void WebContextDelegateWorkerHelperThreadController::runScript(const QUrl& source) {
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

void WebContextDelegateWorkerHelperThreadController::receiveMessage(
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

void WebContextDelegateWorkerHelperThreadController::callEntryPointInWorker(
    const QString& entry, QObject* data) {
  if (!running_) {
    return;
  }

  QJSValue func = exports_.property(entry);
  if (!func.isCallable()) {
    return;
  }

  QJSValueList argv;
  argv.append(engine_->newQObject(data));

  func.call(argv);

  delete data;
}

} // namespace qquick
} // namespace oxide

OxideQQuickWebContextDelegateWorkerPrivate::OxideQQuickWebContextDelegateWorkerPrivate() :
    io_thread_controller(new WebContextDelegateWorkerIOThreadController()),
    constructed_(false),
    helper_thread_controller_(new WebContextDelegateWorkerHelperThreadController()),
    ui_thread_controller_(new WebContextDelegateWorkerUIThreadController()) {}

OxideQQuickWebContextDelegateWorkerPrivate::~OxideQQuickWebContextDelegateWorkerPrivate() {
  helper_thread_controller_->deleteLater();
}

// static
OxideQQuickWebContextDelegateWorkerPrivate*
OxideQQuickWebContextDelegateWorkerPrivate::get(OxideQQuickWebContextDelegateWorker* q) {
  return q->d_func();
}

OxideQQuickWebContextDelegateWorker::OxideQQuickWebContextDelegateWorker() :
    d_ptr(new OxideQQuickWebContextDelegateWorkerPrivate()) {
  Q_D(OxideQQuickWebContextDelegateWorker);

  d->helper_thread_controller_->moveToThread(NetworkDelegateHelperThread::instance());

  connect(d->ui_thread_controller_.data(), SIGNAL(runScript(const QUrl&)),
          d->helper_thread_controller_, SLOT(runScript(const QUrl&)));
  connect(d->ui_thread_controller_.data(), SIGNAL(sendMessage(const QVariant&)),
          d->helper_thread_controller_, SLOT(receiveMessage(const QVariant&)));

  connect(d->helper_thread_controller_, SIGNAL(error(const QString&)),
          this, SIGNAL(error(const QString&)));
  connect(d->helper_thread_controller_, SIGNAL(sendMessage(const QVariant&)),
          this, SIGNAL(message(const QVariant&)));

  connect(d->io_thread_controller.data(), SIGNAL(callEntryPointInWorker(const QString&, QObject*)),
          d->helper_thread_controller_, SLOT(callEntryPointInWorker(const QString&, QObject*)),
          Qt::BlockingQueuedConnection);
}

OxideQQuickWebContextDelegateWorker::~OxideQQuickWebContextDelegateWorker() {
  Q_D(OxideQQuickWebContextDelegateWorker);

  OxideQQuickWebContext* context = qobject_cast<OxideQQuickWebContext *>(parent());
  if (context) {
    OxideQQuickWebContextPrivate::get(context)->delegateWorkerDestroyed(this);
  }

  disconnect(d->ui_thread_controller_.data(), SIGNAL(runScript(const QUrl&)),
             d->helper_thread_controller_, SLOT(runScript(const QUrl&)));
  disconnect(d->ui_thread_controller_.data(), SIGNAL(sendMessage(const QVariant&)),
             d->helper_thread_controller_, SLOT(receiveMessage(const QVariant&)));

  disconnect(d->helper_thread_controller_, SIGNAL(error(const QString&)),
             this, SIGNAL(error(const QString&)));
  disconnect(d->helper_thread_controller_, SIGNAL(sendMessage(const QVariant&)),
             this, SIGNAL(message(const QVariant&)));

  disconnect(d->io_thread_controller.data(), SIGNAL(callEntryPointInWorker(const QString&, QObject*)),
             d->helper_thread_controller_, SLOT(callEntryPointInWorker(const QString&, QObject*)));
}

void OxideQQuickWebContextDelegateWorker::classBegin() {}

void OxideQQuickWebContextDelegateWorker::componentComplete() {
  Q_D(OxideQQuickWebContextDelegateWorker);

  Q_ASSERT(!d->constructed_);
  d->constructed_ = true;

  if (d->source_.isEmpty()) {
    qWarning() << "WebContextDelegateWorker.source not set";
    return;
  }

  Q_EMIT d->ui_thread_controller_->runScript(d->source_);
}

QUrl OxideQQuickWebContextDelegateWorker::source() const {
  Q_D(const OxideQQuickWebContextDelegateWorker);

  return d->source_;
}

void OxideQQuickWebContextDelegateWorker::setSource(const QUrl& source) {
  Q_D(OxideQQuickWebContextDelegateWorker);

  if (!source.isLocalFile() && !source.isEmpty()) {
    qWarning() << "WebContextDelegateWorker.source only supports local file URL's";
    return;
  }

  d->source_ = source;
}

void OxideQQuickWebContextDelegateWorker::sendMessage(const QVariant& message) {
  Q_D(OxideQQuickWebContextDelegateWorker);

  if (message.type() != QVariant::Map &&
      message.type() != QVariant::List &&
      message.type() != QVariant::StringList) {
    qWarning() << "Called WebContextDelegateWorker.sendMessage with an invalid argument";
    return;
  }

  Q_EMIT d->ui_thread_controller_->sendMessage(message);
}

#include "oxideqquickwebcontextdelegateworker.moc"
