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

#include "oxideqwebcontextmenuparams.h"
#include "oxideqwebcontextmenuparams_p.h"

#include <string>

#include "qt/core/glue/macros.h"
#include "qt/core/glue/web_context_menu_params.h"

using oxide::qt::WebContextMenuParams;

// static
OxideQWebContextMenuParams OxideQWebContextMenuParamsData::Create(
    const WebContextMenuParams& params) {
  STATIC_ASSERT_MATCHING_ENUM(OxideQWebContextMenuParams::MediaTypeNone,
                              oxide::qt::MEDIA_TYPE_NONE)
  STATIC_ASSERT_MATCHING_ENUM(OxideQWebContextMenuParams::MediaTypeImage,
                              oxide::qt::MEDIA_TYPE_IMAGE)
  STATIC_ASSERT_MATCHING_ENUM(OxideQWebContextMenuParams::MediaTypeVideo,
                              oxide::qt::MEDIA_TYPE_VIDEO)
  STATIC_ASSERT_MATCHING_ENUM(OxideQWebContextMenuParams::MediaTypeAudio,
                              oxide::qt::MEDIA_TYPE_AUDIO)
  STATIC_ASSERT_MATCHING_ENUM(OxideQWebContextMenuParams::MediaTypeCanvas,
                              oxide::qt::MEDIA_TYPE_CANVAS)
  STATIC_ASSERT_MATCHING_ENUM(OxideQWebContextMenuParams::MediaTypePlugin,
                              oxide::qt::MEDIA_TYPE_PLUGIN)

  QSharedDataPointer<OxideQWebContextMenuParamsData> d(
      new OxideQWebContextMenuParamsData());
  d->page_url = params.page_url;
  d->frame_url = params.frame_url;

  d->is_link = !params.unfiltered_link_url.isEmpty();
  d->is_editable = params.is_editable;
  d->is_selection = !params.selection_text.isEmpty();
  d->media_type =
      static_cast<OxideQWebContextMenuParams::MediaType>(params.media_type);

  d->link_url = params.link_url;
  d->link_text = params.link_text;
  d->title_text = params.title_text;
  d->src_url = params.src_url;
  d->selection_text = params.selection_text;

  return OxideQWebContextMenuParams(d);
}

OxideQWebContextMenuParamsData::OxideQWebContextMenuParamsData() = default;

OxideQWebContextMenuParamsData::~OxideQWebContextMenuParamsData() = default;

OxideQWebContextMenuParams::OxideQWebContextMenuParams(
    QSharedDataPointer<OxideQWebContextMenuParamsData> d)
    : d(d) {}

OxideQWebContextMenuParams::OxideQWebContextMenuParams()
    : d(new OxideQWebContextMenuParamsData()) {}

OxideQWebContextMenuParams::OxideQWebContextMenuParams(
    const OxideQWebContextMenuParams& other) = default;

OxideQWebContextMenuParams::~OxideQWebContextMenuParams() = default;

OxideQWebContextMenuParams& OxideQWebContextMenuParams::operator=(
    const OxideQWebContextMenuParams& other) = default;

bool OxideQWebContextMenuParams::operator==(
    const OxideQWebContextMenuParams& other) const {
  return d == other.d;
}

bool OxideQWebContextMenuParams::operator!=(
    const OxideQWebContextMenuParams& other) const {
  return !(*this == other);
}

QUrl OxideQWebContextMenuParams::pageUrl() const {
  return d->page_url;
}

QUrl OxideQWebContextMenuParams::frameUrl() const {
  return d->frame_url;
}

bool OxideQWebContextMenuParams::isLink() const {
  return d->is_link;
}

bool OxideQWebContextMenuParams::isEditable() const {
  return d->is_editable;
}

bool OxideQWebContextMenuParams::isSelection() const {
  return d->is_selection;
}

OxideQWebContextMenuParams::MediaType 
OxideQWebContextMenuParams::mediaType() const {
  return d->media_type;
}

QUrl OxideQWebContextMenuParams::linkUrl() const {
  return d->link_url;
}

QString OxideQWebContextMenuParams::linkText() const {
  return d->link_text;
}

QString OxideQWebContextMenuParams::titleText() const {
  return d->title_text;
}

QUrl OxideQWebContextMenuParams::srcUrl() const {
  return d->src_url;
}

QString OxideQWebContextMenuParams::selectionText() const {
  return d->selection_text;
}
