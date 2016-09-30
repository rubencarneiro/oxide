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

#ifndef _OXIDE_QT_CORE_GLUE_CHROME_CONTROLLER_H_
#define _OXIDE_QT_CORE_GLUE_CHROME_CONTROLLER_H_

#include <memory>

#include <QtGlobal>

#include "qt/core/glue/oxide_qt_proxy_base.h"
#include "qt/core/glue/web_contents_id.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ChromeControllerClient;

class Q_DECL_EXPORT ChromeController : public ProxyBase<ChromeController> {
 public:

  enum class Mode {
    Auto,
    Shown,
    Hidden
  };

  static std::unique_ptr<ChromeController> create(
      ChromeControllerClient* client,
      QObject* handle);

  virtual void init(WebContentsID web_contents_id) = 0;

  virtual int topControlsHeight() const = 0;
  virtual void setTopControlsHeight(int height) = 0;

  virtual Mode mode() const = 0;
  virtual void setMode(Mode mode) = 0;

  virtual bool animationEnabled() const = 0;
  virtual void setAnimationEnabled(bool enabled) = 0;

  virtual void show(bool animated) = 0;
  virtual void hide(bool animated) = 0;

  virtual int topControlsOffset() const = 0;
  virtual int topContentOffset() const = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_CHROME_CONTROLLER_H_
