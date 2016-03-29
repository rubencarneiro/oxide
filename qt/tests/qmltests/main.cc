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
#include <QFileInfo>
#include <QGuiApplication>
#include <QLatin1String>
#include <QList>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
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

static QObject* GetUtils(QQmlEngine* engine, QJSEngine* js_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(js_engine);

  return new OxideTestingUtils();
}

static QObject* GetClipboardTestUtils(QQmlEngine* engine,
                                      QJSEngine* js_engine) {
  Q_UNUSED(engine);
  Q_UNUSED(js_engine);

  return new ClipboardTestUtils();
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
  QString plugin_path;
  QString test_path;
  QByteArray name;
  QString data_dir;

  int index = 1;
  int outargc = 1;
  while (index < argc) {
    char* arg = argv[index];
    if (QLatin1String(arg) == QLatin1String("--input") && (index + 1) < argc) {
      if (!test_path.isEmpty()) {
        qFatal("Can only specify --input once");
      }
      test_path = stripQuotes(QString::fromLatin1(argv[index + 1]));
      index += 2;
    } else if (QLatin1String(arg) == QLatin1String("--name") && (index + 1) < argc) {
      if (!name.isEmpty()) {
        qFatal("Can only specify --name once");
      }
      name = stripQuotes(QString::fromLatin1(argv[index + 1])).toLatin1();
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
      if (!data_dir.isEmpty()) {
        qFatal("Can only specify --tmpdir once");
      }
      data_dir = stripQuotes(QString::fromLatin1(argv[index + 1]));
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

  if (!plugin_path.isEmpty()) {
    app.addLibraryPath(plugin_path);
  }

  QuickTestResult::setCurrentAppname(argv[0]);
  QuickTestResult::setProgramName(name.constData());
  QuickTestResult::parseArgs(outargc, argv);

  if (test_path.isEmpty()) {
    test_path = QDir::currentPath();
  }

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

  qmlRegisterSingletonType<OxideTestingUtils>(
      "Oxide.testsupport", 1, 0, "Utils", GetUtils);
  qmlRegisterSingletonType<ClipboardTestUtils>(
      "Oxide.testsupport", 1, 0, "ClipboardTestUtils", GetClipboardTestUtils);
  qmlRegisterUncreatableType<QObjectTestHelper>(
      "Oxide.testsupport", 1, 0, "QObjectTestHelper",
      "Create this with Utils.createQObjectTestHelper()");
  qmlRegisterType<ExternalProtocolHandler>(
      "Oxide.testsupport", 1, 0, "ExternalProtocolHandler");
  qmlRegisterUncreatableType<WebContextTestSupport>(
      "Oxide.testsupport", 1, 0, "WebContextTestSupport",
      "Create this with Utils.createWebContextTestSupport()");

  qmlRegisterSingletonType<OxideTestingUtils>(
      "Oxide.testsupport.hack", 1, 0, "Utils", GetUtils);

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

    QTestRootObject::instance()->reset();

    QString path = fi.absoluteFilePath();
    view.setSource(QUrl::fromLocalFile(path));

    if (QTest::printAvailableFunctions) {
      continue;
    }

    if (view.status() == QQuickView::Error) {
      HandleCompileErrors(fi, &view);
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
      if (!QTestRootObject::instance()->hasQuit() &&
          QTestRootObject::instance()->hasTestCase()) {
        event_loop.exec();
      }
    }
  }

  QuickTestResult::setProgramName(nullptr);
  return QuickTestResult::exitCode();
}
