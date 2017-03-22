// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016-2017 Canonical Ltd.

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

#include "qt/core/api/oxideqwebcontextmenuparams.h"
#include "qt/core/api/oxideqwebcontextmenuparams_p.h"
#include "qt/core/glue/web_context_menu_params.h"

using oxide::qt::MediaType;
using oxide::qt::WebContextMenuParams;

namespace {

WebContextMenuParams MakeTestParams(const QUrl& page_url,
                                    const QUrl& frame_url,
                                    const QUrl& link_url,
                                    const QString& link_text,
                                    const QString& title_text,
                                    MediaType media_type,
                                    const QUrl& src_url,
                                    const QString& selection_text,
                                    bool is_editable) {
  WebContextMenuParams params;
  params.page_url = page_url;
  params.frame_url = frame_url;
  params.unfiltered_link_url = link_url;
  params.link_url = link_url;
  params.link_text = link_text;
  params.title_text = title_text;
  params.media_type = media_type;
  params.src_url = src_url;
  params.selection_text = selection_text;
  params.is_editable = is_editable;

  return params;
}

}

class tst_oxideqwebcontextmenuparams : public QObject {
  Q_OBJECT

 private Q_SLOTS:
  void construct();
  void assignment();
  void copy();
  void equality();
  void isLink();
  void isEditable();
  void isSelection();
  void mediaType();
};

void tst_oxideqwebcontextmenuparams::construct() {
  OxideQWebContextMenuParams params =
      OxideQWebContextMenuParamsData::Create(
          MakeTestParams(QUrl("https://www.twitter.com/"),
                         QUrl("https://www.google.com/"),
                         QUrl("http://www.example.com/"),
                         "Test link",
                         "Test title",
                         oxide::qt::MEDIA_TYPE_IMAGE,
                         QUrl("http://www.example.com/image.jpg"),
                         "Lorem ipsum dolor sit amet", false));

  QCOMPARE(params.pageUrl(), QUrl("https://www.twitter.com/"));
  QCOMPARE(params.frameUrl(), QUrl("https://www.google.com/"));
  QVERIFY(params.isLink());
  QVERIFY(!params.isEditable());
  QVERIFY(params.isSelection());
  QCOMPARE(params.mediaType(), OxideQWebContextMenuParams::MediaTypeImage);
  QCOMPARE(params.linkUrl(), QUrl("http://www.example.com/"));
  QCOMPARE(params.linkText(), QLatin1String("Test link"));
  QCOMPARE(params.titleText(), QLatin1String("Test title"));
  QCOMPARE(params.srcUrl(), QUrl("http://www.example.com/image.jpg"));
  QCOMPARE(params.selectionText(), QLatin1String("Lorem ipsum dolor sit amet"));
}

void tst_oxideqwebcontextmenuparams::assignment() {
  OxideQWebContextMenuParams params;
  QCOMPARE(params.pageUrl(), QUrl());

  params =
      OxideQWebContextMenuParamsData::Create(
          MakeTestParams(QUrl("https://www.twitter.com/"),
                         QUrl("https://www.google.com/"),
                         QUrl("http://www.example.com/"),
                         "Test link",
                         "Test title",
                         oxide::qt::MEDIA_TYPE_IMAGE,
                         QUrl("http://www.example.com/image.jpg"),
                         "Lorem ipsum dolor sit amet", false));

  QCOMPARE(params.pageUrl(), QUrl("https://www.twitter.com/"));
  QCOMPARE(params.frameUrl(), QUrl("https://www.google.com/"));
  QVERIFY(params.isLink());
  QVERIFY(!params.isEditable());
  QVERIFY(params.isSelection());
  QCOMPARE(params.mediaType(), OxideQWebContextMenuParams::MediaTypeImage);
  QCOMPARE(params.linkUrl(), QUrl("http://www.example.com/"));
  QCOMPARE(params.linkText(), QLatin1String("Test link"));
  QCOMPARE(params.titleText(), QLatin1String("Test title"));
  QCOMPARE(params.srcUrl(), QUrl("http://www.example.com/image.jpg"));
  QCOMPARE(params.selectionText(), QLatin1String("Lorem ipsum dolor sit amet"));
}

void tst_oxideqwebcontextmenuparams::copy() {
  OxideQWebContextMenuParams params1 =
      OxideQWebContextMenuParamsData::Create(
          MakeTestParams(QUrl("https://www.twitter.com/"),
                         QUrl("https://www.google.com/"),
                         QUrl("http://www.example.com/"),
                         "Test link",
                         "Test title",
                         oxide::qt::MEDIA_TYPE_IMAGE,
                         QUrl("http://www.example.com/image.jpg"),
                         "", false));
  QCOMPARE(params1.pageUrl(), QUrl("https://www.twitter.com/"));

  OxideQWebContextMenuParams params2(params1);

  QCOMPARE(params2.pageUrl(), QUrl("https://www.twitter.com/"));
  QCOMPARE(params2.frameUrl(), QUrl("https://www.google.com/"));
  QVERIFY(params2.isLink());
  QVERIFY(!params2.isEditable());
  QVERIFY(!params2.isSelection());
  QCOMPARE(params2.mediaType(), OxideQWebContextMenuParams::MediaTypeImage);
  QCOMPARE(params2.linkUrl(), QUrl("http://www.example.com/"));
  QCOMPARE(params2.linkText(), QLatin1String("Test link"));
  QCOMPARE(params2.titleText(), QLatin1String("Test title"));
  QCOMPARE(params2.srcUrl(), QUrl("http://www.example.com/image.jpg"));
}

void tst_oxideqwebcontextmenuparams::equality() {
  WebContextMenuParams input_params =
      MakeTestParams(QUrl("https://www.twitter.com/"),
                     QUrl("https://www.google.com/"),
                     QUrl("http://www.example.com/"),
                     "Test link",
                     "Test title",
                     oxide::qt::MEDIA_TYPE_IMAGE,
                     QUrl("http://www.example.com/image.jpg"),
                     "Lorem ipsum dolor sit amet", false);

  OxideQWebContextMenuParams params1 =
      OxideQWebContextMenuParamsData::Create(input_params);
  OxideQWebContextMenuParams params2(params1);
  OxideQWebContextMenuParams params3 = params2;

  OxideQWebContextMenuParams params4 =
      OxideQWebContextMenuParamsData::Create(input_params);

  QVERIFY(params1 == params2);
  QVERIFY(!(params1 != params2));
  QVERIFY(params1 == params3);
  QVERIFY(!(params1 != params3));
  QVERIFY(!(params1 == params4));
  QVERIFY(params1 != params4);
}

void tst_oxideqwebcontextmenuparams::isLink() {
  WebContextMenuParams input_params =
      MakeTestParams(QUrl("https://www.twitter.com/"),
                     QUrl("https://www.google.com/"),
                     QUrl("http://www.example.com/"),
                     "Test link",
                     "Test title",
                     oxide::qt::MEDIA_TYPE_IMAGE,
                     QUrl("http://www.example.com/image.jpg"),
                     "Lorem ipsum dolor sit amet", false);

  OxideQWebContextMenuParams params =
      OxideQWebContextMenuParamsData::Create(input_params);
  QVERIFY(params.isLink());

  input_params.unfiltered_link_url = QUrl();
  params = OxideQWebContextMenuParamsData::Create(input_params);
  QVERIFY(!params.isLink());
}

void tst_oxideqwebcontextmenuparams::isEditable() {
  WebContextMenuParams input_params =
      MakeTestParams(QUrl("https://www.twitter.com/"),
                     QUrl("https://www.google.com/"),
                     QUrl("http://www.example.com/"),
                     "Test link",
                     "Test title",
                     oxide::qt::MEDIA_TYPE_IMAGE,
                     QUrl("http://www.example.com/image.jpg"),
                     "Lorem ipsum dolor sit amet", false);

  OxideQWebContextMenuParams params =
      OxideQWebContextMenuParamsData::Create(input_params);
  QVERIFY(!params.isEditable());

  input_params.is_editable = true;
  params = OxideQWebContextMenuParamsData::Create(input_params);
  QVERIFY(params.isEditable());
}

void tst_oxideqwebcontextmenuparams::isSelection() {
  WebContextMenuParams input_params =
      MakeTestParams(QUrl("https://www.twitter.com/"),
                     QUrl("https://www.google.com/"),
                     QUrl("http://www.example.com/"),
                     "Test link",
                     "Test title",
                     oxide::qt::MEDIA_TYPE_IMAGE,
                     QUrl("http://www.example.com/image.jpg"),
                     "", false);

  OxideQWebContextMenuParams params =
      OxideQWebContextMenuParamsData::Create(input_params);
  QVERIFY(!params.isSelection());

  input_params.selection_text = "Lorem ipsum dolor sit amet";
  params = OxideQWebContextMenuParamsData::Create(input_params);
  QVERIFY(params.isSelection());
}

void tst_oxideqwebcontextmenuparams::mediaType() {
  WebContextMenuParams input_params =
      MakeTestParams(QUrl("https://www.twitter.com/"),
                     QUrl("https://www.google.com/"),
                     QUrl("http://www.example.com/"),
                     "Test link",
                     "Test title",
                     oxide::qt::MEDIA_TYPE_IMAGE,
                     QUrl("http://www.example.com/image.jpg"),
                     "Lorem ipsum dolor sit amet", false);

  OxideQWebContextMenuParams params =
      OxideQWebContextMenuParamsData::Create(input_params);
  QCOMPARE(params.mediaType(), OxideQWebContextMenuParams::MediaTypeImage);

  input_params.media_type = oxide::qt::MEDIA_TYPE_NONE;
  params = OxideQWebContextMenuParamsData::Create(input_params);
  QCOMPARE(params.mediaType(), OxideQWebContextMenuParams::MediaTypeNone);

  input_params.media_type = oxide::qt::MEDIA_TYPE_VIDEO;
  params = OxideQWebContextMenuParamsData::Create(input_params);
  QCOMPARE(params.mediaType(), OxideQWebContextMenuParams::MediaTypeVideo);
}

QTEST_MAIN(tst_oxideqwebcontextmenuparams)

#include "tst_oxideqwebcontextmenuparams.moc"
