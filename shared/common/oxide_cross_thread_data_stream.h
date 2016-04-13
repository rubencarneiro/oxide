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

#ifndef _OXIDE_SHARED_BASE_CROSS_THREAD_DATA_STREAM_H_
#define _OXIDE_SHARED_BASE_CROSS_THREAD_DATA_STREAM_H_

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread_checker.h"

#include "shared/common/oxide_shared_export.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace net {
class IOBuffer;
}

namespace oxide {

class OXIDE_SHARED_EXPORT CrossThreadDataStream
    : public base::RefCountedThreadSafe<CrossThreadDataStream> {
 public:
  CrossThreadDataStream();

  bool Initialize(int size);

  int BytesAvailable() const;
  bool IsEOF() const;
  int Read(net::IOBuffer* buf, int buf_size);

  int Write(net::IOBuffer* buf, int buf_size, bool eof);

  void SetDataAvailableCallback(const base::Closure& callback);
  void SetDidReadCallback(const base::Closure& callback);

 protected:
  friend class base::RefCountedThreadSafe<CrossThreadDataStream>;
  virtual ~CrossThreadDataStream();

  bool CalledOnReadThread() const;
  bool CalledOnWriteThread() const;
  bool IsInitialized() const;

  bool CanAllocateSpaceForWriting() const;
  char* AllocateSpaceForWriting(int requested_size,
                                int* returned_size);
  void CommitWrite(bool eof);

  bool CanReadData() const;
  char* PeekReadData(int* returned_size);
  void ConsumeData(int size);

 private:
  int BytesAvailableLocked() const;

  void RunDataAvailableCallbackOnReadThread();
  void RunDidReadCallbackOnWriteThread();

  base::ThreadChecker read_thread_checker_;
  base::ThreadChecker write_thread_checker_;

  // Must only be accessed on the read thread
  base::Closure data_available_callback_;

  scoped_refptr<base::SingleThreadTaskRunner> read_thread_task_runner_;

  // Must only be accessed on the write thread
  base::Closure did_read_callback_;

  scoped_refptr<base::SingleThreadTaskRunner> write_thread_task_runner_;

  mutable base::Lock lock_;

  int buffer_size_;

  char* buffer_;

  int buffer_reserved_start_;
  int buffer_reserved_end_;
  int buffer_start_;
  int buffer_end_;
  bool eof_;

  DISALLOW_COPY_AND_ASSIGN(CrossThreadDataStream);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BASE_CROSS_THREAD_DATA_STREAM_H_
