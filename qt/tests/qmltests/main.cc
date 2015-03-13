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
#include <QDirIterator>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLatin1String>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QQmlNetworkAccessManagerFactory>
#include <QQuickView>
#include <QString>
#include <QStringList>
#include <QtDebug>
#include <QTest>
#include <QTextStream>
#include <QtGlobal>
#include <QtQuickTest/private/qtestoptions_p.h>
#include <QtQuickTest/private/quicktestresult_p.h>
#include <QtQuickVersion>
#include <QUrl>
#include <QOpenGLContext>
#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
#include <QtQuick/private/qsgcontext_p.h>
#else
#include <QtGui/private/qopenglcontext_p.h>
#endif

#include "qt/core/api/oxideqglobal.h"

class TestNetworkAccessManager : public QNetworkAccessManager {
 public:
  TestNetworkAccessManager(const QDir& test_dir, QObject* parent)
      : QNetworkAccessManager(parent),
        test_dir_(test_dir) {}
  virtual ~TestNetworkAccessManager() {}

  QNetworkReply* createRequest(QNetworkAccessManager::Operation op,
                               const QNetworkRequest& req,
                               QIODevice* outgoing_data) final;

 private:
  QDir test_dir_;
};

QNetworkReply* TestNetworkAccessManager::createRequest(
    QNetworkAccessManager::Operation op,
    const QNetworkRequest& req,
    QIODevice* outgoing_data) {
  if (req.url().scheme() == QLatin1String("test")) {
    if (!req.url().host().isEmpty()) {
      return nullptr;
    }

    QUrl redirect;
    redirect.setScheme(QLatin1String("file"));

    QFileInfo fi(test_dir_, req.url().path().mid(1));
    redirect.setPath(fi.filePath());

    QNetworkRequest r(req);
    r.setUrl(redirect);

    return QNetworkAccessManager::createRequest(op, r, outgoing_data);
  }

  return QNetworkAccessManager::createRequest(op, req, outgoing_data);
}

class TestNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory {
 public:
  TestNetworkAccessManagerFactory(const QDir& test_dir)
      : test_dir_(test_dir) {}
  virtual ~TestNetworkAccessManagerFactory() {}

  QNetworkAccessManager* create(QObject* parent) final {
    return new TestNetworkAccessManager(test_dir_, parent);
  }

 private:
  QDir test_dir_;
};

// We don't use quick_test_main() here for running the qmltest binary as we want to
// be able to have a per-test datadir. However, some of quick_test_main() is
// duplicated here because we still use other bits of the QtQuickTest module

class QTestRootObject : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool windowShown READ windowShown NOTIFY windowShownChanged)
  Q_PROPERTY(bool hasTestCase READ hasTestCase WRITE setHasTestCase NOTIFY hasTestCaseChanged)

 public:
  QTestRootObject(QObject* parent = 0)
      : QObject(parent),
        has_quit_(false), window_shown_(false), has_test_case_(false) {}

  static QTestRootObject* instance() {
    static QPointer<QTestRootObject> object = new QTestRootObject();
    Q_ASSERT(object);
    return object;
  }

  bool has_quit_:1;
  bool hasTestCase() const { return has_test_case_; }
  void setHasTestCase(bool value) { has_test_case_ = value; emit hasTestCaseChanged(); }

  bool windowShown() const { return window_shown_; }
  void setWindowShown(bool value) { window_shown_ = value; emit windowShownChanged(); }

  void init() { setWindowShown(false); setHasTestCase(false); has_quit_ = false; }

 Q_SIGNALS:
  void windowShownChanged();
  void hasTestCaseChanged();

 private Q_SLOTS:
  void quit() { has_quit_ = true; }

 private:
  bool window_shown_:1;
  bool has_test_case_:1;
};

static QObject* GetTestRootObject(QQmlEngine* engine, QJSEngine* js_engine)
{
  Q_UNUSED(engine);
  Q_UNUSED(js_engine);
  return QTestRootObject::instance();
}

static void HandleCompileErrors(const QFileInfo& fi, QQuickView* view) {
  const QList<QQmlError> errors = view->errors();

  QuickTestResult results;
  results.setTestCaseName(fi.baseName());
  results.startLogging();
  results.setFunctionName(QLatin1String("compile"));

  QString message;
  QTextStream str(&message);
  str << "\n  " << QDir::toNativeSeparators(fi.absoluteFilePath()) << " produced "
      << errors.size() << " error(s):\n";
  for (QList<QQmlError>::const_iterator it = errors.begin();
       it != errors.end(); ++it) {
    const QQmlError& e = *it;
    str << "    ";
    if (e.url().isLocalFile()) {
      str << QDir::toNativeSeparators(e.url().toLocalFile());
    } else {
      str << e.url().toString();
    }
    if (e.line() > 0) {
      str << ':' << e.line() << ',' << e.column();
    }
    str << ": " << e.description() << '\n';
  }

  str << "  Working directory: "
      << QDir::toNativeSeparators(QDir::current().absolutePath()) << '\n';
  if (QQmlEngine *engine = view->engine()) {
    const QStringList import_paths = engine->importPathList();
    str << "  View: " << view->metaObject()->className() << ", import paths:\n";
    for (QStringList::const_iterator it = import_paths.begin();
         it != import_paths.end(); ++it) {
      str << "    '" << QDir::toNativeSeparators(*it) << "'\n";
    }
    const QStringList plugin_paths = engine->pluginPathList();
    str << "  Plugin paths:\n";
    for (QStringList::const_iterator it = plugin_paths.begin();
         it != plugin_paths.end(); ++it) {
      str << "    '" << QDir::toNativeSeparators(*it) << "'\n";
    }
  }

  qWarning("%s", qPrintable(message));

  results.fail(errors.at(0).description(),
               errors.at(0).url(), errors.at(0).line());
  results.finishTestData();
  results.finishTestDataCleanup();
  results.finishTestFunction();
  results.setFunctionName(QString());
  results.stopLogging();
}

static QString stripQuotes(const QString& in) {
  if (in.length() >= 2 && in.startsWith("\"") && in.endsWith("\"")) {
    return in.mid(1, in.length() - 2);
  }

  return in;
}

int main(int argc, char** argv) {
  QStringList imports;
  QStringList library_paths;
  QString test_path;
  QByteArray name;

  int index = 1;
  int outargc = 1;
  while (index < argc) {
    char* arg = argv[index];
    if (QLatin1String(arg) == QLatin1String("-import") && (index + 1) < argc) {
      imports.append(stripQuotes(QString::fromLatin1(argv[index + 1])));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("-input") && (index + 1) < argc) {
      if (!test_path.isEmpty()) {
        qFatal("Can only specify -input once");
      }
      test_path = stripQuotes(QString::fromLatin1(argv[index + 1]));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("-name") && (index + 1) < argc) {
      if (!name.isEmpty()) {
        qFatal("Can only specify -name once");
      }
      name = stripQuotes(QString::fromLatin1(argv[index + 1])).toLatin1();
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("-add-library-path") && (index + 1) < argc) {
      library_paths.append(stripQuotes(QString::fromLatin1(argv[index + 1])));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("-nss-db-path") && (index + 1) < argc) {
      if (!oxideGetNSSDbPath().isEmpty()) {
        qFatal("Can only specify -nss-db-path once");
      }
      oxideSetNSSDbPath(stripQuotes(QString::fromLatin1(argv[index + 1])));
      index += 2;
    } else if (index != outargc) {
      argv[outargc++] = argv[index++];
    } else {
      outargc++;
      index++;
    }
  }

  argv[outargc] = nullptr;

  QGuiApplication app(outargc, argv);

  QOpenGLContext context;
  context.create();
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
  qt_gl_set_global_share_context(&context);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
  QOpenGLContextPrivate::setGlobalShareContext(&context);
#else
  QSGContext::setSharedOpenGLContext(&context);
#endif

  for (int i = 0; i < library_paths.size(); ++i) {
    app.addLibraryPath(library_paths[i]);
  }

  QuickTestResult::setCurrentAppname(argv[0]);
  QuickTestResult::setProgramName(name.constData());
  QuickTestResult::parseArgs(outargc, argv);

  if (test_path.isEmpty()) {
    test_path = QDir::currentPath();
  }

  QString data_dir(qgetenv("OXIDE_RUNTESTS_TMPDIR"));
  if (data_dir.isEmpty()) {
    data_dir = QDir::currentPath();
  }

  QDir test_dir;
  QStringList files;

  QFileInfo test_path_info(test_path);
  if (test_path_info.isFile()) {
    test_dir = test_path_info.dir();
    if (!test_path.endsWith(".qml")) {
      qFatal("Test file '%s' does not end with '.qml'", qPrintable(test_path));
    }
    files.append(test_path);
  } else if (test_path_info.isDir()) {
    test_dir.cd(test_path);
    Q_ASSERT(test_path_info.isDir());
    const QStringList filters(QStringLiteral("tst_*.qml"));
    QDirIterator iter(test_path, filters, QDir::Files,
                      QDirIterator::Subdirectories |
                      QDirIterator::FollowSymlinks);
    while (iter.hasNext()) {
      files.append(iter.next());
    }
    files.sort();
    if (files.isEmpty()) {
      qFatal("Directory '%s' does not contain any test files",
             qPrintable(test_path));
    }
  } else {
    qFatal("Test file '%s' does not exist", qPrintable(test_path));
  }

  qmlRegisterSingletonType<QTestRootObject>(
      "Qt.test.qtestroot", 1, 0, "QTestRootObject", GetTestRootObject);

  QEventLoop event_loop;

  TestNetworkAccessManagerFactory nam_factory(test_dir);
  QQmlEngine engine;
  engine.setNetworkAccessManagerFactory(&nam_factory);

  QQuickView view(&engine, nullptr);
  view.setFlags(Qt::Window | Qt::WindowSystemMenuHint |
                Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint |
                Qt::WindowCloseButtonHint);

  QObject::connect(view.engine(), SIGNAL(quit()),
                   QTestRootObject::instance(), SLOT(quit()));
  QObject::connect(view.engine(), SIGNAL(quit()),
                   &event_loop, SLOT(quit()));

  for (QStringList::iterator it = imports.begin(); it != imports.end(); ++it) {
    view.engine()->addImportPath(*it);
  }

  for (QStringList::iterator it = files.begin(); it != files.end(); ++it) {
    const QFileInfo fi(*it);
    if (!fi.exists()) {
      continue;
    }

    QDir dir(data_dir);
    if (files.size() > 1) {
      dir = data_dir + QDir::separator() + fi.baseName();
    }
    view.rootContext()->setContextProperty(
        QStringLiteral("QMLTEST_DATADIR"),
        QUrl::fromLocalFile(dir.absolutePath()));

    view.setObjectName(fi.baseName());
    view.setTitle(view.objectName());

    QTestRootObject::instance()->init();

    QString path = fi.absoluteFilePath();
    view.setSource(QUrl::fromLocalFile(path));

    if (QTest::printAvailableFunctions) {
      continue;
    }

    if (view.status() == QQuickView::Error) {
      HandleCompileErrors(fi, &view);
      continue;
    }

    if (!QTestRootObject::instance()->has_quit_) {
      view.setFramePosition(QPoint(50, 50));
      if (view.size().isEmpty()) {
        qWarning().nospace() << "Test '" << QDir::toNativeSeparators(path) <<
                                "' has invalid size " << view.size() <<
                                ", resizing.";
        view.resize(200, 200);
      }
      view.show();
      view.requestActivate();
      QTest::qWaitForWindowExposed(&view);
      if (view.isExposed()) {
        QTestRootObject::instance()->setWindowShown(true);
      }
      if (!QTestRootObject::instance()->has_quit_ &&
          QTestRootObject::instance()->hasTestCase()) {
        event_loop.exec();
      }
    }
  }

  QuickTestResult::setProgramName(nullptr);
  return QuickTestResult::exitCode();
}

#include "main.moc"
