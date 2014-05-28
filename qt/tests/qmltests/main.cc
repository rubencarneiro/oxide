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

#include <QGuiApplication>
#include <QLatin1String>
#include <QString>
#include <QtGlobal>
#include <QtQuickTest/quicktest.h>
#include <QtQuickVersion>
#if defined(ENABLE_COMPOSITING)
#include <QOpenGLContext>
#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
#include <QtQuick/private/qsgcontext_p.h>
#else
#include <QtGui/private/qopenglcontext_p.h>
#endif
#endif

static QString stripQuotes(const QString& in) {
  if (in.length() >= 2 && in.startsWith("\"") && in.endsWith("\"")) {
    return in.mid(1, in.length() - 2);
  }

  return in;
}

int main(int argc, char** argv) {
  char** filtered_argv = new char*[argc + 1];
  int filtered_argc = 1;
  filtered_argv[0] = argv[0];

  QString name;

  int index = 1;
  while (index < argc) {
    char* arg = argv[index];
    if (QLatin1String(arg) == QLatin1String("-name") && (index + 1) < argc) {
      name = stripQuotes(QString::fromLatin1(argv[index + 1]));
      index += 2;
    } else {
      filtered_argv[filtered_argc] = arg;
      ++filtered_argc;
      ++index;
    }
  }

  filtered_argv[filtered_argc] = NULL;

  QGuiApplication app(filtered_argc, filtered_argv);

#if defined(ENABLE_COMPOSITING)
  QOpenGLContext context;
  context.create();
#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
  QSGContext::setSharedOpenGLContext(&context);
#else
  QOpenGLContextPrivate::setGlobalShareContext(&context);
#endif
#endif

  return quick_test_main(filtered_argc, filtered_argv,
                         name.toUtf8().constData(), NULL);
}
