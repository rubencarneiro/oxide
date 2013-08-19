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

#include "oxide_q_web_frame.h"

#include "qt/lib/browser/oxide_qt_web_frame.h"

class OxideQWebFramePrivate FINAL {
 public:
  OxideQWebFramePrivate(oxide::qt::WebFrame* owner) :
      owner_(owner) {}

  oxide::qt::WebFrame* owner() const { return owner_; }

 private:
  oxide::qt::WebFrame* owner_;
};

OxideQWebFrame::OxideQWebFrame(oxide::qt::WebFrame* owner) :
    QObject(),
    d_ptr(new OxideQWebFramePrivate(owner)) {}

OxideQWebFrame::~OxideQWebFrame() {}

QUrl OxideQWebFrame::url() const {
  Q_D(const OxideQWebFrame);

  return QUrl(QString::fromStdString(d->owner()->url().spec()));
}

OxideQWebFrame* OxideQWebFrame::parentFrame() const {
  Q_D(const OxideQWebFrame);

  return static_cast<oxide::qt::WebFrame *>(
      d->owner()->parent())->q_web_frame();
}

QList<OxideQWebFrame *> OxideQWebFrame::childFrames() const {
  Q_D(const OxideQWebFrame);

  QList<OxideQWebFrame *> list;

  std::vector<oxide::WebFrame *> frames = d->owner()->GetChildFrames();
  for (std::vector<oxide::WebFrame *>::iterator it = frames.begin();
       it != frames.end(); ++it) {
    list.append(static_cast<oxide::qt::WebFrame *>(*it)->q_web_frame());
  }

  return list;
}
