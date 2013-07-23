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

#include "oxide_ssl_config_service.h"

namespace oxide {

void SSLConfigService::GetSSLConfig(net::SSLConfig* config) {
  // TODO: Allow this to be configured through GlobalSettings

  config->rev_checking_enabled = true;
  config->version_min = default_version_min();
  config->version_max = default_version_max();
  config->channel_id_enabled = true;
  config->false_start_enabled = true;
  config->unrestricted_ssl3_fallback_enabled = false;
  config->send_client_cert = false;
  config->verify_ev_cert = false;
  config->version_fallback = false;
  config->cert_io_enabled = true;

  SetSSLConfigFlags(config);
}

} // namespace oxide
