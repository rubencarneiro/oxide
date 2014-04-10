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

#include "oxide_qquick_file_picker_delegate.h"

#include <QDebug>
#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include "qt/quick/api/oxideqquickwebview_p.h"
#include "qt/quick/api/oxideqquickwebview_p_p.h"

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
  FilePickerContext(FilePickerDelegate* delegate,
                    oxide::qt::FilePickerDelegate::Mode mode,
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
  FilePickerDelegate* delegate_;
  oxide::qt::FilePickerDelegate::Mode mode_;
  QString title_;
  QString default_file_name_;
  QStringList accept_types_;
};

FilePickerContext::FilePickerContext(
    FilePickerDelegate* delegate,
    oxide::qt::FilePickerDelegate::Mode mode,
    const QString& title,
    const QFileInfo& default_file_name,
    const QStringList& accept_types) :
    delegate_(delegate),
    mode_(mode),
    title_(title),
    default_file_name_(default_file_name.filePath()),
    accept_types_(accept_types) {}

bool FilePickerContext::allowMultipleFiles() const {
  return (mode_ == oxide::qt::FilePickerDelegate::OpenMultiple);
}

bool FilePickerContext::directory() const {
  return (mode_ == oxide::qt::FilePickerDelegate::UploadFolder);
}

void FilePickerContext::accept(const QVariant& files) const {
  QFileInfoList info;
  Q_FOREACH(const QString& file, files.toStringList()) {
    info.append(QFileInfo(file));
  }
  if ((info.size() > 1) && !allowMultipleFiles()) {
    qWarning() << "This file picker does not allow selecting multiple files";
    info.erase(info.begin() + 1, info.end());
  }
  delegate_->Done(info, mode_);
}

void FilePickerContext::reject() const {
  delegate_->Done(QFileInfoList(), mode_);
}

FilePickerDelegate::FilePickerDelegate(OxideQQuickWebView* webview) :
    web_view_(webview) {}

void FilePickerDelegate::Show(Mode mode,
                              const QString& title,
                              const QFileInfo& defaultFileName,
                              const QStringList& acceptTypes) {
  FilePickerContext* contextObject =
      new FilePickerContext(this, mode, title, defaultFileName, acceptTypes);
  QQmlComponent* component = web_view_->filePicker();
  if (!component) {
    qWarning() << "Content requested a file picker, "
                  "but the application hasn't provided one";
    delete contextObject;
    Done(QFileInfoList(), mode);
    return;
  }
  QQmlContext* baseContext = component->creationContext();
  if (!baseContext) {
    baseContext = QQmlEngine::contextForObject(web_view_);
  }
  context_.reset(new QQmlContext(baseContext));

  context_->setContextProperty(QLatin1String("model"), contextObject);
  context_->setContextObject(contextObject);
  contextObject->setParent(context_.data());

  item_.reset(qobject_cast<QQuickItem*>(component->beginCreate(context_.data())));
  if (!item_) {
    qWarning() << "Failed to create file picker";
    context_.reset();
    Done(QFileInfoList(), mode);
    return;
  }

  OxideQQuickWebViewPrivate::get(web_view_)->addAttachedPropertyTo(item_.data());
  item_->setParentItem(web_view_);
  component->completeCreate();
}

void FilePickerDelegate::Hide() {
  if (!item_.isNull()) {
    item_->setVisible(false);
  }
}

} // namespace qquick
} // namespace oxide

#include "oxide_qquick_file_picker_delegate.moc"
