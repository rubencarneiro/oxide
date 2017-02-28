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

#ifndef _OXIDE_QT_CORE_BROWSER_CHROME_CONTROLLER_H_
#define _OXIDE_QT_CORE_BROWSER_CHROME_CONTROLLER_H_

#include <QtGlobal>

#include <memory>

#include "base/macros.h"

#include "qt/core/glue/chrome_controller.h"
#include "shared/browser/chrome_controller_observer.h"
#include "shared/browser/screen_observer.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ChromeControllerClient;
class ContentsViewImpl;

class ChromeControllerImpl : public oxide::ChromeControllerObserver,
                             public ChromeController,
                             public oxide::ScreenObserver {
 public:
  ChromeControllerImpl(qt::ChromeControllerClient* client,
                       QObject* handle);
  ~ChromeControllerImpl() override;

 private:
  // oxide::ChromeControllerObserver implementation
  void ContentOrTopControlsOffsetChanged() override;

  // ChromeController implementation
  void init(WebContentsID web_contents_id) override;
  int topControlsHeight() const override;
  void setTopControlsHeight(int height) override;
  Mode mode() const override;
  void setMode(Mode mode) override;
  bool animationEnabled() const override;
  void setAnimationEnabled(bool enabled) override;
  void show(bool animated) override;
  void hide(bool animated) override;
  int topControlsOffset() const override;
  int topContentOffset() const override;

  // oxide::ScreenObserver implementation
  void OnDisplayPropertiesChanged(const display::Display& display) override;

  qt::ChromeControllerClient* client_;

  ContentsViewImpl* contents_view_;

  struct InitProps;
  std::unique_ptr<InitProps> init_props_;

  int top_controls_height_;

  DISALLOW_COPY_AND_ASSIGN(ChromeControllerImpl);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_CHROME_CONTROLLER_H_
