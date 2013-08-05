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

#define IPC_MESSAGE_IMPL
#include "oxide_message_generator.h"

// Generate constructors.
#include "ipc/struct_constructor_macros.h"
#include "oxide_message_generator.h"

// Generate destructors.
#include "ipc/struct_destructor_macros.h"
#include "oxide_message_generator.h"

// Generate param traits write methods.
#include "ipc/param_traits_write_macros.h"
namespace IPC {
#include "oxide_message_generator.h"
}  // namespace IPC

// Generate param traits read methods.
#include "ipc/param_traits_read_macros.h"
namespace IPC {
#include "oxide_message_generator.h"
}  // namespace IPC

// Generate param traits log methods.
#include "ipc/param_traits_log_macros.h"
namespace IPC {
#include "oxide_message_generator.h"
}  // namespace IPC
