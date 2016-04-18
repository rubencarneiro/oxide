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

#include "base/bind.h"
#include "base/test/launcher/unit_test_launcher.h"
#include "content/public/test/unittest_test_suite.h"
#include "mojo/edk/embedder/embedder.h"

#include "oxide_test_suite.h"

int main(int argc, char** argv) {
  content::UnitTestTestSuite test_suite(new oxide::TestSuite(argc, argv));
  mojo::edk::Init();
  return base::LaunchUnitTests(
      argc, argv, base::Bind(&content::UnitTestTestSuite::Run,
                             base::Unretained(&test_suite)));
}
