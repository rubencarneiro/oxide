// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.
// Copyright (C) 2013 Digia Plc. and/or its subsidiary(-ies)

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

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWindow>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <Qt>
#include <QtQuickVersion>
#include <QUrl>

#include "QtCore/private/qabstractanimation_p.h"
#if QTQUICK_VERSION >= 0x050200
#include <QOpenGLContext>
#include "QtQuick/private/qsgcontext_p.h"
#endif

void usage() {
  qWarning("Usage: oxideqmlscene [options] <filename>");
  qWarning(" ");
  qWarning(" Options:");
  qWarning("  --maximized ..................... Run maximized");
  qWarning("  --fullscreen .................... Run fullscreen");
  qWarning("  --slow-animations ............... Run all animations in slow motion");
  qWarning("  --quit .......................... Quit immediately after starting");
  qWarning("  --resize-to-root ................ Resize the window to the size of the root item");

  qWarning(" ");
  exit(1);
}

struct Options {
  Options() :
      maximized(false),
      fullscreen(false),
      slowAnimations(false),
      quit(false),
      resizeToRoot(false) {}

  QUrl file;
  bool maximized;
  bool fullscreen;
  bool slowAnimations;
  bool quit;
  bool resizeToRoot;
};

int main(int argc, char** argv) {
  Options options;

  QStringList imports;
  for (int i = 1; i < argc; ++i) {
    char* arg = argv[i];
    if (arg[0] != '-' && QFileInfo(QFile::decodeName(arg)).exists()) {
      options.file = QUrl::fromLocalFile(arg);
    } else {
      const QString larg = QString::fromLatin1(arg).toLower();
      if (larg == QLatin1String("--maximized")) {
        options.maximized = true;
      } else if (larg == QLatin1String("--fullscreen")) {
        options.fullscreen = true;
      } else if (larg == QLatin1String("--slow-animations")) {
        options.slowAnimations = true;
      } else if (larg == QLatin1String("--quit")) {
        options.quit = true;
      } else if (larg == QLatin1String("--resize-to-root")) {
        options.resizeToRoot = true;
      } else if (larg == QLatin1String("-i") && i + 1 < argc) {
        imports.append(QString::fromLatin1(argv[++i]));
      } else {
        usage();
      }
    }
  }

  if (options.file.isEmpty()) {
    usage();
  }

  QGuiApplication app(argc, argv);
  app.setApplicationName("OxideQmlViewer");

#if QTQUICK_VERSION >= 0x050200
  QOpenGLContext glcontext;
  glcontext.create();
  QSGContext::setSharedOpenGLContext(&glcontext);
#endif

  QUnifiedTimer::instance()->setSlowModeEnabled(options.slowAnimations);

  QQmlEngine engine;
  QQmlComponent component(&engine);
  for (int i = 0; i < imports.size(); ++i) {
    engine.addImportPath(imports.at(i));
  }

  QObject::connect(&engine, SIGNAL(quit()), QCoreApplication::instance(), SLOT(quit()));

  component.loadUrl(options.file);
  if (!component.isReady()) {
    qWarning("%s", qPrintable(component.errorString()));
    return -1;
  }

  QObject* toplevel = component.create();
  if (!toplevel && component.isError()) {
    qWarning("%s", qPrintable(component.errorString()));
    return -1;
  }

  QScopedPointer<QQuickWindow> window(qobject_cast<QQuickWindow *>(toplevel));
  if (window) {
    engine.setIncubationController(window->incubationController());
  } else {
    QQuickItem* contentItem = qobject_cast<QQuickItem *>(toplevel);
    if (contentItem) {
      QQuickView* view = new QQuickView(&engine, NULL);
      window.reset(view);

      QString name = contentItem->objectName();
      window->setTitle(
          name.isEmpty() ?
            QString::fromLatin1("oxideqmlscene") :
            QString::fromLatin1("oxideqmlscene: ") + name);

      if (options.resizeToRoot) {
        view->setResizeMode(QQuickView::SizeViewToRootObject);
      } else {
        view->setResizeMode(QQuickView::SizeRootObjectToView);
      }

      view->setContent(options.file, &component, contentItem);
    }
  }

  if (window->flags() == Qt::Window) {
    window->setFlags(Qt::Window |
                     Qt::WindowSystemMenuHint |
                     Qt::WindowTitleHint |
                     Qt::WindowMinMaxButtonsHint |
                     Qt::WindowCloseButtonHint |
                     Qt::WindowFullscreenButtonHint);
  }

  if (options.fullscreen) {
    window->showFullScreen();
  } else if (options.maximized) {
    window->showMaximized();
  } else if (!window->isVisible()) {
    window->show();
  }

  if (options.quit) {
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
  }

  return app.exec();
}
