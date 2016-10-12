// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTest>
#include <QUrl>

#include "qt/quick/api/oxideqquicknavigationitem.h"
#include "qt/quick/api/oxideqquicknavigationitem_p.h"

class tst_oxideqquicknavigationitem : public QObject {
  Q_OBJECT

 private Q_SLOTS:
  void copyConstruct();
  void assignment();
  void equality();
};

void tst_oxideqquicknavigationitem::copyConstruct() {
  QDateTime timestamp = QDateTime::currentDateTime();
  OxideQQuickNavigationItem a =
      OxideQQuickNavigationItemData::createForTesting(
          QUrl("https://www.google.com"),
          QUrl("https://www.google.com"),
          "Test", timestamp);
  QCOMPARE(a.url(), QUrl("https://www.google.com"));
  QCOMPARE(a.originalUrl(), QUrl("https://www.google.com"));
  QCOMPARE(a.title(), QString("Test"));
  QCOMPARE(a.timestamp(), timestamp);

  OxideQQuickNavigationItem b(a);

  QCOMPARE(b.url(), a.url());
  QCOMPARE(b.originalUrl(), a.originalUrl());
  QCOMPARE(b.title(), a.title());
  QCOMPARE(b.timestamp(), a.timestamp());
}

void tst_oxideqquicknavigationitem::assignment() {
  OxideQQuickNavigationItem a =
      OxideQQuickNavigationItemData::createForTesting(
          QUrl("https://www.google.com"),
          QUrl("https://www.google.com"),
          "Test", QDateTime::currentDateTime());
  OxideQQuickNavigationItem b;

  b = a;

  QCOMPARE(b.url(), a.url());
  QCOMPARE(b.originalUrl(), a.originalUrl());
  QCOMPARE(b.title(), a.title());
  QCOMPARE(b.timestamp(), a.timestamp());
}

void tst_oxideqquicknavigationitem::equality() {
  OxideQQuickNavigationItem a =
      OxideQQuickNavigationItemData::createForTesting(
          QUrl("https://www.google.com"),
          QUrl("https://www.google.com"),
          "Test", QDateTime::currentDateTime());
  OxideQQuickNavigationItem b =
      OxideQQuickNavigationItemData::createForTesting(
          QUrl("https://www.google.com"),
          QUrl("https://www.google.com"),
          "Test", QDateTime::currentDateTime());

  QVERIFY(a != b);

  b = a;

  QVERIFY(a == b);
}

QTEST_MAIN(tst_oxideqquicknavigationitem)

#include "tst_oxideqquicknavigationitem.moc"
