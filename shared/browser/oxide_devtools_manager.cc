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

#include "oxide_devtools_manager.h"

#include <utility>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/singleton.h"
#include "components/devtools_http_handler/devtools_http_handler.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "content/public/browser/devtools_socket_factory.h"
#include "net/base/ip_address.h"
#include "net/base/net_errors.h"
#include "net/log/net_log.h"
#include "net/socket/tcp_server_socket.h"

#include "oxide_browser_context.h"
#include "oxide_devtools_http_handler_delegate.h"
#include "oxide_user_agent_settings.h"

namespace oxide {

namespace {
const int kDefaultPort = 8484;
const char kDefaultAddress[] = "127.0.0.1";

const int kMinPort = 1024;
const int kMaxPort = 65535;

const int kBackLog = 1;
};

class TCPServerSocketFactory : public content::DevToolsSocketFactory {
 public:
  TCPServerSocketFactory(const std::string& address, int port)
      : address_(address),
        port_(port) {}

 private:
  std::unique_ptr<net::ServerSocket> CreateForHttpServer() override {
    std::unique_ptr<net::TCPServerSocket> socket(
        new net::TCPServerSocket(nullptr, net::NetLog::Source()));
    if (socket->ListenWithAddressAndPort(address_,
                                         port_,
                                         kBackLog) != net::OK) {
      return nullptr;
    }

    return std::move(socket);
  }

  std::unique_ptr<net::ServerSocket> CreateForTethering(
      std::string* out_name) override {
    NOTIMPLEMENTED();
    return nullptr;
  }

  std::string address_;
  int port_;

  DISALLOW_COPY_AND_ASSIGN(TCPServerSocketFactory);
};

class DevToolsManagerFactory : public BrowserContextKeyedServiceFactory {
 public:
  static DevToolsManagerFactory* GetInstance();
  static DevToolsManager* GetForContext(content::BrowserContext* context);

 private:
  friend struct base::DefaultSingletonTraits<DevToolsManagerFactory>;

  DevToolsManagerFactory();
  ~DevToolsManagerFactory() override;

  // BrowserContextKeyedServiceFactory implementation
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(DevToolsManagerFactory);
};

DevToolsManagerFactory::DevToolsManagerFactory()
    : BrowserContextKeyedServiceFactory(
        "DevToolsManager",
        BrowserContextDependencyManager::GetInstance()) {
  DependsOn(UserAgentSettings::GetFactory());
}

DevToolsManagerFactory::~DevToolsManagerFactory() {}

KeyedService* DevToolsManagerFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  return new DevToolsManager(BrowserContext::FromContent(context));
}

content::BrowserContext* DevToolsManagerFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return BrowserContext::FromContent(context)->GetOriginalContext();
}

// static
DevToolsManagerFactory* DevToolsManagerFactory::GetInstance() {
  return base::Singleton<DevToolsManagerFactory>::get();
}

// static
DevToolsManager* DevToolsManagerFactory::GetForContext(
    content::BrowserContext* context) {
  return static_cast<DevToolsManager*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

DevToolsManager::DevToolsManager(BrowserContext* context)
    : context_(context),
      enabled_(false),
      port_(kDefaultPort),
      address_(kDefaultAddress) {}

DevToolsManager::~DevToolsManager() {}

// static
DevToolsManager* DevToolsManager::Get(content::BrowserContext* context) {
  return DevToolsManagerFactory::GetForContext(context);
}

void DevToolsManager::SetEnabled(bool enabled) {
  if (!enabled) {
    http_handler_.reset();
    return;
  }

  if (http_handler_.get()) {
    return;
  }

  std::unique_ptr<TCPServerSocketFactory> factory(
      new TCPServerSocketFactory(address_, port_));

  http_handler_.reset(
      new devtools_http_handler::DevToolsHttpHandler(
        std::move(factory),
        std::string(),
        new DevtoolsHttpHandlerDelegate(),
        base::FilePath(),
        base::FilePath(),
        UserAgentSettings::Get(context_)->GetProduct(),
        UserAgentSettings::Get(context_)->GetUserAgent()));
}

void DevToolsManager::SetPort(int port) {
  if (enabled_) {
    LOG(WARNING) << "Please disable devtools before setting the port";
    return;
  }

  if (port < kMinPort || port > kMaxPort) {
    LOG(WARNING) << "Invalid port " << port;
    return;
  }

  port_ = port;
}

void DevToolsManager::SetAddress(const std::string& ip_literal) {
  if (enabled_) {
    LOG(WARNING) << "Please disable devtools before setting the IP";
    return;
  }

  if (!net::IPAddress().AssignFromIPLiteral(ip_literal)) {
    LOG(WARNING) << "Invalid IP address " << ip_literal;
    return;
  }

  address_ = ip_literal;
}

// static
void DevToolsManager::GetValidPorts(int* min, int* max) {
  *min = kMinPort;
  *max = kMaxPort;
}

} // namespace oxide
