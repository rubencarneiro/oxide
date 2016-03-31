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

#ifndef _OXIDE_QT_TESTS_QMLTESTS_TEST_NAM_FACTORY_H_
#define _OXIDE_QT_TESTS_QMLTESTS_TEST_NAM_FACTORY_H_

#include <QDir>
#include <QQmlNetworkAccessManagerFactory>

class TestNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory {
 public:
  TestNetworkAccessManagerFactory(const QDir& test_dir);
  ~TestNetworkAccessManagerFactory() override;

  QNetworkAccessManager* create(QObject* parent) override;

 private:
  QDir test_dir_;
};

#endif // _OXIDE_QT_TESTS_QMLTESTS_TEST_NAM_FACTORY_H_
