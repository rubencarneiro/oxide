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

#include "oxide_cross_thread_data_stream.h"

#include <cstdlib>
#include <cstring>

#include "base/bind.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/single_thread_task_runner.h"
#include "net/base/io_buffer.h"

namespace oxide {

int CrossThreadDataStream::BytesAvailableLocked() const {
  DCHECK(CalledOnReadThread());
  lock_.AssertAcquired();

  if (buffer_start_ == -1) {
    return 0;
  }

  int bytes = buffer_end_ - buffer_start_;
  if (bytes <= 0) {
    bytes = buffer_size_ + bytes;
  }

  return bytes;
}

CrossThreadDataStream::~CrossThreadDataStream() {
  if (buffer_) {
    free(buffer_);
    buffer_ = nullptr;
  }
}

bool CrossThreadDataStream::CalledOnReadThread() const {
  return read_thread_checker_.CalledOnValidThread();
}

bool CrossThreadDataStream::CalledOnWriteThread() const {
  return write_thread_checker_.CalledOnValidThread();
}

bool CrossThreadDataStream::IsInitialized() const {
  return buffer_ != nullptr;
}

bool CrossThreadDataStream::CanAllocateSpaceForWriting() const {
  DCHECK(CalledOnWriteThread());

  base::AutoLock lock(lock_);
  DCHECK(IsInitialized());

  if (eof_) {
    return false;
  }

  if (buffer_start_ == -1) {
    DCHECK_EQ(buffer_end_, -1);
    return true;
  }

  if (buffer_start_ == buffer_end_) {
    return false;
  }

  if (buffer_start_ == 0 && buffer_end_ == buffer_size_) {
    return false;
  }

  return true;
}

char* CrossThreadDataStream::AllocateSpaceForWriting(int requested_size,
                                                     int* returned_size) {
  DCHECK(CalledOnWriteThread());
  DCHECK(CanAllocateSpaceForWriting());
  DCHECK(returned_size);

  base::AutoLock lock(lock_);

  DCHECK_EQ(buffer_reserved_start_, -1);
  DCHECK_EQ(buffer_reserved_end_, -1);

  if (requested_size > buffer_size_) {
    requested_size = buffer_size_;
  }

  if (buffer_start_ == -1) {
    // This is the first allocation
    buffer_reserved_start_ = 0;
    buffer_reserved_end_ = requested_size;
    *returned_size = requested_size;
    return buffer_; 
  }

  DCHECK_NE(buffer_end_, -1);

  if (buffer_start_ < buffer_end_) {
    // This is the non wrap-around case
    if (buffer_end_ == buffer_size_) {
      // We've reached the end of the buffer, so wrap around
      buffer_reserved_start_ = 0;

      if (buffer_start_ < requested_size) {
        *returned_size = buffer_reserved_end_ = buffer_start_;
      } else {
        *returned_size = buffer_reserved_end_ = requested_size;
      }
    } else {
      // We have space at the end of the buffer
      buffer_reserved_start_ = buffer_end_;

      if ((buffer_end_ + requested_size) > buffer_size_) {
        // There is insufficient space at the end of the buffer
        *returned_size = buffer_size_ - buffer_end_;
        buffer_reserved_end_ = buffer_size_;
      } else {
        // We can do the full allocation at the end of the buffer
        *returned_size = requested_size;
        buffer_reserved_end_ = buffer_end_ + requested_size;
      }
    }
  } else {
    // This is the wrap-around case
    buffer_reserved_start_ = buffer_end_;

    if ((buffer_end_ + requested_size) > buffer_start_) {
      // There is insufficient space at the start of the buffer
      *returned_size = buffer_start_ - buffer_end_;
      buffer_reserved_end_ = buffer_start_;
    } else {
      // We can do the full allocation at the start of the buffer
      *returned_size = requested_size;
      buffer_reserved_end_ = buffer_end_ + requested_size;
    }
  }

  DCHECK_GT(buffer_reserved_start_, -1);
  DCHECK_GT(buffer_reserved_end_, -1);
  DCHECK_LE(buffer_reserved_end_, buffer_size_);
  DCHECK_LT(buffer_reserved_start_, buffer_reserved_end_);

  return buffer_ + buffer_reserved_start_;

}

void CrossThreadDataStream::CommitWrite(bool eof) {
  DCHECK(CalledOnWriteThread());

  base::Closure callback;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner;

  {
    base::AutoLock lock(lock_);
    DCHECK(IsInitialized());
    DCHECK(!eof_);
    DCHECK_GT(buffer_reserved_start_, -1);
    DCHECK_GT(buffer_reserved_end_, -1);
    DCHECK(buffer_start_ == -1 ||
           (buffer_end_ == buffer_size_ && buffer_reserved_start_ == 0) ||
           buffer_reserved_start_ == buffer_end_);
    DCHECK(buffer_end_ == -1 ||
           (buffer_end_ == buffer_size_ && buffer_reserved_end_ < buffer_end_) ||
           buffer_reserved_end_ > buffer_end_);

    if (buffer_start_ == -1) {
      buffer_start_ = buffer_reserved_start_;
    }
    buffer_end_ = buffer_reserved_end_;
    eof_ = eof;

    buffer_reserved_start_ = -1;
    buffer_reserved_end_ = -1;

    callback = data_available_callback_;
    task_runner = read_thread_task_runner_;
  }

  if (!callback.is_null()) {
    task_runner->PostTask(FROM_HERE, callback);
  }
}

bool CrossThreadDataStream::CanReadData() const {
  DCHECK(CalledOnReadThread());

  base::AutoLock lock(lock_);
  DCHECK(IsInitialized());

  if (buffer_start_ == -1) {
    return false;
  }

  return true;
}

char* CrossThreadDataStream::PeekReadData(int* returned_size) {
  DCHECK(CalledOnReadThread());
  DCHECK(CanReadData());
  DCHECK(returned_size);

  base::AutoLock lock(lock_);

  if (buffer_end_ > buffer_start_) {
    *returned_size = buffer_end_ - buffer_start_;
  } else {
    *returned_size = buffer_size_ - buffer_end_;
  }

  return buffer_ + buffer_start_;
}

void CrossThreadDataStream::ConsumeData(int size) {
  DCHECK(CalledOnReadThread());
  DCHECK(CanReadData());

  base::Closure callback;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner;

  {
    base::AutoLock lock(lock_);

    DCHECK((buffer_start_ < buffer_end_ && buffer_start_ + size <= buffer_end_) ||
           (buffer_start_ >= buffer_end_ && buffer_start_ + size <= buffer_size_));

    buffer_start_ += size;
    if (buffer_start_ == buffer_end_) {
      buffer_start_ = buffer_end_ = -1;
    } else if (buffer_start_ == buffer_size_) {
      DCHECK_GT(buffer_end_, 0);
      buffer_start_ = 0;
    }

    callback = did_read_callback_;
    task_runner = write_thread_task_runner_;
  }

  if (!callback.is_null()) {
    task_runner->PostTask(FROM_HERE, callback);
  }
}

CrossThreadDataStream::CrossThreadDataStream()
    : buffer_size_(0),
      buffer_(nullptr),
      buffer_reserved_start_(-1),
      buffer_reserved_end_(-1),
      buffer_start_(-1),
      buffer_end_(-1),
      eof_(false) {
  read_thread_checker_.DetachFromThread();
  write_thread_checker_.DetachFromThread();
}

bool CrossThreadDataStream::Initialize(int size) {
  base::AutoLock lock(lock_);

  DCHECK(!IsInitialized());

  if (size <= 0) {
    return false;
  }

  buffer_ = static_cast<char*>(malloc(size));

  if (!buffer_) {
    return false;
  }

  buffer_size_ = size;
  return true;
}

int CrossThreadDataStream::BytesAvailable() const {
  DCHECK(CalledOnReadThread());

  base::AutoLock lock(lock_);
  return BytesAvailableLocked();
}

bool CrossThreadDataStream::IsEOF() const {
  DCHECK(CalledOnReadThread());

  base::AutoLock lock(lock_);
  return eof_ && BytesAvailableLocked() == 0;
}

int CrossThreadDataStream::Read(net::IOBuffer* buf, int buf_size) {
  DCHECK(CalledOnReadThread());

  int bytes_read = 0;

  while (CanReadData() && bytes_read < buf_size) {
    int size = 0;
    char* memory = PeekReadData(&size);
    size = std::min(size, buf_size);
    memcpy(buf->data() + bytes_read, memory, size);
    ConsumeData(size);

    bytes_read += size;
  }

  return bytes_read;
}

int CrossThreadDataStream::Write(net::IOBuffer* buf, int buf_size, bool eof) {
  DCHECK(CalledOnWriteThread());

  int bytes_written = 0;

  while (CanAllocateSpaceForWriting() && bytes_written < buf_size) {
    int allocated = 0;
    char* memory = AllocateSpaceForWriting(buf_size - bytes_written, &allocated);
    memcpy(memory, buf->data() + bytes_written, allocated);

    bytes_written += allocated;

    CommitWrite(eof && bytes_written == buf_size);
  }

  return bytes_written;
}

void CrossThreadDataStream::SetDataAvailableCallback(
    const base::Closure& callback) {
  DCHECK(CalledOnReadThread());

  base::AutoLock lock(lock_);

  data_available_callback_= callback;
  read_thread_task_runner_ = base::MessageLoopProxy::current();
}

void CrossThreadDataStream::SetDidReadCallback(
    const base::Closure& callback) {
  DCHECK(CalledOnWriteThread());

  base::AutoLock lock(lock_);

  did_read_callback_ = callback;
  write_thread_task_runner_ = base::MessageLoopProxy::current();
}

} // namespace oxide
