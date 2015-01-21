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

#include <hybris/properties/properties.h>

#include "base/logging.h"
#include "base/memory/singleton.h"

namespace oxide {

AndroidProperties::AndroidProperties()
    : available_(false) {
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
}

AndroidProperties::~AndroidProperties() {}

// static
AndroidProperties* AndroidProperties::GetInstance() {
  return Singleton<AndroidProperties>::get();
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

} // namespace oxide
