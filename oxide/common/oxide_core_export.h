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

#ifndef _OXIDE_COMMON_CORE_EXPORT_H_
#define _OXIDE_COMMON_CORE_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(OXIDE_CORE_IMPLEMENTATION)
#define OXIDE_CORE_EXPORT __attribute__((visibility("default")))
#else
#define OXIDE_CORE_EXPORT
#endif
#else // defined(COMPONENT_BUILD)
#define OXIDE_CORE_EXPORT
#endif

#endif // _OXIDE_COMMON_CORE_EXPORT_H_
