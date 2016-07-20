// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.
// Copyright (C) 2015 The Qt Company Ltd.

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
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLatin1String>
#include <QList>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickView>
#include <QScreen>
#include <QSet>
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

#include "qml_test_support.h"
#include "quick_test_compat.h"
#include "test_nam_factory.h"

Q_DECLARE_METATYPE(QScreen*)
Q_DECLARE_METATYPE(QList<QScreen*>)

// We don't use quick_test_main() here for running the qmltest binary as we
// want to be able to have a per-test datadir. However, some of
// quick_test_main() is duplicated here because we still use other bits of the
// QtQuickTest module

struct Options {
  QString import_path;
  QString plugin_path;
  QString nss_db_path;
  QString tmp_path;
  QString name;
  QStringList test_file_names;
  QString exclude_list_path;
  bool single_process = false;
};

static QString StripQuotes(const QString& in) {
  if (in.length() >= 2 && in.startsWith("\"") && in.endsWith("\"")) {
    return in.mid(1, in.length() - 2);
  }

  return in;
}

static Options ParseOptions(int& argc, char** argv) {
  Options options;

  int index = 1;
  int outargc = 1;

  while (index < argc) {
    auto handle_string_option = [&index, argc, argv](const char* option,
                                                     QString* out) {
      if (QLatin1String(argv[index]) == QLatin1String(option) &&
          (index + 1) < argc) {
        if (!out->isEmpty()) {
          qFatal("Can't specify %s more than once", option);
        }
        *out = StripQuotes(QString::fromLatin1(argv[index + 1]));
        index += 2;
        return true;
      }
      return false;
    };

    auto handle_stringlist_option = [&index, argc, argv](const char* option,
                                                         QStringList* out) {
      if (QLatin1String(argv[index]) == QLatin1String(option) &&
          (index + 1) < argc) {
        out->push_back(StripQuotes(QString::fromLatin1(argv[index + 1])));
        index += 2;
        return true;
      }
      return false;
    };

    auto handle_bool_option = [&index, argv](const char* option,
                                             bool* out) {
      if (QLatin1String(argv[index]) == QLatin1String(option)) {
        *out = true;
        ++index;
        return true;
      }
      return false;
    };

    if (handle_string_option("--qml-import-path", &options.import_path)) {
      continue;
    }
    if (handle_string_option("--qt-plugin-path", &options.plugin_path)) {
      continue;
    }
    if (handle_string_option("--nss-db-path", &options.nss_db_path)) {
      continue;
    }
    if (handle_string_option("--tmpdir", &options.tmp_path)) {
      continue;
    }
    if (handle_string_option("--name", &options.name)) {
      continue;
    }
    if (handle_stringlist_option("--file", &options.test_file_names)) {
      continue;
    }
    if (handle_string_option("--exclude-list", &options.exclude_list_path)) {
      continue;
    }
    if (handle_bool_option("--single-process", &options.single_process)) {
      continue;
    }

    if (index != outargc) {
      argv[outargc++] = argv[index++];
    } else {
      outargc++;
      index++;
    }
  }

  argv[outargc] = nullptr;
  argc = outargc;

  return std::move(options);
}

static QObject* GetTestRootObject(QQmlEngine* engine, QJSEngine* js_engine)
{
  Q_UNUSED(engine);
  Q_UNUSED(js_engine);

  return QTestRootObject::instance();
}

static QObject* GetTestSupport(QQmlEngine* engine, QJSEngine* js_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(js_engine);

  return TestSupport::instance();
}

static QObject* GetTestSupportHack(QQmlEngine* engine, QJSEngine* js_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(js_engine);

  return new TestSupport();
}

static QObject* GetClipboardTestUtils(QQmlEngine* engine,
                                      QJSEngine* js_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(js_engine);

  return new ClipboardTestUtils();
}

static void RegisterQmlTypes() {
  qRegisterMetaType<QScreen*>();
  qRegisterMetaType<QList<QScreen*>>();

  qmlRegisterSingletonType<QTestRootObject>(
      "Qt.test.qtestroot", 1, 0, "QTestRootObject", GetTestRootObject);

  qmlRegisterSingletonType<TestSupport>(
      "Oxide.testsupport", 1, 0, "TestSupport", GetTestSupport);
  qmlRegisterSingletonType<ClipboardTestUtils>(
      "Oxide.testsupport", 1, 0, "ClipboardTestUtils", GetClipboardTestUtils);
  qmlRegisterUncreatableType<QObjectTestHelper>(
      "Oxide.testsupport", 1, 0, "QObjectTestHelper",
      "Create this with TestSupport.createQObjectTestHelper()");
  qmlRegisterType<ExternalProtocolHandler>(
      "Oxide.testsupport", 1, 0, "ExternalProtocolHandler");
  qmlRegisterUncreatableType<WebContextTestSupport>(
      "Oxide.testsupport", 1, 0, "WebContextTestSupport",
      "WebContextTestSupport only exists to provide attached properties");
  qmlRegisterUncreatableType<WebViewTestSupport>(
      "Oxide.testsupport", 1, 0, "WebViewTestSupport",
      "WebViewTestSupport only exists to provide attached properties");
  qmlRegisterUncreatableType<TestWindow>(
      "Oxide.testsupport", 1, 0, "TestWindow",
      "TestWindow only exists to provide attached properties");
  qmlRegisterUncreatableType<ItemTestSupport>(
      "Oxide.testsupport", 1, 0, "ItemTestSupport",
      "ItemTestSupport only exists to provide attached properties");

  qmlRegisterSingletonType<TestSupport>(
      "Oxide.testsupport.hack", 1, 0, "TestSupport", GetTestSupportHack);
}

static QJSValue CreateCommonTestConstants(QQmlEngine* engine,
                                          const Options& options) {
  QJSValue constants = engine->newObject();
  constants.setProperty(QStringLiteral("SINGLE_PROCESS"),
                        options.single_process);
  return constants;
}

static void RecordCompileError(const QString& source,
                               const QList<QQmlError>& errors,
                               QQmlEngine* engine) {
  QuickTestResult results;
  results.setTestCaseName(source);
  results.startLogging();
  results.setFunctionName(QLatin1String("compile"));

  QString message;
  QTextStream str(&message);

  str << "\n  " << source << " produced " << errors.size() << " error(s):\n";

  for (const auto& error : errors) {
    str << "    ";
    if (error.url().isLocalFile()) {
      str << QDir::toNativeSeparators(error.url().toLocalFile());
    } else {
      str << error.url().toString();
    }
    if (error.line() > 0) {
      str << ':' << error.line() << ',' << error.column();
    }
    str << ": " << error.description() << '\n';
  }

  str << "  Working directory: "
      << QDir::toNativeSeparators(QDir::current().absolutePath()) << '\n';

  const QStringList& import_paths = engine->importPathList();
  str << "  Import paths:\n";
  for (const auto& import_path : import_paths) {
    str << "    '" << QDir::toNativeSeparators(import_path) << "'\n";
  }

  const QStringList& plugin_paths = engine->pluginPathList();
  str << "  Plugin paths:\n";
  for (const auto& plugin_path : plugin_paths) {
    str << "    '" << QDir::toNativeSeparators(plugin_path) << "'\n";
  }

  qWarning("%s", qPrintable(message));

  results.fail(errors.at(0).description(),
               errors.at(0).url(),
               errors.at(0).line());
  results.finishTestData();
  results.finishTestDataCleanup();
  results.finishTestFunction();
  results.setFunctionName(QString());
  results.stopLogging();
}

static void RecordCompileError(const QFileInfo& fi, QQuickView* view) {
  RecordCompileError(fi.baseName(), view->errors(), view->engine());
}

static QObject* CreateTestWebContext(const QUrl& data_url, QQmlEngine* engine) {
  QString component_data("import Oxide.testsupport 1.0\nTestWebContext {\n");
  component_data.append("  dataPath: \"");
  component_data.append(data_url.toString());
  component_data.append("\"\n}");

  QQmlComponent component(engine);
  component.setData(component_data.toUtf8(), QUrl());

  if (!component.isReady()) {
    RecordCompileError(QString(), component.errors(), engine);
    qFatal("Failed to create component for TestWebContext. It's not possible "
           "to run the tests!");
  }

  QObject* context = component.create(engine->rootContext());
  QQmlEngine::setObjectOwnership(context, QQmlEngine::CppOwnership);

  return context;
}

static QSet<QString> LoadExcludeList(const Options& options) {
  QSet<QString> rv;

  if (options.exclude_list_path.isEmpty()) {
    return rv;
  }

  QFile file(options.exclude_list_path);
  if (!file.open(QIODevice::ReadOnly)) {
    qFatal("Failed to open exclude list '%s'",
           qPrintable(options.exclude_list_path));
    return rv;
  }

  QTextStream stream(&file);

  while (!stream.atEnd()) {
    QString line = stream.readLine();
    rv.insert(line);
  }

  return rv;
}

static bool WaitForSizeToBeAllocatedForRootObject(QQuickView* view) {
  QQuickItem* root = view->rootObject();

  QElapsedTimer timer;
  timer.start();

  while (root->width() < 10.f || root->height() < 10.f) {
    int remaining = 5000 - timer.elapsed();
    if (remaining <= 0) {
      return false;
    }
 
    QCoreApplication::processEvents(QEventLoop::AllEvents, remaining);
    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    QTest::qSleep(10);
  }

  return !(root->width() < 10.f || root->height() < 10.f);
}

static QStringList BuildTestList(const QString& test_path,
                                 const Options& options) {
  QStringList files;

  QFileInfo test_path_info(test_path);

  if (test_path_info.isFile()) {
    if (!options.test_file_names.isEmpty()) {
      qFatal("--file option is invalid for this test");
    }
    if (!test_path.endsWith(".qml")) {
      qFatal("Test file '%s' does not end with '.qml'", qPrintable(test_path));
    }
    files.append(test_path);
  } else if (!options.test_file_names.isEmpty()) {
    Q_ASSERT(test_path_info.isDir());
    QDir test_dir(test_path);

    for (const auto& file_name : options.test_file_names) {
      if (!file_name.endsWith(".qml")) {
        qFatal("Test file '%s' does not end with '.qml'", qPrintable(file_name));
      }
      QFileInfo file_info(test_dir, file_name);
      if (!file_info.isFile()) {
        qFatal("Test file '%s' does not exist", qPrintable(file_name));
      }
      files.append(file_info.absoluteFilePath());
    }
  } else {
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
  }

  return files;
}

static void RunTest(QQuickView* view,
                    const QFileInfo& file_info,
                    const QDir& tmp_dir,
                    QJSValue* test_constants,
                    QObject* test_web_context) {
  QQmlEngine* engine = view->engine();

  test_constants->setProperty(
      QStringLiteral("TMPDIR"),
      engine->toScriptValue(QUrl::fromLocalFile(tmp_dir.absolutePath())));

  QScopedPointer<QObject> local_test_web_context;
  if (!test_web_context) {
    local_test_web_context.reset(
        CreateTestWebContext(QUrl::fromLocalFile(tmp_dir.absolutePath()),
                             engine));
    test_web_context = local_test_web_context.data();
  }
  engine->rootContext()->setContextProperty(
      QStringLiteral("SingletonTestWebContext"),
      test_web_context);

  view->setResizeMode(QQuickView::SizeViewToRootObject);
  view->setObjectName(file_info.baseName());
  view->setTitle(view->objectName());

  QTestRootObject::instance()->reset();
  TestSupport::instance()->reset();

  QString path = file_info.absoluteFilePath();
  view->setSource(QUrl::fromLocalFile(path));

  if (QTest::printAvailableFunctions) {
    return;
  }

  if (view->status() == QQuickView::Error) {
    RecordCompileError(file_info, view);
    return;
  }

  if (QTestRootObject::instance()->hasQuit()) {
    return;
  }

  TestSupport::instance()->setTestLoaded(true);

  if (QTestRootObject::instance()->hasQuit()) {
    return;
  }
  
  view->setFramePosition(QPoint(50, 50));
  if (view->initialSize().isEmpty()) {
    view->resize(600, 400);
    view->setResizeMode(QQuickView::SizeRootObjectToView);
  }

  view->show(); 
  if (!QTest::qWaitForWindowExposed(view)) {
    qWarning().nospace() <<
        "Test '" << QDir::toNativeSeparators(path) << "' window not "
        "exposed after show().";
  }

  if (view->resizeMode() == QQuickView::SizeRootObjectToView &&
      !WaitForSizeToBeAllocatedForRootObject(view)) {
    qWarning().nospace() <<
        "Test '" << QDir::toNativeSeparators(path) << "' root item not "
        "greater than 0x0 after resize().";
  }

  view->requestActivate();
  if (!QTest::qWaitForWindowActive(view)) {
    qWarning().nospace() <<
        "Test '" << QDir::toNativeSeparators(path) << "' window not active "
        "after requestActivate().";
  }

  if (view->isExposed()) {
    QTestRootObject::instance()->setWindowShown(true);
  } else {
    qWarning().nospace() <<
        "Test '" << QDir::toNativeSeparators(path) << "' window was never "
        "exposed! If the test case was expecting windowShown, it will "
        "hang.";
  }

  if (QTestRootObject::instance()->hasQuit()) {
    return;
  }

  if (!QTestRootObject::instance()->hasTestCase()) {
    return;
  }

  QEventLoop event_loop;
  QObject::connect(view->engine(), SIGNAL(quit()),
                   &event_loop, SLOT(quit()));
  event_loop.exec();
}

int main(int argc, char** argv) {
  QString test_path(QML_TEST_PATH);

  Options options = ParseOptions(argc, argv);

  if (options.name.isEmpty()) {
    qFatal("Didn't specify a test name!");
  }

  QGuiApplication app(argc, argv);

  if (!options.nss_db_path.isEmpty()) {
    oxideSetNSSDbPath(options.nss_db_path);
  }
  if (!options.plugin_path.isEmpty()) {
    app.addLibraryPath(options.plugin_path);
  }
  if (options.single_process) {
    oxideSetProcessModel(OxideProcessModelSingleProcess);
  }

  QOpenGLContext context;
  context.create();
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
  qt_gl_set_global_share_context(&context);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
  QOpenGLContextPrivate::setGlobalShareContext(&context);
#else
  QSGContext::setSharedOpenGLContext(&context);
#endif

  QuickTestResult::setCurrentAppname(argv[0]);
  QByteArray test_name_ba = options.name.toUtf8();
  QuickTestResult::setProgramName(test_name_ba.constData());
  QuickTestResult::parseArgs(argc, argv);

  if (!QFile::exists(test_path)) {
    test_path = QCoreApplication::applicationDirPath();
  }

  if (options.tmp_path.isEmpty()) {
    options.tmp_path = QDir::currentPath();
  }

  QDir test_dir;
  {
    QFileInfo fi(test_path);
    if (fi.isDir()) {
      test_dir.cd(fi.absoluteFilePath());
    } else {
      test_dir = fi.absoluteDir();
    }
  }

  QStringList files = BuildTestList(test_path, options);

  QSet<QString> exclude_list = LoadExcludeList(options);

  RegisterQmlTypes();

  // |num_factory| should outlive |engine|
  TestNetworkAccessManagerFactory nam_factory(test_dir);

  QQmlEngine engine;
  engine.setNetworkAccessManagerFactory(&nam_factory);

  if (!options.import_path.isEmpty()) {
    engine.addImportPath(options.import_path);
  }

  QJSValue test_constants = CreateCommonTestConstants(&engine, options);
  engine.rootContext()->setContextProperty(
      QStringLiteral("TestConstants"),
      QVariant::fromValue(test_constants));

  QScopedPointer<QObject> single_process_web_context;
  if (options.single_process) {
    QDir tmp_dir(options.tmp_path);
    single_process_web_context.reset(
        CreateTestWebContext(QUrl::fromLocalFile(tmp_dir.absolutePath()),
                             &engine));
  }

  QQuickView view(&engine, nullptr);
  view.setFlags(Qt::Window | Qt::WindowSystemMenuHint |
                Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint |
                Qt::WindowCloseButtonHint);

  QObject::connect(view.engine(), SIGNAL(quit()),
                   QTestRootObject::instance(), SLOT(quit()));

  for (QStringList::iterator it = files.begin(); it != files.end(); ++it) {
    const QFileInfo fi(*it);
    if (!fi.exists()) {
      continue;
    }

    if (exclude_list.contains(fi.fileName())) {
      continue;
    }

    QDir tmp_dir(options.tmp_path);
    if (files.size() > 1) {
      tmp_dir = options.tmp_path + QDir::separator() + fi.fileName();
    }

    RunTest(&view, fi, tmp_dir, &test_constants,
            single_process_web_context.data());
  }

  QuickTestResult::setProgramName(nullptr);
  return QuickTestResult::exitCode();
}
