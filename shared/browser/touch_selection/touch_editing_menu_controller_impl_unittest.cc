// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2017 Canonical Ltd.

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

#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "base/run_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "cc/output/compositor_frame_metadata.h"
#include "content/public/common/context_menu_params.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/geometry/rect_f.h"
#include "ui/gfx/geometry/size_f.h"
#include "ui/gfx/selection_bound.h"
#include "ui/touch_selection/touch_handle.h"
#include "ui/touch_selection/touch_selection_controller.h"
#include "url/gurl.h"

#include "shared/browser/chrome_controller.h"
#include "shared/browser/context_menu/web_context_menu_actions.h"
#include "shared/browser/web_contents_client.h"
#include "shared/browser/web_contents_helper.h"
#include "shared/common/oxide_enum_flags.h"
#include "shared/test/web_contents_test_harness.h"

#include "touch_editing_menu.h"
#include "touch_editing_menu_controller_client.h"
#include "touch_editing_menu_controller_impl.h"

namespace oxide {

using testing::_;
using testing::StrictMock;

namespace {

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(blink::WebContextMenuData::EditFlags)

gfx::SelectionBound MakeSelectionBound(gfx::SelectionBound::Type type,
                                       const gfx::PointF& edge_top,
                                       const gfx::PointF& edge_bottom,
                                       bool visible) {
  gfx::SelectionBound bound;
  bound.set_type(type);
  bound.SetEdgeTop(edge_top);
  bound.SetEdgeBottom(edge_bottom);
  bound.set_visible(visible);

  return bound;
}

class StubTouchHandleDrawable : public ui::TouchHandleDrawable {
 public:
  StubTouchHandleDrawable() = default;
  ~StubTouchHandleDrawable() override = default;

 private:
  void SetEnabled(bool enabled) override {}
  void SetOrientation(ui::TouchHandleOrientation orientation,
                      bool mirror_vertical,
                      bool mirror_horizontal) override {}
  void SetOrigin(const gfx::PointF& origin) override {
    origin_ = origin;
  }
  void SetAlpha(float alpha) override {}
  gfx::RectF GetVisibleBounds() const override {
    return gfx::RectF(origin_, gfx::SizeF(10, 10));
  }
  float GetDrawableHorizontalPaddingRatio() const override { return 0.5f; }

  gfx::PointF origin_;
};

class StubTouchEditingMenuControllerClient
    : public TouchEditingMenuControllerClient {
 public:
  StubTouchEditingMenuControllerClient(
      ui::TouchSelectionController* touch_selection_controller,
      ChromeController* chrome_controller,
      content::WebContents* web_contents)
      : touch_selection_controller_(touch_selection_controller),
        chrome_controller_(chrome_controller),
        web_contents_(web_contents) {}

  void set_touch_selection_controller(ui::TouchSelectionController* controller) {
    touch_selection_controller_ = controller;
  }

  void set_editing_capabilities(blink::WebContextMenuData::EditFlags caps) {
    editing_capabilities_ = caps;
  }

 private:
  ui::TouchSelectionController* GetTouchSelectionController() const override {
    return touch_selection_controller_;
  }

  ChromeController* GetChromeController() const override {
    return chrome_controller_;
  }

  blink::WebContextMenuData::EditFlags GetEditingCapabilities() const override {
    return editing_capabilities_;
  }

  content::WebContents* GetWebContents() const override {
    return web_contents_;
  }

  ui::TouchSelectionController* touch_selection_controller_;
  ChromeController* chrome_controller_;
  content::WebContents* web_contents_;

  blink::WebContextMenuData::EditFlags editing_capabilities_ =
      blink::WebContextMenuData::kCanDoNone;
};

class TouchEditingMenuSink : public TouchEditingMenu {
 public:
  MOCK_METHOD0(Show, void());
  MOCK_METHOD0(Hide, void());
  gfx::Size GetSizeIncludingMargin() const { return gfx::Size(); }
  MOCK_METHOD1(SetOrigin, void(const gfx::PointF&));
};

class MockTouchEditingMenu : public TouchEditingMenu {
 public:
  MockTouchEditingMenu(TouchEditingMenu* sink,
                       TouchEditingMenuClient* client,
                       const gfx::Size& initial_size)
      : sink_(sink),
        client_(client),
        size_(initial_size),
        weak_ptr_factory_(this) {}

  TouchEditingMenuClient* client() const { return client_; }

  base::WeakPtr<MockTouchEditingMenu> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  void set_size(const gfx::Size& size) { size_ = size; }

  void Show() override { sink_->Show(); }
  void Hide() override { sink_->Hide(); }
  gfx::Size GetSizeIncludingMargin() const override { return size_; }
  void SetOrigin(const gfx::PointF& origin) override {
    LOG(INFO) << "SetOrigin: " << origin.ToString();
    sink_->SetOrigin(origin);
  }

 private:
  TouchEditingMenu* sink_;
  TouchEditingMenuClient* client_;
  gfx::Size size_;

  base::WeakPtrFactory<MockTouchEditingMenu> weak_ptr_factory_;
};

class MockWebContentsClient : public WebContentsClient {
 public:
  MockWebContentsClient(TouchEditingMenu* menu_sink);
  ~MockWebContentsClient() override = default;

  void set_create_touch_editing_menu(bool create) {
    create_touch_editing_menu_ = create;
  }

  void set_initial_menu_size(const gfx::Size& size) {
    initial_menu_size_ = size;
  }

  MockTouchEditingMenu* last_created_menu() const {
    return last_created_menu_.get();
  }

  MOCK_METHOD2(
      CreateTouchEditingMenu,
      std::unique_ptr<TouchEditingMenu>(blink::WebContextMenuData::EditFlags,
                                        TouchEditingMenuClient*));

  std::unique_ptr<TouchEditingMenu> CreateTouchEditingMenuImpl(
      blink::WebContextMenuData::EditFlags edit_flags,
      TouchEditingMenuClient* client);

 private:
  TouchEditingMenu* menu_sink_;

  bool create_touch_editing_menu_ = true;

  gfx::Size initial_menu_size_{300, 50};

  base::WeakPtr<MockTouchEditingMenu> last_created_menu_;
};

MockWebContentsClient::MockWebContentsClient(TouchEditingMenu* menu_sink)
    : menu_sink_(menu_sink) {
  ON_CALL(*this, CreateTouchEditingMenu(_,_)).WillByDefault(
      testing::Invoke(
          this,
          &MockWebContentsClient::CreateTouchEditingMenuImpl));
}

std::unique_ptr<TouchEditingMenu>
MockWebContentsClient::CreateTouchEditingMenuImpl(
    blink::WebContextMenuData::EditFlags edit_flags,
    TouchEditingMenuClient* client) {
  if (!create_touch_editing_menu_) {
    return nullptr;
  }

  std::unique_ptr<MockTouchEditingMenu> menu =
      base::MakeUnique<MockTouchEditingMenu>(menu_sink_, client,
                                             initial_menu_size_);
  last_created_menu_ = menu->GetWeakPtr();
  return std::move(menu);
}

} // namespace

class TouchEditingMenuControllerImplTest
    : public WebContentsTestHarness,
      public ui::TouchSelectionControllerClient {
 public:
  TouchEditingMenuControllerImplTest() = default;
  ~TouchEditingMenuControllerImplTest() override = default;

 protected:
  TouchEditingMenuSink& menu_sink() {
    return *menu_sink_.get();
  }

  MockWebContentsClient& web_contents_client() const {
    return *web_contents_client_.get();
  }

  ui::TouchSelectionController* touch_selection_controller() const {
    return touch_selection_controller_.get();
  }
  void SetTouchSelectionController(
      std::unique_ptr<ui::TouchSelectionController> controller);

  ChromeController* chrome_controller() const {
    return chrome_controller_.get();
  }

  StubTouchEditingMenuControllerClient& controller_client() const {
    return *controller_client_.get();
  }

  TouchEditingMenuControllerImpl* controller() const {
    return controller_.get();
  }

  MockTouchEditingMenu* GetLastCreatedMenu() const;

  void SetCreateTouchEditingMenu(bool create);
  void SetInitialMenuSize(const gfx::Size& size);

  void SetEditingCapabilities(blink::WebContextMenuData::EditFlags caps);

 private:
  // testing::Test implementation
  void SetUp() override;

  // ui::TouchSelectionControllerClient implementation
  bool SupportsAnimation() const override { return false; }
  void SetNeedsAnimate() override {}
  void MoveCaret(const gfx::PointF& position) override {}
  void MoveRangeSelectionExtent(const gfx::PointF& extent) override {}
  void SelectBetweenCoordinates(const gfx::PointF& base,
                                const gfx::PointF& extent) override {}
  void OnSelectionEvent(ui::SelectionEventType event) override;
  std::unique_ptr<ui::TouchHandleDrawable> CreateDrawable() override {
    return base::MakeUnique<StubTouchHandleDrawable>();
  }

  std::unique_ptr<TouchEditingMenuSink> menu_sink_;

  std::unique_ptr<StrictMock<MockWebContentsClient>> web_contents_client_;

  std::unique_ptr<ui::TouchSelectionController> touch_selection_controller_;

  std::unique_ptr<ChromeController> chrome_controller_;

  std::unique_ptr<StrictMock<StubTouchEditingMenuControllerClient>>
      controller_client_;
  std::unique_ptr<TouchEditingMenuControllerImpl> controller_;
};

void TouchEditingMenuControllerImplTest::SetTouchSelectionController(
    std::unique_ptr<ui::TouchSelectionController> controller) {
  touch_selection_controller_ = std::move(controller);
  controller_client_->set_touch_selection_controller(
      touch_selection_controller_.get());
}

MockTouchEditingMenu*
TouchEditingMenuControllerImplTest::GetLastCreatedMenu() const {
  return web_contents_client_->last_created_menu();
}

void TouchEditingMenuControllerImplTest::SetCreateTouchEditingMenu(
    bool create) {
  web_contents_client_->set_create_touch_editing_menu(create);
}

void TouchEditingMenuControllerImplTest::SetInitialMenuSize(
    const gfx::Size& size) {
  web_contents_client_->set_initial_menu_size(size);
}

void TouchEditingMenuControllerImplTest::SetEditingCapabilities(
    blink::WebContextMenuData::EditFlags caps) {
  controller_client_->set_editing_capabilities(caps);
}

void TouchEditingMenuControllerImplTest::SetUp() {
  WebContentsTestHarness::SetUp();

  menu_sink_ = base::MakeUnique<TouchEditingMenuSink>();

  web_contents_client_ =
      base::MakeUnique<StrictMock<MockWebContentsClient>>(menu_sink_.get());
  WebContentsHelper::CreateForWebContents(web_contents(), nullptr);
  WebContentsHelper::FromWebContents(web_contents())->SetClient(
      web_contents_client_.get());

  ui::TouchSelectionController::Config tsc_config;
  tsc_config.enable_adaptive_handle_orientation = true;
  touch_selection_controller_ =
      base::MakeUnique<ui::TouchSelectionController>(this, tsc_config);
  touch_selection_controller_->OnViewportChanged(gfx::RectF(0, 0, 800, 600));

  chrome_controller_ = ChromeController::CreateForWebContents(web_contents());

  controller_client_ =
      base::MakeUnique<StrictMock<StubTouchEditingMenuControllerClient>>(
          touch_selection_controller_.get(),
          chrome_controller_.get(),
          web_contents());
  controller_ =
      base::MakeUnique<TouchEditingMenuControllerImpl>(
          controller_client_.get());
  controller_->SetViewportBounds(gfx::RectF(600, 600, 800, 600));
  controller_->SetTopLevelWindowBounds(gfx::Rect(500, 500, 1000, 800));
}

void TouchEditingMenuControllerImplTest::OnSelectionEvent(
    ui::SelectionEventType event) {
  controller_->OnSelectionEvent(event);
}

TEST_F(TouchEditingMenuControllerImplTest, SelectionHandlesShownNoMenu) {
  SetCreateTouchEditingMenu(false);

  EXPECT_CALL(web_contents_client(), CreateTouchEditingMenu(_,controller()));

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));
}

TEST_F(TouchEditingMenuControllerImplTest, EditingCapabilities) {
  SetCreateTouchEditingMenu(false);

  SetEditingCapabilities(
      blink::WebContextMenuData::kCanCut
      | blink::WebContextMenuData::kCanCopy
      | blink::WebContextMenuData::kCanDelete
      | blink::WebContextMenuData::kCanSelectAll);

  blink::WebContextMenuData::EditFlags expected_caps =
      blink::WebContextMenuData::kCanCut
      | blink::WebContextMenuData::kCanCopy
      | blink::WebContextMenuData::kCanSelectAll;

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(expected_caps, controller()));

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));
  touch_selection_controller()->OnSelectionBoundsChanged(
      gfx::SelectionBound(), gfx::SelectionBound());

  SetEditingCapabilities(
      blink::WebContextMenuData::kCanCopy
      | blink::WebContextMenuData::kCanSelectAll);
  expected_caps =
      blink::WebContextMenuData::kCanCopy
      | blink::WebContextMenuData::kCanSelectAll;

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(expected_caps, controller()));

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));
}

TEST_F(TouchEditingMenuControllerImplTest, SelectionHandlesShown) {
  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));
}

TEST_F(TouchEditingMenuControllerImplTest, SelectionHandlesMoved) {
  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 10)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 60), gfx::PointF(50, 110), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 60), gfx::PointF(750, 110), true));

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 10)));
  EXPECT_CALL(menu_sink(), Hide());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 60), gfx::PointF(50, 110), false),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 60), gfx::PointF(750, 110), false));

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(600, 10)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 60), gfx::PointF(50, 110), false),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 60), gfx::PointF(750, 110), true));
}

TEST_F(TouchEditingMenuControllerImplTest, SelectionHandlesCleared) {
  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));

  EXPECT_TRUE(GetLastCreatedMenu());

  touch_selection_controller()->OnSelectionBoundsChanged(
      gfx::SelectionBound(), gfx::SelectionBound());

  EXPECT_FALSE(GetLastCreatedMenu());
}

TEST_F(TouchEditingMenuControllerImplTest, SelectionHandleDrag) {
  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));

  EXPECT_CALL(menu_sink(), Hide());
  controller()->OnSelectionEvent(ui::SELECTION_HANDLE_DRAG_STARTED);

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(275, 0)));
  EXPECT_CALL(menu_sink(), Hide());
  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(100, 50), gfx::PointF(100, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));

  EXPECT_CALL(menu_sink(), Show());
  controller()->OnSelectionEvent(ui::SELECTION_HANDLE_DRAG_STOPPED);
}

TEST_F(TouchEditingMenuControllerImplTest, InsertionHandleShown) {
  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));
}

TEST_F(TouchEditingMenuControllerImplTest, ContextMenuAfterInsertionHandleShown) {
  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
  EXPECT_CALL(menu_sink(), Show());

  content::ContextMenuParams params;
  params.source_type = ui::MENU_SOURCE_LONG_PRESS;
  params.is_editable = true;
  EXPECT_TRUE(controller()->HandleContextMenu(params));
}

TEST_F(TouchEditingMenuControllerImplTest, ContextMenuTriggeredInsertionHandle) {
  content::ContextMenuParams params;
  params.source_type = ui::MENU_SOURCE_LONG_PRESS;
  params.is_editable = true;
  EXPECT_TRUE(controller()->HandleContextMenu(params));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));
}

TEST_F(TouchEditingMenuControllerImplTest, InsertionHandleTapped) {
  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
  EXPECT_CALL(menu_sink(), Show());

  controller()->OnSelectionEvent(ui::INSERTION_HANDLE_TAPPED);
  EXPECT_TRUE(GetLastCreatedMenu());

  controller()->OnSelectionEvent(ui::INSERTION_HANDLE_TAPPED);
  EXPECT_FALSE(GetLastCreatedMenu());
}

TEST_F(TouchEditingMenuControllerImplTest, InsertionHandleMoved) {
  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
  EXPECT_CALL(menu_sink(), Show());

  controller()->OnSelectionEvent(ui::INSERTION_HANDLE_TAPPED);

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-80, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(70, 50), gfx::PointF(70, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(70, 50), gfx::PointF(70, 100), true));

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-80, 0)));
  EXPECT_CALL(menu_sink(), Hide());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(70, 50), gfx::PointF(70, 100), false),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(70, 50), gfx::PointF(70, 100), false));

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-60, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(90, 50), gfx::PointF(90, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(90, 50), gfx::PointF(90, 100), true));
}

TEST_F(TouchEditingMenuControllerImplTest, InsertionHandleCleared) {
  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
  EXPECT_CALL(menu_sink(), Show());

  controller()->OnSelectionEvent(ui::INSERTION_HANDLE_TAPPED);
  EXPECT_TRUE(GetLastCreatedMenu());

  touch_selection_controller()->OnSelectionBoundsChanged(
      gfx::SelectionBound(), gfx::SelectionBound());

  EXPECT_FALSE(GetLastCreatedMenu());
}

namespace {

struct ContextMenuTestParams {
  ui::MenuSourceType source_type;
  bool is_editable;
  base::StringPiece selection_text;
  bool handled;
};

} // namespace

class TouchEditingMenuControllerImplHandleContextMenuTest
    : public TouchEditingMenuControllerImplTest,
      public testing::WithParamInterface<ContextMenuTestParams> {};

INSTANTIATE_TEST_CASE_P(_,
    TouchEditingMenuControllerImplHandleContextMenuTest,
    testing::Values(
        ContextMenuTestParams{ui::MENU_SOURCE_LONG_PRESS, true, "", true},
        ContextMenuTestParams{ui::MENU_SOURCE_LONG_PRESS, false, "", false},
        ContextMenuTestParams{ui::MENU_SOURCE_LONG_PRESS, true, "foo", false},
        ContextMenuTestParams{ui::MENU_SOURCE_MOUSE, true, "", false}));

TEST_P(TouchEditingMenuControllerImplHandleContextMenuTest, _) {
  const auto& params = GetParam();

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));

  if (params.handled) {
    EXPECT_CALL(web_contents_client(),
                CreateTouchEditingMenu(_, controller()));
    EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
    EXPECT_CALL(menu_sink(), Show());
  }

  content::ContextMenuParams p;
  p.source_type = params.source_type;
  p.is_editable = params.is_editable;
  p.selection_text = base::UTF8ToUTF16(params.selection_text);
  EXPECT_EQ(params.handled, controller()->HandleContextMenu(p));
}

TEST_F(TouchEditingMenuControllerImplTest, TouchSelectionControllerSwapped) {
  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));

  EXPECT_TRUE(GetLastCreatedMenu());

  std::unique_ptr<ui::TouchSelectionController> new_touch_selection_controller =
      base::MakeUnique<ui::TouchSelectionController>(
          this,
          ui::TouchSelectionController::Config());
  new_touch_selection_controller->OnViewportChanged(gfx::RectF(0, 0, 800, 600));
  SetTouchSelectionController(std::move(new_touch_selection_controller));
  controller()->TouchSelectionControllerSwapped();
  EXPECT_FALSE(GetLastCreatedMenu());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
  EXPECT_CALL(menu_sink(), Show());

  controller()->OnSelectionEvent(ui::INSERTION_HANDLE_TAPPED);
  EXPECT_TRUE(GetLastCreatedMenu());
}

TEST_F(TouchEditingMenuControllerImplTest, SetViewportBounds) {
  controller()->SetViewportBounds(gfx::RectF(600, 600, 800, 100));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, -10)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 40), gfx::PointF(50, 90), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 40), gfx::PointF(750, 90), true));

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 100)));
  controller()->SetViewportBounds(gfx::RectF(500, 500, 800, 100));
}

TEST_F(TouchEditingMenuControllerImplTest, SetTopLevelWindowBounds) {
  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
  EXPECT_CALL(menu_sink(), Show());

  controller()->OnSelectionEvent(ui::INSERTION_HANDLE_TAPPED);

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0))).Times(2);

  controller()->SetTopLevelWindowBounds(gfx::Rect(400, 400, 1000, 800));
  controller()->SetViewportBounds(gfx::RectF(500, 500, 800, 600));
}

TEST_F(TouchEditingMenuControllerImplTest, TopControlsChanged) {
  chrome_controller()->SetTopControlsHeight(50);

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 50)));

  cc::CompositorFrameMetadata metadata;
  metadata.top_controls_height = 50;
  metadata.top_controls_shown_ratio = 1;
  chrome_controller()->FrameMetadataUpdated(
      base::make_optional(metadata.Clone()));
}

namespace {

struct ExecCommandParams {
  WebContextMenuAction action;
  bool hides;
};

} // namespace

class TouchEditingMenuControllerImplSelectionMenuExecCommandTest
    : public TouchEditingMenuControllerImplTest,
      public testing::WithParamInterface<ExecCommandParams> {};

INSTANTIATE_TEST_CASE_P(_,
    TouchEditingMenuControllerImplSelectionMenuExecCommandTest,
    testing::Values(
        ExecCommandParams{WebContextMenuAction::Copy, true},
        ExecCommandParams{WebContextMenuAction::SelectAll, false}));

TEST_P(TouchEditingMenuControllerImplSelectionMenuExecCommandTest, _) {
  const auto& params = GetParam();

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));

  if (params.hides) {
    EXPECT_CALL(menu_sink(), Hide());
  }

  GetLastCreatedMenu()->client()->ExecuteCommand(params.action);

  if (params.hides) {
    EXPECT_EQ(ui::TouchSelectionController::INACTIVE,
              touch_selection_controller()->active_status());
  }

  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(params.hides, !GetLastCreatedMenu());
}

class TouchEditingMenuControllerImplInsertionMenuExecCommandTest
    : public TouchEditingMenuControllerImplTest,
      public testing::WithParamInterface<ExecCommandParams> {};

INSTANTIATE_TEST_CASE_P(_,
    TouchEditingMenuControllerImplInsertionMenuExecCommandTest,
    testing::Values(
        ExecCommandParams{WebContextMenuAction::Copy, true},
        ExecCommandParams{WebContextMenuAction::SelectAll, false}));

TEST_P(TouchEditingMenuControllerImplInsertionMenuExecCommandTest, _) {
  const auto& params = GetParam();

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
  EXPECT_CALL(menu_sink(), Show());

  controller()->OnSelectionEvent(ui::INSERTION_HANDLE_TAPPED);

  if (params.hides) {
    EXPECT_CALL(menu_sink(), Hide());
  }

  GetLastCreatedMenu()->client()->ExecuteCommand(params.action);

  if (params.hides) {
    EXPECT_EQ(ui::TouchSelectionController::INACTIVE,
              touch_selection_controller()->active_status());
  }

  base::RunLoop().RunUntilIdle();
  EXPECT_EQ(params.hides, !GetLastCreatedMenu());
}

TEST_F(TouchEditingMenuControllerImplTest, InsertionMenuClosed) {
  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::CENTER,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(-100, 0)));
  EXPECT_CALL(menu_sink(), Show());

  controller()->OnSelectionEvent(ui::INSERTION_HANDLE_TAPPED);

  EXPECT_CALL(menu_sink(), Hide());
  GetLastCreatedMenu()->client()->Close();

  EXPECT_EQ(ui::TouchSelectionController::INACTIVE,
            touch_selection_controller()->active_status());

  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(GetLastCreatedMenu());
}

TEST_F(TouchEditingMenuControllerImplTest, SelectionMenuClose) {
  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));

  EXPECT_CALL(menu_sink(), Hide());
  GetLastCreatedMenu()->client()->Close();

  EXPECT_EQ(ui::TouchSelectionController::INACTIVE,
            touch_selection_controller()->active_status());

  base::RunLoop().RunUntilIdle();
  EXPECT_FALSE(GetLastCreatedMenu());
}

TEST_F(TouchEditingMenuControllerImplTest, MenuWasResized) {
  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(250, 0)));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(
      MakeSelectionBound(gfx::SelectionBound::LEFT,
                         gfx::PointF(50, 50), gfx::PointF(50, 100), true),
      MakeSelectionBound(gfx::SelectionBound::RIGHT,
                         gfx::PointF(750, 50), gfx::PointF(750, 100), true));

  EXPECT_CALL(menu_sink(), SetOrigin(gfx::PointF(200, 110)));
  GetLastCreatedMenu()->set_size(gfx::Size(400, 100));
  GetLastCreatedMenu()->client()->WasResized();
}

namespace {

struct PositionTestParams {
  gfx::SelectionBound start;
  gfx::SelectionBound end;
  gfx::Size menu_size;
  gfx::RectF viewport_bounds;
  gfx::Rect window_bounds;
  float top_controls_height;
  float top_controls_shown_ratio;

  gfx::PointF expected_origin;
};

} // namespace

class TouchEditingMenuControllerImplPositionCalculationTest
    : public TouchEditingMenuControllerImplTest,
      public testing::WithParamInterface<PositionTestParams> {};

INSTANTIATE_TEST_CASE_P(_,
    TouchEditingMenuControllerImplPositionCalculationTest,
    testing::Values(
        PositionTestParams{
            MakeSelectionBound(gfx::SelectionBound::CENTER,
                               gfx::PointF(50, 80), gfx::PointF(50, 130), true),
            MakeSelectionBound(gfx::SelectionBound::CENTER,
                               gfx::PointF(50, 80), gfx::PointF(50, 130), true),
            gfx::Size(300, 50),
            gfx::RectF(600, 600, 800, 600),
            gfx::Rect(550, 550, 900, 700),
            0, 0,
            gfx::PointF(-50, 30)},
        PositionTestParams{
            MakeSelectionBound(gfx::SelectionBound::LEFT,
                               gfx::PointF(740, 40), gfx::PointF(740, 90), true),
            MakeSelectionBound(gfx::SelectionBound::RIGHT,
                               gfx::PointF(760, 40), gfx::PointF(760, 90), true),
            gfx::Size(300, 50),
            gfx::RectF(600, 600, 800, 600),
            gfx::Rect(550, 550, 900, 700),
            50, 1,
            gfx::PointF(550, 150)},
        PositionTestParams{
            MakeSelectionBound(gfx::SelectionBound::LEFT,
                               gfx::PointF(50, 40), gfx::PointF(50, 90), true),
            MakeSelectionBound(gfx::SelectionBound::RIGHT,
                               gfx::PointF(700, 40), gfx::PointF(700, 90), true),
            gfx::Size(300, 50),
            gfx::RectF(600, 600, 800, 170),
            gfx::Rect(500, 500, 1000, 370),
            50, 1,
            gfx::PointF(225, 40)},
        PositionTestParams{
            MakeSelectionBound(gfx::SelectionBound::CENTER,
                               gfx::PointF(50, 20), gfx::PointF(50, 70), true),
            MakeSelectionBound(gfx::SelectionBound::CENTER,
                               gfx::PointF(50, 20), gfx::PointF(50, 70), true),
            gfx::Size(300, 50),
            gfx::RectF(600, 600, 800, 600),
            gfx::Rect(550, 580, 900, 640),
            0, 0,
            gfx::PointF(-50, 80)},
        PositionTestParams{
            MakeSelectionBound(gfx::SelectionBound::LEFT,
                               gfx::PointF(740, 510), gfx::PointF(740, 560), true),
            MakeSelectionBound(gfx::SelectionBound::RIGHT,
                               gfx::PointF(760, 510), gfx::PointF(760, 560), true),
            gfx::Size(300, 50),
            gfx::RectF(600, 600, 800, 600),
            gfx::Rect(550, 550, 900, 700),
            50, 1,
            gfx::PointF(550, 500)},
        PositionTestParams{
            MakeSelectionBound(gfx::SelectionBound::LEFT,
                               gfx::PointF(220, 40), gfx::PointF(220, 90), true),
            MakeSelectionBound(gfx::SelectionBound::RIGHT,
                               gfx::PointF(240, 40), gfx::PointF(240, 90), true),
            gfx::Size(300, 50),
            gfx::RectF(600, 600, 250, 600),
            gfx::Rect(590, 590, 270, 620),
            0, 0,
            gfx::PointF(-10, 100)},
        PositionTestParams{
            MakeSelectionBound(gfx::SelectionBound::LEFT,
                               gfx::PointF(50, 30), gfx::PointF(50, 80), true),
            MakeSelectionBound(gfx::SelectionBound::RIGHT,
                               gfx::PointF(700, 30), gfx::PointF(700, 80), true),
            gfx::Size(300, 50),
            gfx::RectF(600, 600, 800, 110),
            gfx::Rect(590, 590, 820, 130),
            0, 0,
            gfx::PointF(225, -10)}));

TEST_P(TouchEditingMenuControllerImplPositionCalculationTest, _) {
  const auto& params = GetParam();

  SetInitialMenuSize(params.menu_size);
  controller()->SetViewportBounds(params.viewport_bounds);
  float top_controls_shown_height =
      params.top_controls_height * params.top_controls_shown_ratio;
  touch_selection_controller()->OnViewportChanged(
      gfx::RectF(0, top_controls_shown_height,
                 params.viewport_bounds.width(),
                 params.viewport_bounds.height() - top_controls_shown_height));
  controller()->SetTopLevelWindowBounds(params.window_bounds);
  chrome_controller()->SetTopControlsHeight(params.top_controls_height);
  cc::CompositorFrameMetadata metadata;
  metadata.top_controls_height = params.top_controls_height;
  metadata.top_controls_shown_ratio = params.top_controls_shown_ratio;
  chrome_controller()->FrameMetadataUpdated(
      base::make_optional(metadata.Clone()));

  EXPECT_CALL(web_contents_client(),
              CreateTouchEditingMenu(_, controller()));
  EXPECT_CALL(menu_sink(), SetOrigin(params.expected_origin));
  EXPECT_CALL(menu_sink(), Show());

  touch_selection_controller()->OnSelectionBoundsChanged(params.start,
                                                         params.end);
  if (params.start == params.end &&
      params.start.type() == gfx::SelectionBound::CENTER) {
    controller()->OnSelectionEvent(ui::INSERTION_HANDLE_TAPPED);
  }
}

} // namespace oxide
