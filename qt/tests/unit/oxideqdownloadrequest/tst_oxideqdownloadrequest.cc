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

#include <QLatin1String>
#include <QTest>
#include <QUrl>

#include "qt/core/api/oxideqdownloadrequest.h"

class tst_oxideqdownloadrequest : public QObject {
  Q_OBJECT

 private Q_SLOTS:
  void construct();
  void assignment();
  void copy();
  void equality();
};

void tst_oxideqdownloadrequest::construct() {
  OxideQDownloadRequest request(QUrl("http://www.example.com/foo.html"),
                                "text/html",
                                false,
                                "bar.html",
                                "foo=1;bar=2",
                                "http://www.google.com/",
                                "Oxide Test");

  QCOMPARE(request.url(), QUrl("http://www.example.com/foo.html"));
  QCOMPARE(request.mimeType(), QLatin1String("text/html"));
  QVERIFY(!request.shouldPrompt());
  QCOMPARE(request.suggestedFilename(), QLatin1String("bar.html"));
  QCOMPARE(request.cookies().size(), 2);
  QCOMPARE(request.cookies()[0], QLatin1String("foo=1"));
  QCOMPARE(request.cookies()[1], QLatin1String("bar=2"));
  QCOMPARE(request.referrer(), QLatin1String("http://www.google.com/"));
  QCOMPARE(request.userAgent(), QLatin1String("Oxide Test"));
}

void tst_oxideqdownloadrequest::assignment() {
  OxideQDownloadRequest request;
  QCOMPARE(request.url(), QUrl());

  request = OxideQDownloadRequest(QUrl("http://www.example.com/foo.html"),
                                  "text/html",
                                  false,
                                  "bar.html",
                                  "foo=1;bar=2",
                                  "http://www.google.com/",
                                  "Oxide Test");

  QCOMPARE(request.url(), QUrl("http://www.example.com/foo.html"));
  QCOMPARE(request.mimeType(), QLatin1String("text/html"));
  QVERIFY(!request.shouldPrompt());
  QCOMPARE(request.suggestedFilename(), QLatin1String("bar.html"));
  QCOMPARE(request.cookies().size(), 2);
  QCOMPARE(request.cookies()[0], QLatin1String("foo=1"));
  QCOMPARE(request.cookies()[1], QLatin1String("bar=2"));
  QCOMPARE(request.referrer(), QLatin1String("http://www.google.com/"));
  QCOMPARE(request.userAgent(), QLatin1String("Oxide Test"));
}

void tst_oxideqdownloadrequest::copy() {
  OxideQDownloadRequest request(QUrl("http://www.example.com/foo.html"),
                                "text/html",
                                false,
                                "bar.html",
                                "foo=1;bar=2",
                                "http://www.google.com/",
                                "Oxide Test");
  QCOMPARE(request.url(), QUrl("http://www.example.com/foo.html"));

  OxideQDownloadRequest request2(request);
  QCOMPARE(request2.url(), QUrl("http://www.example.com/foo.html"));
  QCOMPARE(request2.mimeType(), QLatin1String("text/html"));
  QVERIFY(!request2.shouldPrompt());
  QCOMPARE(request2.suggestedFilename(), QLatin1String("bar.html"));
  QCOMPARE(request2.cookies().size(), 2);
  QCOMPARE(request2.cookies()[0], QLatin1String("foo=1"));
  QCOMPARE(request2.cookies()[1], QLatin1String("bar=2"));
  QCOMPARE(request2.referrer(), QLatin1String("http://www.google.com/"));
  QCOMPARE(request2.userAgent(), QLatin1String("Oxide Test"));
}

void tst_oxideqdownloadrequest::equality() {
  OxideQDownloadRequest request(QUrl("http://www.example.com/foo.html"),
                                "text/html",
                                false,
                                "bar.html",
                                "foo=1;bar=2",
                                "http://www.google.com/",
                                "Oxide Test");
  OxideQDownloadRequest request2(request);

  QVERIFY(request == request2);
  QVERIFY(!(request != request2));

  // OxideQDownloadRequest::operator== only does a pointer comparison of the
  // shared data, so only copies are considered equal

  OxideQDownloadRequest fake_copy(QUrl("http://www.example.com/foo.html"),
                                 "text/html",
                                 false,
                                 "bar.html",
                                 "foo=1;bar=2",
                                 "http://www.google.com/",
                                 "Oxide Test");
  QVERIFY(request != fake_copy);
  QVERIFY(!(request == fake_copy));
}

QTEST_MAIN(tst_oxideqdownloadrequest)

#include "tst_oxideqdownloadrequest.moc"
