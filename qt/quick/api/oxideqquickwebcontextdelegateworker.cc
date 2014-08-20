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
// until this is completed. We do this because we require the worker to be
// on a thread with a Qt event loop, so that signals / slots work

// There are several classes involved to make WebContextDelegateWorker function:
// - OxideQQuickWebContextDelegateWorkerPrivate:
//    This is the private impl for OxideQQuickWebContextDelegateWorker and lives
//    on the UI thread. It has strong references to IOThreadController and
//    HelperThreadController and delivers signals to HelperThreadController
//    for starting the worker and sending messages
//
// - HelperThreadController:
//    This object lives on the helper thread, and is responsible for
//    evaluating the script and receiving events from the other threads via
//    several slots. It is shared between IOThreadController and
//    OxideQQuickWebContextDelegateWorkerPrivateother for the purpose of
//    keeping the script alive as long as either the IO thread or UI thread
//    entry points remain accessible
//
// - IOThreadController:
//    This lives on the UI thread and OxideQQuickWebContextDelegateWorkerPrivate
//    has an owning reference to it, but is accessed from Chromium's IO thread.
//    It has a strong reference to HelperThreadController, which it delivers
//    blocking signals to for processing events from Chromium. Accesses to
//    this object must always be via a QSharedPointer

namespace oxide {
namespace qquick {
namespace webcontextdelegateworker {

class HelperThread : public QThread {
 public:
  HelperThread(QObject* parent)
      : QThread(parent) {}

  virtual ~HelperThread() {
    Q_ASSERT(s_instance == this);
    s_instance = NULL;

    quit();
    wait();
  }

  static HelperThread* instance() {
    if (!s_instance) {
      s_instance = new HelperThread(QGuiApplication::instance());
      // XXX(chrisccoulson): Should we set a priority here? Remember, this will
      //  be blocking the IO thread
      s_instance->setObjectName("Oxide_WebContextDelegateWorkerHelperThread");
      s_instance->start();
    }

    return s_instance;
  }

 private:
  static HelperThread* s_instance;
};

HelperThread* HelperThread::s_instance;

class Api : public QObject {
  Q_OBJECT

  Q_PROPERTY(QJSValue onMessage READ onMessage WRITE setOnMessage)

 public:
  Api(HelperThreadController* controller);
  virtual ~Api() {}

  QJSValue onMessage() const {
    return on_message_handler_;
  }
  void setOnMessage(const QJSValue& func) {
    on_message_handler_ = func;
  }

  Q_INVOKABLE void sendMessage(const QVariant& message);

 private:
  HelperThreadController* controller_;
  QJSValue on_message_handler_;
};

class HelperThreadController : public QObject {
  Q_OBJECT

 public:
  HelperThreadController()
      : running_(false),
        api_(this) {}
  virtual ~HelperThreadController() {
    Q_ASSERT(thread() == QThread::currentThread());
  }

 Q_SIGNALS:
  void sendMessage(const QVariant& message);
  void error(const QString& error);

 public Q_SLOTS:
  void runScript(const QUrl& url);
  void receiveMessage(const QVariant& message);

  void callEntryPointInWorker(const QString& entry, QObject* data);

 private:
  bool running_;
  Api api_;
  QScopedPointer<QJSEngine> engine_;
  QJSValue exports_;
};

IOThreadController::IOThreadController(
    const QSharedPointer<HelperThreadController>& ht_controller)
    : ht_controller_(ht_controller) {
  connect(this, SIGNAL(callEntryPointInWorker(const QString&, QObject*)),
          ht_controller_.data(),
          SLOT(callEntryPointInWorker(const QString&, QObject*)),
          Qt::BlockingQueuedConnection);
}

IOThreadController::~IOThreadController() {
  Q_ASSERT(thread() == QThread::currentThread());
  disconnect(this, SIGNAL(callEntryPointInWorker(const QString&, QObject*)),
             ht_controller_.data(),
             SLOT(callEntryPointInWorker(const QString&, QObject*)));
}

Api::Api(HelperThreadController* controller)
    : QObject(controller),
      controller_(controller) {}
   
void Api::sendMessage(const QVariant& message) {
  if (message.type() != QVariant::Map &&
      message.type() != QVariant::List &&
      message.type() != QVariant::StringList) {
    return;
  }

  Q_EMIT controller_->sendMessage(message);
}

void HelperThreadController::runScript(const QUrl& source) {
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

void HelperThreadController::receiveMessage(
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

void HelperThreadController::callEntryPointInWorker(
    const QString& entry, QObject* data) {
  Q_ASSERT(thread() == QThread::currentThread());
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

} // namespace webcontextdelegateworker
} // namespace qquick
} // namespace oxide

using namespace oxide::qquick::webcontextdelegateworker;

OxideQQuickWebContextDelegateWorkerPrivate::OxideQQuickWebContextDelegateWorkerPrivate() :
    constructed_(false),
    attached_count_(0),
    in_destruction_(false),
    helper_thread_controller_(new HelperThreadController(), &QObject::deleteLater),
    io_thread_controller_(new IOThreadController(helper_thread_controller_),
                          &QObject::deleteLater) {}

OxideQQuickWebContextDelegateWorkerPrivate::~OxideQQuickWebContextDelegateWorkerPrivate() {}

// static
OxideQQuickWebContextDelegateWorkerPrivate*
OxideQQuickWebContextDelegateWorkerPrivate::get(OxideQQuickWebContextDelegateWorker* q) {
  return q->d_func();
}

OxideQQuickWebContextDelegateWorker::OxideQQuickWebContextDelegateWorker() :
    d_ptr(new OxideQQuickWebContextDelegateWorkerPrivate()) {
  Q_D(OxideQQuickWebContextDelegateWorker);

  d->helper_thread_controller_->moveToThread(HelperThread::instance());

  connect(d, SIGNAL(runScript(const QUrl&)),
          d->helper_thread_controller_.data(), SLOT(runScript(const QUrl&)));
  connect(d, SIGNAL(sendMessage(const QVariant&)),
          d->helper_thread_controller_.data(),
          SLOT(receiveMessage(const QVariant&)));

  connect(d->helper_thread_controller_.data(), SIGNAL(error(const QString&)),
          this, SIGNAL(error(const QString&)));
  connect(d->helper_thread_controller_.data(),
          SIGNAL(sendMessage(const QVariant&)),
          this, SIGNAL(message(const QVariant&)));
}

OxideQQuickWebContextDelegateWorker::~OxideQQuickWebContextDelegateWorker() {
  Q_D(OxideQQuickWebContextDelegateWorker);

  Q_ASSERT(!d->in_destruction_);
  d->in_destruction_ = true;

  OxideQQuickWebContext* context = qobject_cast<OxideQQuickWebContext *>(parent());
  if (context) {
    OxideQQuickWebContextPrivate::get(context)->delegateWorkerDestroyed(this);
  }

  Q_ASSERT(d->attached_count_ == 0);

  disconnect(d, SIGNAL(runScript(const QUrl&)),
             d->helper_thread_controller_.data(),
             SLOT(runScript(const QUrl&)));
  disconnect(d, SIGNAL(sendMessage(const QVariant&)),
             d->helper_thread_controller_.data(),
             SLOT(receiveMessage(const QVariant&)));

  disconnect(d->helper_thread_controller_.data(),
             SIGNAL(error(const QString&)),
             this, SIGNAL(error(const QString&)));
  disconnect(d->helper_thread_controller_.data(),
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

  Q_EMIT d->runScript(d->source_);
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

  Q_EMIT d->sendMessage(message);
}

#include "oxideqquickwebcontextdelegateworker.moc"
