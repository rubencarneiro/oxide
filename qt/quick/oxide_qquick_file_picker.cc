// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#include "oxide_qquick_file_picker.h"

#include <QDebug>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include "qt/core/glue/oxide_qt_file_picker_proxy_client.h"
#include "qt/quick/api/oxideqquickwebview.h"
#include "qt/quick/api/oxideqquickwebview_p.h"

namespace oxide {
namespace qquick {

class FilePickerContext : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool allowMultipleFiles READ allowMultipleFiles CONSTANT FINAL)
  Q_PROPERTY(bool directory READ directory CONSTANT FINAL)
  Q_PROPERTY(QString title READ title CONSTANT FINAL)
  Q_PROPERTY(QString defaultFileName READ defaultFileName CONSTANT FINAL)
  Q_PROPERTY(QStringList acceptTypes READ acceptTypes CONSTANT FINAL)

 public:
  virtual ~FilePickerContext() {}
  FilePickerContext(oxide::qt::FilePickerProxyClient* client,
                    oxide::qt::FilePickerProxy::Mode mode,
                    const QString& title,
                    const QFileInfo& default_file_name,
                    const QStringList& accept_types);

  bool allowMultipleFiles() const;
  bool directory() const;
  const QString& title() const { return title_; }
  const QString& defaultFileName() const { return default_file_name_; }
  const QStringList& acceptTypes() const { return accept_types_; }

 public Q_SLOTS:
  void accept(const QVariant& files) const;
  void reject() const;

 private:
  oxide::qt::FilePickerProxyClient* client_;
  oxide::qt::FilePickerProxy::Mode mode_;
  QString title_;
  QString default_file_name_;
  QStringList accept_types_;
};

FilePickerContext::FilePickerContext(
    oxide::qt::FilePickerProxyClient* client,
    oxide::qt::FilePickerProxy::Mode mode,
    const QString& title,
    const QFileInfo& default_file_name,
    const QStringList& accept_types)
    : client_(client),
      mode_(mode),
      title_(title),
      default_file_name_(default_file_name.filePath()),
      accept_types_(accept_types) {}

bool FilePickerContext::allowMultipleFiles() const {
  return (mode_ == oxide::qt::FilePickerProxy::OpenMultiple);
}

bool FilePickerContext::directory() const {
  return (mode_ == oxide::qt::FilePickerProxy::UploadFolder);
}

void FilePickerContext::accept(const QVariant& files) const {
  QFileInfoList info;
  Q_FOREACH(const QString& file, files.toStringList()) {
    if (QFileInfo::exists(file)) {
      info.append(QFileInfo(file));
    }
  }
  if ((info.size() > 1) && !allowMultipleFiles()) {
    qWarning() <<
        "FilePickerContext::accept: This file picker does not allow selecting "
        "multiple files";
    info.erase(info.begin() + 1, info.end());
  }
  client_->done(info, mode_);
}

void FilePickerContext::reject() const {
  client_->done(QFileInfoList(), mode_);
}

FilePicker::~FilePicker() {}

void FilePicker::Show(Mode mode,
                      const QString& title,
                      const QFileInfo& default_fileName,
                      const QStringList& accept_types) {
  if (!view_) {
    qWarning() << "FilePicker::Show: Can't show after the view has gone";
    client_->done(QFileInfoList(), mode);
    return;
  }

  FilePickerContext* contextObject =
      new FilePickerContext(client_, mode, title, default_fileName, accept_types);
  QQmlComponent* component = view_->filePicker();
  if (!component) {
    qWarning() <<
        "FilePicker::Show: Content requested a file picker, but the "
        "application hasn't provided one";
    delete contextObject;
    client_->done(QFileInfoList(), mode);
    return;
  }

  QQmlContext* baseContext = component->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(view_);
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextProperty(QLatin1String("model"), contextObject);
  context_->setContextObject(contextObject);
  contextObject->setParent(context_.data());

  item_.reset(qobject_cast<QQuickItem*>(component->beginCreate(context_.data())));
  if (!item_) {
    qWarning() <<
        "FilePicker::Show: Failed to create instance of Qml file picker "
        "component";
    context_.reset();
    client_->done(QFileInfoList(), mode);
    return;
  }

  OxideQQuickWebViewPrivate::get(view_)->addAttachedPropertyTo(item_.data());
  item_->setParentItem(view_);
  component->completeCreate();
}

void FilePicker::Hide() {
  if (!item_.isNull()) {
    item_->setVisible(false);
  }
}

FilePicker::FilePicker(OxideQQuickWebView* view,
                       oxide::qt::FilePickerProxyClient* client)
    : view_(view),
      client_(client) {}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_file_picker.moc"
