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

#include "contents_view_impl.h"

#include <QCursor>
#include <QEvent>
#include <QImage>
#include <QKeyEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPixmap>
#include <QRect>
#include <QTouchEvent>
#include <QWindow>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_vector.h"
#include "content/common/cursors/webcursor.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/common/drop_data.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "third_party/WebKit/public/platform/WebInputEvent.h"
#include "ui/events/gesture_detection/motion_event.h"
#include "ui/gfx/geometry/point.h"

#include "qt/core/browser/input/oxide_qt_input_method_context.h"
#include "qt/core/glue/contents_view_client.h"
#include "qt/core/glue/web_popup_menu.h"
#include "shared/browser/chrome_controller.h"
#include "shared/browser/compositor/oxide_compositor_frame_data.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/browser/oxide_web_contents_view.h"

#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_drag_utils.h"
#include "oxide_qt_event_utils.h"
#include "oxide_qt_screen_utils.h"
#include "oxide_qt_skutils.h"
#include "oxide_qt_touch_handle_drawable.h"
#include "oxide_qt_type_conversions.h"
#include "qt_screen.h"
#include "qt_web_popup_menu.h"

namespace oxide {
namespace qt {

using oxide::ChromeController;

namespace {

inline QCursor QCursorFromWebCursor(blink::WebCursorInfo::Type type) {
  Qt::CursorShape cs = Qt::ArrowCursor;
  switch (type) {
  case blink::WebCursorInfo::TypeCross:
    cs = Qt::CrossCursor;
    break;

  case blink::WebCursorInfo::TypeHand:
    cs = Qt::PointingHandCursor;
    break;

  case blink::WebCursorInfo::TypeCell:
  case blink::WebCursorInfo::TypeIBeam:
    cs = Qt::IBeamCursor;
    break;

  case blink::WebCursorInfo::TypeWait:
    cs = Qt::WaitCursor;
    break;

  case blink::WebCursorInfo::TypeHelp:
    cs = Qt::WhatsThisCursor;
    break;

  case blink::WebCursorInfo::TypeEastResize:
  case blink::WebCursorInfo::TypeWestResize:
  case blink::WebCursorInfo::TypeEastWestResize:
    cs = Qt::SizeHorCursor;
    break;

  case blink::WebCursorInfo::TypeNorthResize:
  case blink::WebCursorInfo::TypeSouthResize:
  case blink::WebCursorInfo::TypeNorthSouthResize:
    cs = Qt::SizeVerCursor;
    break;

  case blink::WebCursorInfo::TypeNorthEastResize:
  case blink::WebCursorInfo::TypeSouthWestResize:
    cs = Qt::SizeBDiagCursor;
    break;

  case blink::WebCursorInfo::TypeNorthWestResize:
  case blink::WebCursorInfo::TypeSouthEastResize:
    cs = Qt::SizeFDiagCursor;
    break;

  case blink::WebCursorInfo::TypeNorthEastSouthWestResize:
  case blink::WebCursorInfo::TypeNorthWestSouthEastResize:
  case blink::WebCursorInfo::TypeMove:
    cs = Qt::SizeAllCursor;
    break;

  case blink::WebCursorInfo::TypeColumnResize:
    cs = Qt::SplitHCursor;
    break;

  case blink::WebCursorInfo::TypeRowResize:
    cs = Qt::SplitVCursor;
    break;

  case blink::WebCursorInfo::TypeMiddlePanning:
  case blink::WebCursorInfo::TypeEastPanning:
  case blink::WebCursorInfo::TypeNorthPanning:
  case blink::WebCursorInfo::TypeNorthEastPanning:
  case blink::WebCursorInfo::TypeNorthWestPanning:
  case blink::WebCursorInfo::TypeSouthPanning:
  case blink::WebCursorInfo::TypeSouthEastPanning:
  case blink::WebCursorInfo::TypeSouthWestPanning:
  case blink::WebCursorInfo::TypeWestPanning:
  case blink::WebCursorInfo::TypeGrab:
  case blink::WebCursorInfo::TypeGrabbing:
    cs = Qt::ClosedHandCursor;
    break;

  case blink::WebCursorInfo::TypeProgress:
    cs = Qt::BusyCursor;
    break;

  case blink::WebCursorInfo::TypeNoDrop:
  case blink::WebCursorInfo::TypeNotAllowed:
    cs = Qt::ForbiddenCursor;
    break;

  case blink::WebCursorInfo::TypeCopy:
  case blink::WebCursorInfo::TypeContextMenu:
  case blink::WebCursorInfo::TypeVerticalText:
  case blink::WebCursorInfo::TypeAlias:
  case blink::WebCursorInfo::TypeZoomIn:
  case blink::WebCursorInfo::TypeZoomOut:
  case blink::WebCursorInfo::TypeCustom:
  case blink::WebCursorInfo::TypePointer:
  case blink::WebCursorInfo::TypeNone:
  default:
    break;
  }

  return QCursor(cs);
}

}

class CompositorFrameHandleImpl : public CompositorFrameHandle {
 public:
  CompositorFrameHandleImpl(oxide::CompositorFrameHandle* frame,
                            float location_bar_content_offset,
                            QScreen* screen);
  ~CompositorFrameHandleImpl() override {}

  CompositorFrameHandle::Type GetType() override;
  const QRectF& GetRect() const override;
  const QSize& GetSizeInPixels() const override;
  QImage GetSoftwareFrame() override;
  unsigned int GetAcceleratedFrameTexture() override;
  EGLImageKHR GetImageFrame() override;

 private:
  scoped_refptr<oxide::CompositorFrameHandle> frame_;
  QRectF rect_;
  QSize size_in_pixels_;
};

CompositorFrameHandleImpl::CompositorFrameHandleImpl(
    oxide::CompositorFrameHandle* frame,
    float location_bar_content_offset,
    QScreen* screen)
    : frame_(frame) {
  if (!frame_) {
    return;
  }

  size_in_pixels_ = QSize(frame->data()->size_in_pixels.width(),
                          frame->data()->size_in_pixels.height());

  gfx::RectF rect =
      gfx::ScaleRect(gfx::RectF(gfx::SizeF(ToChromium(size_in_pixels_))),
                     1 / frame->data()->device_scale);
  rect += gfx::Vector2dF(0, location_bar_content_offset);

  rect_ = ToQt(DpiUtils::ConvertChromiumPixelsToQt(rect, screen));
}

CompositorFrameHandle::Type CompositorFrameHandleImpl::GetType() {
  if (!frame_.get()) {
    return CompositorFrameHandle::TYPE_INVALID;
  }
  if (frame_->data()->gl_frame_data) {
    DCHECK_NE(frame_->data()->gl_frame_data->type,
              oxide::GLFrameData::Type::INVALID);
    if (frame_->data()->gl_frame_data->type ==
        oxide::GLFrameData::Type::TEXTURE) {
      return CompositorFrameHandle::TYPE_ACCELERATED;
    }
    return CompositorFrameHandle::TYPE_IMAGE;
  }
  if (frame_->data()->software_frame_data) {
    return CompositorFrameHandle::TYPE_SOFTWARE;
  }

  NOTREACHED();
  return CompositorFrameHandle::TYPE_INVALID;
}

const QRectF& CompositorFrameHandleImpl::GetRect() const {
  return rect_;
}

const QSize& CompositorFrameHandleImpl::GetSizeInPixels() const {
  return size_in_pixels_;
}

QImage CompositorFrameHandleImpl::GetSoftwareFrame() {
  DCHECK_EQ(GetType(), CompositorFrameHandle::TYPE_SOFTWARE);
  return QImage(
      frame_->data()->software_frame_data->pixels->front(),
      frame_->data()->size_in_pixels.width(),
      frame_->data()->size_in_pixels.height(),
      QImage::Format_ARGB32);
}

unsigned int CompositorFrameHandleImpl::GetAcceleratedFrameTexture() {
  DCHECK_EQ(GetType(), CompositorFrameHandle::TYPE_ACCELERATED);
  return frame_->data()->gl_frame_data->resource.texture;
}

EGLImageKHR CompositorFrameHandleImpl::GetImageFrame() {
  return frame_->data()->gl_frame_data->resource.egl_image;
}

QSharedPointer<CompositorFrameHandle> ContentsViewImpl::compositorFrameHandle() {
  if (!compositor_frame_) {
    compositor_frame_ =
        QSharedPointer<CompositorFrameHandle>(
            new CompositorFrameHandleImpl(view()->GetCompositorFrameHandle(),
                                          GetTopContentOffset(),
                                          GetScreen()));
  }

  return compositor_frame_;
}

void ContentsViewImpl::didCommitCompositorFrame() {
  view()->DidCommitCompositorFrame();
}

void ContentsViewImpl::windowChanged() {
  if (window_) {
    window_->disconnect(this);
  }

  window_ = client_->GetWindow();

  if (window_) {
    connect(window_, SIGNAL(screenChanged(QScreen*)),
            SLOT(OnScreenChanged()));
  }

  if (!view()) {
    return;
  }

  view()->ScreenChanged();
  view()->WasResized();
}

void ContentsViewImpl::wasResized() {
  view()->WasResized();
}

void ContentsViewImpl::visibilityChanged() {
  view()->VisibilityChanged();
}

void ContentsViewImpl::activeFocusChanged() {
  view()->FocusChanged();
}

QVariant ContentsViewImpl::inputMethodQuery(Qt::InputMethodQuery query) const {
  return input_method_context_->Query(query);
}

void ContentsViewImpl::handleKeyEvent(QKeyEvent* event) {
  content::NativeWebKeyboardEvent e(MakeNativeWebKeyboardEvent(event, false));
  view()->HandleKeyEvent(e);

  // If the event is a printable character, send a corresponding Char event
  if (event->type() == QEvent::KeyPress && e.text[0] != 0) {
    view()->HandleKeyEvent(MakeNativeWebKeyboardEvent(event, true));
  }

  event->accept();
}

void ContentsViewImpl::handleInputMethodEvent(QInputMethodEvent* event) {
  input_method_context_->HandleEvent(event);
  event->accept();
}

void ContentsViewImpl::handleFocusEvent(QFocusEvent* event) {
  input_method_context_->FocusChanged(event);

  event->accept();
}

void ContentsViewImpl::handleMouseEvent(QMouseEvent* event) {
  if (!(event->button() == Qt::LeftButton ||
        event->button() == Qt::MidButton ||
        event->button() == Qt::RightButton ||
        event->button() == Qt::NoButton)) {
    event->ignore();
    return;
  }

  view()->HandleMouseEvent(
      MakeWebMouseEvent(event,
                        GetScreen(),
                        GetTopContentOffset()));
  event->accept();
}

void ContentsViewImpl::handleTouchUngrabEvent() {
  std::unique_ptr<ui::MotionEvent> cancel_event = motion_event_factory_.Cancel();
  view()->HandleMotionEvent(*cancel_event.get());
}

void ContentsViewImpl::handleWheelEvent(QWheelEvent* event,
                                        const QPointF& window_pos) {
  view()->HandleWheelEvent(
      MakeWebMouseWheelEvent(event,
                             window_pos,
                             GetScreen(),
                             GetTopContentOffset()));
  event->accept();
}

void ContentsViewImpl::handleTouchEvent(QTouchEvent* event) {
  MotionEventFactory::ResultVector events;
  motion_event_factory_.MakeMotionEvents(event,
                                         GetScreen(),
                                         GetTopContentOffset(),
                                         &events);

  for (const auto& e : events) {
    view()->HandleMotionEvent(*e.get());
  }

  event->accept();
}

void ContentsViewImpl::handleHoverEvent(QHoverEvent* event,
                                        const QPointF& window_pos,
                                        const QPoint& global_pos) {
  view()->HandleMouseEvent(
      MakeWebMouseEvent(event,
                        window_pos,
                        global_pos,
                        GetScreen(),
                        GetTopContentOffset()));
  event->accept();
}

void ContentsViewImpl::handleDragEnterEvent(QDragEnterEvent* event) {
  content::DropData drop_data;
  gfx::Point location;
  blink::WebDragOperationsMask allowed_ops = blink::WebDragOperationNone;
  int key_modifiers = 0;

  GetDragEnterEventParams(event,
                          GetScreen(),
                          GetTopContentOffset(),
                          &drop_data,
                          &location,
                          &allowed_ops,
                          &key_modifiers);

  view()->HandleDragEnter(drop_data, location, allowed_ops, key_modifiers);

  event->accept();
}

void ContentsViewImpl::handleDragMoveEvent(QDragMoveEvent* event) {
  gfx::Point location;
  int key_modifiers = 0;

  GetDropEventParams(event,
                     GetScreen(),
                     GetTopContentOffset(),
                     &location, &key_modifiers);

  blink::WebDragOperation op = view()->HandleDragMove(location, key_modifiers);

  Qt::DropAction action;
  if ((action = ToQtDropAction(op)) != Qt::IgnoreAction) {
    event->setDropAction(action);
    event->accept();
  } else {
    event->ignore();
  }
}

void ContentsViewImpl::handleDragLeaveEvent(QDragLeaveEvent* event) {
  view()->HandleDragLeave();
}

void ContentsViewImpl::handleDropEvent(QDropEvent* event) {
  gfx::Point location;
  int key_modifiers = 0;

  GetDropEventParams(event,
                     GetScreen(),
                     GetTopContentOffset(),
                     &location, &key_modifiers);

  blink::WebDragOperation op = view()->HandleDrop(location, key_modifiers);

  Qt::DropAction action;
  if ((action = ToQtDropAction(op)) != Qt::IgnoreAction) {
    event->setDropAction(action);
    event->accept();
  } else {
    event->ignore();
  }
}

void ContentsViewImpl::hideTouchSelectionController() {
  view()->HideTouchSelectionController();
}

float ContentsViewImpl::GetTopContentOffset() const {
  return view()->chrome_controller()->GetTopContentOffset();
}

void ContentsViewImpl::SetInputMethodEnabled(bool enabled) {
  client_->SetInputMethodEnabled(enabled);
}

bool ContentsViewImpl::IsVisible() const {
  return client_->IsVisible();
}

bool ContentsViewImpl::HasFocus() const {
  return client_->HasFocus();
}

gfx::RectF ContentsViewImpl::GetBounds() const {
  QRect bounds = client_->GetBounds();
  return DpiUtils::ConvertQtPixelsToChromium(
      gfx::RectF(ToChromium(bounds)),
      GetScreen());
}

void ContentsViewImpl::SwapCompositorFrame() {
  compositor_frame_.reset();
  client_->ScheduleUpdate();
}

void ContentsViewImpl::EvictCurrentFrame() {
  compositor_frame_.reset();
  client_->EvictCurrentFrame();
}

void ContentsViewImpl::UpdateCursor(const content::WebCursor& cursor) {
  content::WebCursor::CursorInfo cursor_info;

  cursor.GetCursorInfo(&cursor_info);
  if (cursor.IsCustom()) {
    QImage cursor_image = QImageFromSkBitmap(cursor_info.custom_image);
    if (cursor_image.isNull()) {
      return;
    }

    QPixmap cursor_pixmap = QPixmap::fromImage(cursor_image);
    client_->UpdateCursor(QCursor(cursor_pixmap));
  } else {
    client_->UpdateCursor(QCursorFromWebCursor(cursor_info.type));
  }
}

std::unique_ptr<oxide::WebPopupMenu> ContentsViewImpl::CreatePopupMenu(
    const std::vector<content::MenuItem>& items,
    int selected_index,
    bool allow_multiple_selection,
    const gfx::Rect& bounds,
    oxide::WebPopupMenuClient* client) {
  std::unique_ptr<WebPopupMenuImpl> menu =
      base::MakeUnique<WebPopupMenuImpl>(client);
  QRect qt_bounds =
      ToQt(DpiUtils::ConvertChromiumPixelsToQt(bounds, GetScreen()));
  if (!menu->Init(client_->CreateWebPopupMenu(
          WebPopupMenuImpl::BuildMenuItems(items),
          allow_multiple_selection,
          qt_bounds,
          menu.get()))) {
    return nullptr;
  }

  return std::move(menu);
}

ui::TouchHandleDrawable* ContentsViewImpl::CreateTouchHandleDrawable() const {
  TouchHandleDrawable* drawable = new TouchHandleDrawable(this);
  drawable->SetProxy(client_->CreateTouchHandleDrawable());
  return drawable;
}

void ContentsViewImpl::TouchSelectionChanged(
    ui::TouchSelectionController::ActiveStatus status,
    const gfx::RectF& bounds,
    bool handle_drag_in_progress,
    bool insertion_handle_tapped) const {
  TouchSelectionControllerActiveStatus active_status;
  switch (status) {
  case ui::TouchSelectionController::INACTIVE:
    active_status = ACTIVE_STATUS_INACTIVE;
    break;
  case ui::TouchSelectionController::INSERTION_ACTIVE:
    active_status = ACTIVE_STATUS_INSERTION_ACTIVE;
    break;
  case ui::TouchSelectionController::SELECTION_ACTIVE:
    active_status = ACTIVE_STATUS_SELECTION_ACTIVE;
    break;
  default:
    Q_UNREACHABLE();
  }

  client_->TouchSelectionChanged(
      active_status,
      ToQt(DpiUtils::ConvertChromiumPixelsToQt(bounds, GetScreen())),
      handle_drag_in_progress,
      insertion_handle_tapped);
}

void ContentsViewImpl::ContextMenuIntercepted() const {
  client_->ContextMenuIntercepted();
}

oxide::InputMethodContext* ContentsViewImpl::GetInputMethodContext() const {
  return input_method_context_.get();
}

void ContentsViewImpl::UnhandledKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {
  if (event.skip_in_browser) {
    return;
  }

  if (event.type() != blink::WebInputEvent::RawKeyDown &&
      event.type() != blink::WebInputEvent::KeyUp) {
    return;
  }

  if (!event.extra_data()) {
    return;
  }

  QKeyEvent* key_event = reinterpret_cast<QKeyEvent*>(event.extra_data());
  DCHECK(!key_event->isAccepted());

  client_->HandleUnhandledKeyboardEvent(key_event);
}

void ContentsViewImpl::OnScreenChanged() {
  view()->ScreenChanged();
}

ContentsViewImpl::ContentsViewImpl(ContentsViewClient* client,
                                   QObject* native_view)
    : client_(client),
      native_view_(native_view),
      input_method_context_(new InputMethodContext(this)) {
  DCHECK(!client_->view_);
  client_->view_ = this;

  windowChanged();
}

ContentsViewImpl::~ContentsViewImpl() {
  DCHECK_EQ(client_->view_, this);
  client_->view_ = nullptr;
  input_method_context_->DetachClient();
}

// static
ContentsViewImpl* ContentsViewImpl::FromWebContents(
    content::WebContents* contents) {
  oxide::WebContentsView* view =
      oxide::WebContentsView::FromWebContents(contents);
  if (!view) {
    return nullptr;
  }

  return static_cast<ContentsViewImpl*>(view->client());
}

content::WebContents* ContentsViewImpl::GetWebContents() const {
  if (!view()) {
    return nullptr;
  }

  return view()->GetWebContents();
}

display::Display ContentsViewImpl::GetDisplay() const {
  return Screen::GetInstance()->DisplayFromQScreen(GetScreen());
}

QScreen* ContentsViewImpl::GetScreen() const {
  QScreen* screen = nullptr;
  if (window_) {
    screen = window_->screen();
  }
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }

  return screen;
}

} // namespace qt
} // namespace oxide
