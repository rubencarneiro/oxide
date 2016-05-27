// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include "oxide_gesture_provider.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"

#include "ui/display/screen.h"
#include "ui/events/gesture_detection/filtered_gesture_provider.h"
#include "ui/events/gesture_detection/gesture_provider.h"
#include "ui/events/gesture_detection/motion_event.h"

#include "oxide_event_utils.h"

namespace oxide {

namespace {

const double kDefaultRadius = 24.0f;

ui::GestureDetector::Config GetGestureDetectorConfig() {
  ui::GestureDetector::Config config;
  config.longpress_timeout = base::TimeDelta::FromMilliseconds(500);
  config.showpress_timeout = base::TimeDelta::FromMilliseconds(180);
  config.double_tap_timeout = base::TimeDelta::FromMilliseconds(300);

  config.touch_slop = 8.0f;
  config.double_tap_slop = 100.0f;
  config.minimum_fling_velocity = 50.0f;
  config.maximum_fling_velocity = 10000.0f;

  return config;
}

ui::ScaleGestureDetector::Config GetScaleGestureDetectorConfig() {
  ui::ScaleGestureDetector::Config config;
  config.span_slop = 16.0f;
  config.min_scaling_span = 170.0f;
  config.min_pinch_update_span_delta = 0.0f;

  return config;
}

ui::GestureProvider::Config GetGestureProviderConfig() {
  ui::GestureProvider::Config config;
  config.display = display::Screen::GetScreen()->GetPrimaryDisplay();

  config.gesture_detector_config = GetGestureDetectorConfig();
  config.scale_gesture_detector_config = GetScaleGestureDetectorConfig();
  config.gesture_begin_end_types_enabled = false;
  config.min_gesture_bounds_length = kDefaultRadius;

  return config;
}

}

class GestureProviderImpl : public GestureProvider,
                            public ui::GestureProviderClient {
 public:
  GestureProviderImpl(oxide::GestureProviderClient* client);
  ~GestureProviderImpl() override;

 private:
  // GestureProvider implementation
  ui::FilteredGestureProvider::TouchHandlingResult OnTouchEvent(
      const ui::MotionEvent& event) override;
  void OnTouchEventAck(uint32_t unique_event_id,
                       bool consumed) override;
  void SetDoubleTapSupportForPageEnabled(bool enabled) override;
  const ui::MotionEvent* GetCurrentDownEvent() const override;
  void ResetDetection() override;

  // ui::GestureProviderClient implementation
  void OnGestureEvent(const ui::GestureEventData& gesture) override;

  // Need the oxide identifier here, else this becomes
  // "ui::GestureProviderClient"
  oxide::GestureProviderClient* client_;
  ui::FilteredGestureProvider filtered_gesture_provider_;

  DISALLOW_COPY_AND_ASSIGN(GestureProviderImpl);
};

ui::FilteredGestureProvider::TouchHandlingResult
GestureProviderImpl::OnTouchEvent(const ui::MotionEvent& event) {
  return filtered_gesture_provider_.OnTouchEvent(event);
}

void GestureProviderImpl::OnTouchEventAck(uint32_t unique_event_id,
                                          bool consumed) {
  filtered_gesture_provider_.OnTouchEventAck(unique_event_id, consumed);
}

void GestureProviderImpl::SetDoubleTapSupportForPageEnabled(bool enabled) {
  filtered_gesture_provider_.SetDoubleTapSupportForPageEnabled(enabled);
}

const ui::MotionEvent* GestureProviderImpl::GetCurrentDownEvent() const {
  return filtered_gesture_provider_.GetCurrentDownEvent();
}

void GestureProviderImpl::ResetDetection() {
  filtered_gesture_provider_.ResetDetection();
}

void GestureProviderImpl::OnGestureEvent(const ui::GestureEventData& gesture) {
  client_->OnGestureEvent(MakeWebGestureEvent(gesture));
}

GestureProviderImpl::GestureProviderImpl(oxide::GestureProviderClient* client)
    : client_(client),
      filtered_gesture_provider_(GetGestureProviderConfig(), this) {
  filtered_gesture_provider_.SetDoubleTapSupportForPlatformEnabled(true);
}

GestureProviderImpl::~GestureProviderImpl() {}

GestureProviderClient::~GestureProviderClient() {}

// static
std::unique_ptr<GestureProvider> GestureProvider::Create(
    GestureProviderClient* client) {
  DCHECK(client) << "A GestureProviderClient must be provided";
  return base::WrapUnique(new GestureProviderImpl(client));
}

GestureProvider::~GestureProvider() {}

} // namespace oxide
