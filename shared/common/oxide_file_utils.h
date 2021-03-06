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

#ifndef _OXIDE_SHARED_COMMON_FILE_UTILS_H_
#define _OXIDE_SHARED_COMMON_FILE_UTILS_H_

#include "base/callback.h"
#include "base/files/file.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"

#include "shared/common/oxide_shared_export.h"

namespace base {
class FilePath;
class TaskRunner;
}

namespace oxide {

class AsyncFileJobImpl;

class AsyncFileJob {
 public:
  virtual ~AsyncFileJob();
  AsyncFileJob(AsyncFileJobImpl* impl);

 protected:
  scoped_refptr<AsyncFileJobImpl> impl_;
};

class OXIDE_SHARED_EXPORT FileUtils final {
 public:
  typedef base::Callback<void(base::File::Error,
                              const char*,
                              int)> GetFileContentsCallback;

  static AsyncFileJob* GetFileContents(
      base::TaskRunner* task_runner,
      const base::FilePath& file_path,
      const GetFileContentsCallback& callback);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(FileUtils);
};

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_FILE_UTILS_H_
