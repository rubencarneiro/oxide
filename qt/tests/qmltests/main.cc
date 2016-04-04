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

// We don't use quick_test_main() here for running the qmltest binary as we
// want to be able to have a per-test datadir. However, some of
// quick_test_main() is duplicated here because we still use other bits of the
// QtQuickTest module

static QObject* GetTestRootObject(QQmlEngine* engine, QJSEngine* js_engine)
{
  Q_UNUSED(engine);
  Q_UNUSED(js_engine);

  return QTestRootObject::instance();
}

static QObject* GetTestSupport(QQmlEngine* engine, QJSEngine* js_engine) {
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

QJSValue BuildTestConstants(QJSEngine* engine, bool single_process) {
  QJSValue constants = engine->newObject();
  constants.setProperty(QStringLiteral("SINGLE_PROCESS"), single_process);
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

QObject* CreateTestWebContext(const QUrl& data_url, QQmlEngine* engine) {
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

QSet<QString> LoadExcludeList(const QString& path) {
  QSet<QString> rv;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    qFatal("Failed to open exclude list '%s'", qPrintable(path));
    return rv;
  }

  QTextStream stream(&file);

  while (!stream.atEnd()) {
    QString line = stream.readLine();
    rv.insert(line);
  }

  return rv;
}

static QString stripQuotes(const QString& in) {
  if (in.length() >= 2 && in.startsWith("\"") && in.endsWith("\"")) {
    return in.mid(1, in.length() - 2);
  }

  return in;
}

int main(int argc, char** argv) {
  QString test_path(QLatin1String(QML_TEST_PATH));

  QString plugin_path;
  QString import_path;
  QString tmp_path;
  QString test_name;

  QStringList test_file_names;

  QString exclude_list_file_path;

  bool single_process = false;

  int index = 1;
  int outargc = 1;
  while (index < argc) {
    char* arg = argv[index];
    if (QLatin1String(arg) == QLatin1String("--qml-import-path") && (index + 1) < argc) {
      if (!import_path.isEmpty()) {
        qFatal("Can only specify --qml-import-path once");
      }
      import_path = stripQuotes(QString::fromLatin1(argv[index + 1]));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("--qt-plugin-path") && (index + 1) < argc) {
      if (!plugin_path.isEmpty()) {
        qFatal("Can only specify --qt-plugin-path once");
      }
      plugin_path = stripQuotes(QString::fromLatin1(argv[index + 1]));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("--nss-db-path") && (index + 1) < argc) {
      if (!oxideGetNSSDbPath().isEmpty()) {
        qFatal("Can only specify --nss-db-path once");
      }
      oxideSetNSSDbPath(stripQuotes(QString::fromLatin1(argv[index + 1])));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("--tmpdir") && (index + 1) < argc) {
      if (!tmp_path.isEmpty()) {
        qFatal("Can only specify --tmpdir once");
      }
      tmp_path = stripQuotes(QString::fromLatin1(argv[index + 1]));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("--name") && (index + 1) < argc) {
      if (!test_name.isEmpty()) {
        qFatal("Can only specify --name once");
      }
      test_name = stripQuotes(QString::fromLatin1(argv[index + 1]));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("--file") && (index + 1) < argc) {
      test_file_names.append(stripQuotes(QString::fromLatin1(argv[index + 1])));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("--exclude-list") && (index + 1) < argc) {
      if (!exclude_list_file_path.isEmpty()) {
        qFatal("Can only specify --exclude-list once");
      }
      exclude_list_file_path = stripQuotes(QString::fromLatin1(argv[index + 1]));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("--single-process")) {
      single_process = true;
      index += 1;
    } else if (index != outargc) {
      argv[outargc++] = argv[index++];
    } else {
      outargc++;
      index++;
    }
  }

  if (test_name.isEmpty()) {
    qFatal("Didn't specify a test name!");
  }

  argv[outargc] = nullptr;

  QGuiApplication app(outargc, argv);

  if (single_process) {
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

  if (!plugin_path.isEmpty()) {
    app.addLibraryPath(plugin_path);
  }

  QuickTestResult::setCurrentAppname(argv[0]);
  QByteArray test_name_ba = test_name.toUtf8();
  QuickTestResult::setProgramName(test_name_ba.constData());
  QuickTestResult::parseArgs(outargc, argv);

  if (!QFile::exists(test_path)) {
    test_path = QCoreApplication::applicationDirPath();
  }

  if (tmp_path.isEmpty()) {
    tmp_path = QDir::currentPath();
  }

  QDir test_dir;
  QStringList files;

  QFileInfo test_path_info(test_path);
  if (test_path_info.isFile()) {
    if (!test_file_names.isEmpty()) {
      qFatal("--file option is invalid for this test");
    }
    test_dir = test_path_info.dir();
    if (!test_path.endsWith(".qml")) {
      qFatal("Test file '%s' does not end with '.qml'", qPrintable(test_path));
    }
    files.append(test_path);
  } else if (!test_file_names.isEmpty()) {
    Q_ASSERT(test_path_info.isDir());
    test_dir.cd(test_path);

    for (const auto& file_name : test_file_names) {
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
    test_dir.cd(test_path);

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

  QSet<QString> exclude_list;
  if (!exclude_list_file_path.isEmpty()) {
    exclude_list = LoadExcludeList(exclude_list_file_path);
  }

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
      "Create this with TestSupport.createWebContextTestSupport()");
  qmlRegisterUncreatableType<WebViewTestSupport>(
      "Oxide.testsupport", 1, 0, "WebViewTestSupport",
      "Create this with TestSupport.createWebViewTestSupport()");

  qmlRegisterSingletonType<TestSupport>(
      "Oxide.testsupport.hack", 1, 0, "TestSupport", GetTestSupport);

  QEventLoop event_loop;

  // |num_factory| should outlive |engine|
  TestNetworkAccessManagerFactory nam_factory(test_dir);

  QQmlEngine engine;
  engine.setNetworkAccessManagerFactory(&nam_factory);

  if (!import_path.isEmpty()) {
    engine.addImportPath(import_path);
  }

  QJSValue test_constants = BuildTestConstants(&engine, single_process);
  engine.rootContext()->setContextProperty(
      QStringLiteral("TestConstants"),
      QVariant::fromValue(test_constants));

  QScopedPointer<QObject> single_process_web_context;
  if (single_process) {
    QDir tmp_dir(tmp_path);
    single_process_web_context.reset(
        CreateTestWebContext(QUrl::fromLocalFile(tmp_dir.absolutePath()),
                             &engine));
    engine.rootContext()->setContextProperty(
        QStringLiteral("SingletonTestWebContext"),
        single_process_web_context.data());
  }

  QQuickView view(&engine, nullptr);
  view.setFlags(Qt::Window | Qt::WindowSystemMenuHint |
                Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint |
                Qt::WindowCloseButtonHint);

  QObject::connect(view.engine(), SIGNAL(quit()),
                   QTestRootObject::instance(), SLOT(quit()));
  QObject::connect(view.engine(), SIGNAL(quit()),
                   &event_loop, SLOT(quit()));

  for (QStringList::iterator it = files.begin(); it != files.end(); ++it) {
    const QFileInfo fi(*it);
    if (!fi.exists()) {
      continue;
    }

    if (exclude_list.contains(fi.fileName())) {
      continue;
    }

    QDir tmp_dir(tmp_path);
    if (files.size() > 1) {
      tmp_dir = tmp_path + QDir::separator() + fi.fileName();
    }

    test_constants.setProperty(
        QStringLiteral("TMPDIR"),
        engine.toScriptValue(QUrl::fromLocalFile(tmp_dir.absolutePath())));

    QScopedPointer<QObject> test_web_context;
    if (!single_process) {
      test_web_context.reset(
          CreateTestWebContext(QUrl::fromLocalFile(tmp_dir.absolutePath()),
                               &engine));
      engine.rootContext()->setContextProperty(
          QStringLiteral("SingletonTestWebContext"),
          test_web_context.data());
    }

    view.setObjectName(fi.baseName());
    view.setTitle(view.objectName());

    QTestRootObject::instance()->reset();

    QString path = fi.absoluteFilePath();
    view.setSource(QUrl::fromLocalFile(path));

    if (QTest::printAvailableFunctions) {
      continue;
    }

    if (view.status() == QQuickView::Error) {
      RecordCompileError(fi, &view);
      continue;
    }

    if (!QTestRootObject::instance()->hasQuit()) {
      view.setFramePosition(QPoint(50, 50));
      if (view.size().isEmpty()) {
        qWarning().nospace() <<
            "Test '" << QDir::toNativeSeparators(path) << "' has invalid "
            "size " << view.size() << ", resizing.";
        view.resize(200, 200);
      }
      view.show(); 
      if (!QTest::qWaitForWindowExposed(&view)) {
        qWarning().nospace() <<
            "Test '" << QDir::toNativeSeparators(path) << "' window not "
            "exposed after show().";
      }
      view.requestActivate();
      if (!QTest::qWaitForWindowActive(&view)) {
        qWarning().nospace() <<
            "Test '" << QDir::toNativeSeparators(path) << "' window not active "
            "after requestActivate().";
      }
      if (view.isExposed()) {
        QTestRootObject::instance()->setWindowShown(true);
      } else {
        qWarning().nospace() <<
            "Test '" << QDir::toNativeSeparators(path) << "' window was never "
            "exposed! If the test case was expecting windowShown, it will "
            "hang.";
      }

      if (QTestRootObject::instance()->hasQuit()) {
        continue;
      }

      if (!QTestRootObject::instance()->hasTestCase()) {
        continue;
      }

      event_loop.exec();
    }
  }

  QuickTestResult::setProgramName(nullptr);
  return QuickTestResult::exitCode();
}
