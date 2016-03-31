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

#include "test_nam_factory.h"

#include <QFileInfo>
#include <QLatin1String>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>

class TestNetworkAccessManager : public QNetworkAccessManager {
 public:
  TestNetworkAccessManager(const QDir& test_dir, QObject* parent)
      : QNetworkAccessManager(parent),
        test_dir_(test_dir) {}
  ~TestNetworkAccessManager() override {}

  QNetworkReply* createRequest(QNetworkAccessManager::Operation op,
                               const QNetworkRequest& req,
                               QIODevice* outgoing_data) override;

 private:
  QDir test_dir_;
};

QNetworkReply* TestNetworkAccessManager::createRequest(
    QNetworkAccessManager::Operation op,
    const QNetworkRequest& req,
    QIODevice* outgoing_data) {
  if (req.url().scheme() != QLatin1String("test")) {
    return QNetworkAccessManager::createRequest(op, req, outgoing_data);
  }

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

TestNetworkAccessManagerFactory::TestNetworkAccessManagerFactory(
    const QDir& test_dir)
    : test_dir_(test_dir) {}

TestNetworkAccessManagerFactory::~TestNetworkAccessManagerFactory() {}

QNetworkAccessManager* TestNetworkAccessManagerFactory::create(
    QObject* parent) {
  return new TestNetworkAccessManager(test_dir_, parent);
}
