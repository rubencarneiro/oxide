// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_android_properties.h"

#if defined(ENABLE_HYBRIS)
#include <cstdio>
#include <hybris/properties/properties.h>
#include "base/strings/stringprintf.h"
#endif

#include "base/logging.h"
#include "base/memory/singleton.h"

namespace oxide {

namespace {

#if defined(ENABLE_HYBRIS)
std::string ParseOSVersion(const char* os_version_str) {
  int32_t major, minor, bugfix;

  if (!os_version_str[0]) {
    return std::string();
  }

  int num_read = sscanf(os_version_str, "%d.%d.%d", &major, &minor, &bugfix);
  if (num_read <= 0) {
    return std::string();
  }

  if (num_read < 2) {
    minor = 0;
  }
  if (num_read < 3) {
    bugfix = 0;
  }

  return base::StringPrintf("%d.%d.%d", major, minor, bugfix);
}
#endif

}

AndroidProperties::AndroidProperties()
    : available_(false) {
#if defined(ENABLE_HYBRIS)
  char value[PROP_VALUE_MAX];

  if (::property_get("ro.product.name", value, nullptr) <= 0) {
    return;
  }

  available_ = true;
  product_ = value;

  ::property_get("ro.product.device", value, nullptr);
  device_ = value;

  ::property_get("ro.product.board", value, nullptr);
  board_ = value;

  ::property_get("ro.product.brand", value, nullptr);
  brand_ = value;

  ::property_get("ro.product.model", value, nullptr);
  model_ = value;

  ::property_get("ro.build.version.release", value, nullptr);
  os_version_ = ParseOSVersion(value);
#endif
}

AndroidProperties::~AndroidProperties() {}

// static
AndroidProperties* AndroidProperties::GetInstance() {
  return base::Singleton<AndroidProperties>::get();
}

bool AndroidProperties::Available() const {
  return available_;
}

std::string AndroidProperties::GetProduct() const {
  DCHECK(available_);
  return product_;
}

std::string AndroidProperties::GetDevice() const {
  DCHECK(available_);
  return device_;
}

std::string AndroidProperties::GetBoard() const {
  DCHECK(available_);
  return board_;
}

std::string AndroidProperties::GetBrand() const {
  DCHECK(available_);
  return brand_;
}

std::string AndroidProperties::GetModel() const {
  DCHECK(available_);
  return model_;
}

std::string AndroidProperties::GetOSVersion() const {
  DCHECK(available_);
  return os_version_;
}

} // namespace oxide
