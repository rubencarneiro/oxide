import QtQuick 2.0
import QtTest 1.0
import Oxide.testsupport 1.0

TestCase {
  property bool skipTestCase: false

  // We bind completed to skipTestCase to prevent qtest_run from being called
  completed: skipTestCase

  Component.onCompleted: {
    // Unbind completed from skipTestCase once we're constructed
    completed: skipTestCase ? true : false;

    // Signal to the test runner that we're skipped, so that it doesn't
    // exec tht event loop, which would hang indefinitely
    TestSupport.skipTestCase = skipTestCase;
  }
}
