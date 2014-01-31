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

#include "oxide_file_utils.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/files/file_util_proxy.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop_proxy.h"

namespace oxide {

namespace {

class GetFileContentsJob FINAL : public base::RefCounted<GetFileContentsJob> {
 public:
  GetFileContentsJob() :
      file_(base::kInvalidPlatformFileValue) {}

  ~GetFileContentsJob() {
    if (file_ != base::kInvalidPlatformFileValue) {
      base::FileUtilProxy::Close(
          task_runner_.get(), file_,
          base::Bind(&GetFileContentsJob::OnFileClosed));
    }
  }

  bool Run(base::TaskRunner* task_runner,
           const base::FilePath& file_path,
           const FileUtils::GetFileContentsCallback& callback) {
    task_runner_ = task_runner;
    callback_ = callback;

    return base::FileUtilProxy::CreateOrOpen(
        task_runner_.get(), file_path,
        base::File::FLAG_OPEN | base::File::FLAG_READ,
        base::Bind(&GetFileContentsJob::OnFileOpened, this));
  }

 private:
  void OnFileOpened(base::File::Error error,
                    base::PassPlatformFile file,
                    bool created) {
    if (error != base::File::FILE_OK) {
      callback_.Run(error, NULL, -1);
      return;
    }

    file_ = file.ReleaseValue();
    if (!base::FileUtilProxy::GetFileInfoFromPlatformFile(
        task_runner_.get(), file_,
        base::Bind(&GetFileContentsJob::OnGotFileInfo, this))) {
      callback_.Run(base::File::FILE_ERROR_FAILED, NULL, -1);
    }
  }

  void OnGotFileInfo(base::File::Error error,
                     const base::File::Info& info) {
    if (error != base::File::FILE_OK) {
      callback_.Run(error, NULL, -1);
      return;
    }

    if (info.is_directory) {
      callback_.Run(base::File::FILE_ERROR_NOT_A_FILE, NULL, -1);
      return;
    }

    // XXX: info.size is int64, but Read() takes an int
    if (!base::FileUtilProxy::Read(
        task_runner_.get(), file_, 0, info.size,
        base::Bind(&GetFileContentsJob::OnGotData, this))) {
      callback_.Run(base::File::FILE_ERROR_FAILED, NULL, -1);
    }
  }

  void OnGotData(base::File::Error error,
                 const char* data,
                 int bytes_read) {
    if (error != base::File::FILE_OK) {
      callback_.Run(error, NULL, -1);
      return;
    }

    callback_.Run(base::File::FILE_OK, data, bytes_read);
  }

  static void OnFileClosed(base::File::Error error) {
    // XXX: Do something with the error?
  }

  scoped_refptr<base::TaskRunner> task_runner_;
  FileUtils::GetFileContentsCallback callback_;
  base::PlatformFile file_;

  DISALLOW_COPY_AND_ASSIGN(GetFileContentsJob);
};

} // namespace

// static
bool FileUtils::GetFileContents(base::TaskRunner* task_runner,
                                const base::FilePath& file_path,
                                const GetFileContentsCallback& callback) {
  scoped_refptr<GetFileContentsJob> job = new GetFileContentsJob();
  return job->Run(task_runner, file_path, callback);
}

} // namespace oxide
