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

#include "oxideqquickwebcontextdelegateworker_p.h"
#include "oxideqquickwebcontextdelegateworker_p_p.h"

#include <QByteArray>
#include <QFile>
#include <QGuiApplication>
#include <QIODevice>
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueList>
#include <QQmlEngine>
#include <QString>
#include <QtDebug>
#include <QThread>
#include <QVariant>

#include "qt/core/api/oxideqglobal.h"
#include "qt/quick/oxide_qquick_init.h"

#include "oxideqquickwebcontext.h"
#include "oxideqquickwebcontext_p.h"

// WebContextDelegateWorker runs a script on Chromium's IO thread. It can
// exchange messages with the UI thread, and provides entry points for
// handling events from Chromium (on its IO thread).

// There are several classes involved to make WebContextDelegateWorker function:
// - OxideQQuickWebContextDelegateWorkerPrivate:
//    This is the private impl for OxideQQuickWebContextDelegateWorker and lives
//    on the UI thread. It has a strong reference to IOThreadController and
//    calls methods on it asynchronously for starting the worker and sending
//    messages
//
// - IOThreadController:
//    This object lives on Chromium's IO thread and is responsible for
//    evaluating the script, receiving events from Chromium and handling
//    asynchronous calls from the UI thread. OxideQQuickWebContextDelegateWorkerPrivate
//    has an owning reference to it. Because the owning reference lives on the
//    UI thread, accesses to it on the IO thread must be done via QSharedPointer

namespace oxide {
namespace qquick {
namespace webcontextdelegateworker {

IOThreadController::IOThreadController() {}
IOThreadController::~IOThreadController() {}

class Api : public QObject {
  Q_OBJECT

  Q_PROPERTY(QJSValue onMessage READ onMessage WRITE setOnMessage)

 public:
  Api(IOThreadControllerImpl* controller);
  virtual ~Api() {}

  QJSValue onMessage() const {
    return on_message_handler_;
  }
  void setOnMessage(const QJSValue& func) {
    on_message_handler_ = func;
  }

  Q_INVOKABLE void sendMessage(const QVariant& message);

 private:
  IOThreadControllerImpl* controller_;
  QJSValue on_message_handler_;
};

class IOThreadControllerImpl : public IOThreadController {
  Q_OBJECT

 public:
  IOThreadControllerImpl()
      : running_(false),
        api_(this) {}
  virtual ~IOThreadControllerImpl() {
    Q_ASSERT(thread() == QThread::currentThread());
  }

  Q_INVOKABLE void runScript(const QUrl& url);
  Q_INVOKABLE void receiveMessage(const QVariant& message);

 Q_SIGNALS:
  void sendMessage(const QVariant& message);
  void error(const QString& error);

 private:
  // IOThreadController implementation
  void CallEntryPointInWorker(const QString& entry, QObject* data) final;

  bool running_;
  Api api_;
  QScopedPointer<QJSEngine> engine_;
  QJSValue exports_;
};

Api::Api(IOThreadControllerImpl* controller)
    : QObject(controller),
      controller_(controller) {}
   
void Api::sendMessage(const QVariant& message) {
  QVariant aux = message;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  QJsonDocument doc = QJsonDocument::fromVariant(aux);
  if (doc.isNull()) {
    qWarning() << "Api::sendMessage: Invalid message from worker";
    return;
  }

  Q_EMIT controller_->sendMessage(doc.toVariant());
}

void IOThreadControllerImpl::CallEntryPointInWorker(
    const QString& entry, QObject* data) {
  Q_ASSERT(thread() == QThread::currentThread());
  if (!running_) {
    return;
  }

  QJSValue func = exports_.property(entry);
  if (!func.isCallable()) {
    return;
  }

  QQmlEngine::setObjectOwnership(data, QQmlEngine::CppOwnership);

  QJSValueList argv;
  argv.append(engine_->newQObject(data));

  func.call(argv);
}

void IOThreadControllerImpl::runScript(const QUrl& source) {
  Q_ASSERT(thread() == QThread::currentThread());
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

void IOThreadControllerImpl::receiveMessage(
    const QVariant& message) {
  Q_ASSERT(thread() == QThread::currentThread());
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

} // namespace webcontextdelegateworker
} // namespace qquick
} // namespace oxide

using namespace oxide::qquick::webcontextdelegateworker;

OxideQQuickWebContextDelegateWorkerPrivate::OxideQQuickWebContextDelegateWorkerPrivate()
    : attached_count(0),
      owned_by_context(false),
      constructed_(false),
      in_destruction_(false),
      io_thread_controller_(new IOThreadControllerImpl(), &QObject::deleteLater) {}

OxideQQuickWebContextDelegateWorkerPrivate::~OxideQQuickWebContextDelegateWorkerPrivate() {}

// static
OxideQQuickWebContextDelegateWorkerPrivate*
OxideQQuickWebContextDelegateWorkerPrivate::get(OxideQQuickWebContextDelegateWorker* q) {
  return q->d_func();
}

QSharedPointer<oxide::qquick::webcontextdelegateworker::IOThreadController>
OxideQQuickWebContextDelegateWorkerPrivate::io_thread_controller() const {
  return io_thread_controller_;
}

OxideQQuickWebContextDelegateWorker::OxideQQuickWebContextDelegateWorker() :
    d_ptr(new OxideQQuickWebContextDelegateWorkerPrivate()) {
  Q_D(OxideQQuickWebContextDelegateWorker);

  oxide::qquick::EnsureChromiumStarted();
  d->io_thread_controller_->moveToThread(oxideGetIOThread());

  connect(d->io_thread_controller_.data(), SIGNAL(error(const QString&)),
          this, SIGNAL(error(const QString&)));
  connect(d->io_thread_controller_.data(),
          SIGNAL(sendMessage(const QVariant&)),
          this, SIGNAL(message(const QVariant&)));
}

OxideQQuickWebContextDelegateWorker::~OxideQQuickWebContextDelegateWorker() {
  Q_D(OxideQQuickWebContextDelegateWorker);

  Q_ASSERT(!d->in_destruction_);
  d->in_destruction_ = true;

  if (d->context) {
    OxideQQuickWebContextPrivate::get(d->context)->delegateWorkerDestroyed(this);
  }

  Q_ASSERT(d->attached_count == 0);
  Q_ASSERT(!d->context);

  disconnect(d->io_thread_controller_.data(),
             SIGNAL(error(const QString&)),
             this, SIGNAL(error(const QString&)));
  disconnect(d->io_thread_controller_.data(),
             SIGNAL(sendMessage(const QVariant&)),
             this, SIGNAL(message(const QVariant&)));
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

  QMetaObject::invokeMethod(d->io_thread_controller_.data(),
                            "runScript",
                            Q_ARG(QUrl, d->source_));
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

  QVariant aux = message;
  if (aux.userType() == qMetaTypeId<QJSValue>()) {
    aux = aux.value<QJSValue>().toVariant();
  }

  QJsonDocument doc = QJsonDocument::fromVariant(aux);
  if (doc.isNull()) {
    qWarning() <<
        "OxideQQuickWebContextDelegateWorker::sendMessage: Invalid message";
    return;
  }

  QMetaObject::invokeMethod(d->io_thread_controller_.data(),
                            "receiveMessage",
                            Q_ARG(QVariant, doc.toVariant()));
}

#include "oxideqquickwebcontextdelegateworker.moc"
