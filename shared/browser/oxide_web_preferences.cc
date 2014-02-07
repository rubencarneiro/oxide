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

#include "oxide_web_preferences.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_implementation.h"
#include "ui/gl/gl_share_group.h"
#include "webkit/common/webpreferences.h"

#include "shared/common/oxide_content_client.h"
#include "shared/gl/oxide_shared_gl_context.h"

#include "oxide_content_browser_client.h"
#include "oxide_web_view.h"

namespace oxide {

namespace {

bool SupportsCompositing() {
  gfx::GLShareGroup* group =
      ContentClient::instance()->browser()->GetGLShareGroup();
  if (!group) {
    return false;
  }

  SharedGLContext* context = SharedGLContext::FromGfx(group->GetContext());
  if (!context) {
    return false;
  }

  return context->GetImplementation() == gfx::GetGLImplementation();
}

} // namespace

void WebPreferences::UpdateViews() {
  for (std::set<WebView *>::const_iterator it = views_.begin();
       it != views_.end(); ++it) {
    WebView* view = *it;
    content::WebContents* contents = view->web_contents();
    if (!contents) {
      continue;
    }
    contents->GetRenderViewHost()->UpdateWebkitPreferences(
        contents->GetRenderViewHost()->GetWebkitPreferences());
  }
}

WebPreferences::WebPreferences() :
    standard_font_family_(base::UTF8ToUTF16("Times New Roman")),
    fixed_font_family_(base::UTF8ToUTF16("Courier New")),
    serif_font_family_(base::UTF8ToUTF16("Times New Roman")),
    sans_serif_font_family_(base::UTF8ToUTF16("Arial")),
    default_encoding_("ISO-8859-1"),
    default_font_size_(16),
    default_fixed_font_size_(13),
    minimum_font_size_(0) {
  for (unsigned int i = 0; i < ATTR_LAST; ++i) {
    attributes_[i] = false;
  }

  SetAttribute(ATTR_REMOTE_FONTS_ENABLED, true);
  SetAttribute(ATTR_JAVASCRIPT_ENABLED, true);
  SetAttribute(ATTR_WEB_SECURITY_ENABLED, true);
  SetAttribute(ATTR_POPUP_BLOCKER_ENABLED, true);

  // ATTR_ALLOW_SCRIPTS_TO_CLOSE_WINDOWS
  // ATTR_JAVASCRIPT_CAN_ACCESS_CLIPBOARD

  SetAttribute(ATTR_HYPERLINK_AUDITING_ENABLED, true);

  // ATTR_ALLOW_UNIVERSAL_ACCESS_FROM_FILE_URLS
  // ATTR_ALLOW_FILE_ACCESS_FROM_FILE_URLS
  // ATTR_CAN_DISPLAY_INSECURE_CONTENT
  // ATTR_CAN_RUN_INSECURE_CONTENT
  // ATTR_PASSWORD_ECHO_ENABLED

  SetAttribute(ATTR_LOADS_IMAGES_AUTOMATICALLY, true);
  SetAttribute(ATTR_SHRINKS_STANDALONE_IMAGES_TO_FIT, true);

  SetAttribute(ATTR_TEXT_AREAS_ARE_RESIZABLE, true);

  // ATTR_LOCAL_STORAGE_ENABLED
  // ATTR_DATABASES_ENABLED
  // ATTR_APP_CACHE_ENABLED
  // ATTR_FULLSCREEN_ENABLED

  SetAttribute(ATTR_TABS_TO_LINKS, true);

  // ATTR_CARET_BROWSING_ENABLED

  SetAttribute(ATTR_ACCELERATED_COMPOSITING_ENABLED, true);
  SetAttribute(ATTR_SMOOTH_SCROLLING_ENABLED, true);

  // ATTR_TOUCH_ENABLED
  // ATTR_SUPPORTS_MULTIPLE_WINDOWS
  // ATTR_VIEWPORT_ENABLED
}

WebPreferences::~WebPreferences() {
  CHECK(views_.empty());
}

std::string WebPreferences::StandardFontFamily() const {
  return base::UTF16ToUTF8(standard_font_family_);
}

void WebPreferences::SetStandardFontFamily(const std::string& font) {
  standard_font_family_ = base::UTF8ToUTF16(font);
  UpdateViews();
}

std::string WebPreferences::FixedFontFamily() const {
  return base::UTF16ToUTF8(fixed_font_family_);
}

void WebPreferences::SetFixedFontFamily(const std::string& font) {
  fixed_font_family_ = base::UTF8ToUTF16(font);
  UpdateViews();
}

std::string WebPreferences::SerifFontFamily() const {
  return base::UTF16ToUTF8(serif_font_family_);
}

void WebPreferences::SetSerifFontFamily(const std::string& font) {
  serif_font_family_ = base::UTF8ToUTF16(font);
  UpdateViews();
}

std::string WebPreferences::SansSerifFontFamily() const {
  return base::UTF16ToUTF8(sans_serif_font_family_);
}

void WebPreferences::SetSansSerifFontFamily(const std::string& font) {
  sans_serif_font_family_ = base::UTF8ToUTF16(font);
  UpdateViews();
}

void WebPreferences::SetDefaultEncoding(const std::string& encoding) {
  default_encoding_ = encoding;
  UpdateViews();
}

void WebPreferences::SetDefaultFontSize(unsigned size) {
  default_font_size_ = size;
  UpdateViews();
}

void WebPreferences::SetDefaultFixedFontSize(unsigned size) {
  default_fixed_font_size_ = size;
  UpdateViews();
}

void WebPreferences::SetMinimumFontSize(unsigned size) {
  minimum_font_size_ = size;
  UpdateViews();
}

bool WebPreferences::TestAttribute(Attr attr) const {
  DCHECK(attr < ATTR_LAST && attr >= 0);
  return attributes_[attr];
}

void WebPreferences::SetAttribute(Attr attr, bool val) {
  DCHECK(attr < ATTR_LAST && attr >= 0);
  attributes_[attr] = val;
  UpdateViews();
}

void WebPreferences::AddWebView(WebView* view) {
  views_.insert(view);
}

void WebPreferences::RemoveWebView(WebView* view) {
  views_.erase(view);
}

void WebPreferences::ApplyToWebkitPrefs(::WebPreferences* prefs) {
  prefs->standard_font_family_map[webkit_glue::kCommonScript] =
      standard_font_family_;
  prefs->fixed_font_family_map[webkit_glue::kCommonScript] =
      fixed_font_family_;
  prefs->serif_font_family_map[webkit_glue::kCommonScript] =
      serif_font_family_;
  prefs->sans_serif_font_family_map[webkit_glue::kCommonScript] =
      sans_serif_font_family_;

  prefs->default_encoding = default_encoding_;

  prefs->default_font_size = default_font_size_;
  prefs->default_fixed_font_size = default_fixed_font_size_;
  prefs->minimum_font_size = minimum_font_size_;

  prefs->remote_fonts_enabled = attributes_[ATTR_REMOTE_FONTS_ENABLED];

  prefs->javascript_enabled = attributes_[ATTR_JAVASCRIPT_ENABLED];
  prefs->web_security_enabled = attributes_[ATTR_WEB_SECURITY_ENABLED];
  prefs->javascript_can_open_windows_automatically =
      !attributes_[ATTR_POPUP_BLOCKER_ENABLED];
  prefs->allow_scripts_to_close_windows =
      attributes_[ATTR_ALLOW_SCRIPTS_TO_CLOSE_WINDOWS];
  prefs->javascript_can_access_clipboard =
      attributes_[ATTR_JAVASCRIPT_CAN_ACCESS_CLIPBOARD];

  prefs->hyperlink_auditing_enabled =
      attributes_[ATTR_HYPERLINK_AUDITING_ENABLED];
  prefs->allow_universal_access_from_file_urls =
      attributes_[ATTR_ALLOW_UNIVERSAL_ACCESS_FROM_FILE_URLS];
  prefs->allow_file_access_from_file_urls =
      attributes_[ATTR_ALLOW_FILE_ACCESS_FROM_FILE_URLS];
  prefs->allow_displaying_insecure_content =
      attributes_[ATTR_CAN_DISPLAY_INSECURE_CONTENT];
  prefs->allow_running_insecure_content =
      attributes_[ATTR_CAN_RUN_INSECURE_CONTENT];
  prefs->password_echo_enabled = attributes_[ATTR_PASSWORD_ECHO_ENABLED];

  prefs->loads_images_automatically =
      attributes_[ATTR_LOADS_IMAGES_AUTOMATICALLY];
  prefs->shrinks_standalone_images_to_fit =
      attributes_[ATTR_SHRINKS_STANDALONE_IMAGES_TO_FIT];

  prefs->text_areas_are_resizable = attributes_[ATTR_TEXT_AREAS_ARE_RESIZABLE];

  prefs->local_storage_enabled = attributes_[ATTR_LOCAL_STORAGE_ENABLED];
  prefs->databases_enabled = attributes_[ATTR_DATABASES_ENABLED];
  prefs->application_cache_enabled = attributes_[ATTR_APP_CACHE_ENABLED];
  prefs->fullscreen_enabled = attributes_[ATTR_FULLSCREEN_ENABLED];

  prefs->tabs_to_links = attributes_[ATTR_TABS_TO_LINKS];
  prefs->caret_browsing_enabled = attributes_[ATTR_CARET_BROWSING_ENABLED];

  bool compositing =
      attributes_[ATTR_ACCELERATED_COMPOSITING_ENABLED] &&
      SupportsCompositing();
  prefs->accelerated_compositing_enabled = compositing;
  prefs->force_compositing_mode = compositing;

  prefs->enable_scroll_animator = attributes_[ATTR_SMOOTH_SCROLLING_ENABLED];

  prefs->touch_enabled = false; // TODO: Check if touch is supported

  prefs->supports_multiple_windows = attributes_[ATTR_SUPPORTS_MULTIPLE_WINDOWS];

  // Viewport only works in compositing mode
  prefs->viewport_enabled = attributes_[ATTR_VIEWPORT_ENABLED] && compositing;
  // XXX: Should this be a separate pref?
  prefs->viewport_meta_enabled = prefs->viewport_enabled;
}

} // namespace oxide
