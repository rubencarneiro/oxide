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
#include <QLatin1String>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QString>
#include <QtGlobal>
#include <QUrl>
#include <QVariant>

class OxideQmlTestingPlugin : public QQmlExtensionPlugin {
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface" FILE "oxide_qml_testing_plugin.json")
  Q_OBJECT
 public:
  void initializeEngine(QQmlEngine* engine, const char* uri) {
    Q_ASSERT(QLatin1String(uri) == QLatin1String("com.canonical.Oxide.Testing"));

    QUrl url;
    QString path(QString(qgetenv("OXIDE_TESTING_DATA_PATH")));
    if (!path.isEmpty()) {
      QDir dir(path);
      url = QUrl::fromLocalFile(dir.absolutePath());
    }
    engine->rootContext()->setContextProperty("OXIDE_TESTING_DATA_PATH",
                                              QVariant(url));
  }

  void registerTypes(const char* uri) {
    Q_UNUSED(uri);
  }
};

#include "oxide_qml_testing_plugin.moc"
