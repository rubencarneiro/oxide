import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView

  TestCase {
    name: "Screen"
    when: TestSupport.testLoaded

    function getMockQPAShim() {
      return TestSupport.getAppProperty("_oxide_mock_qpa_shim_api");
    }

    function init() {
      getMockQPAShim().resetScreens();
      TestWindow.screen = TestSupport.screens[0];
    }

    function cleanupTestCase() {
      getMockQPAShim().resetScreens();
    }

    function validateScreenGeometry(msg) {
      compare(webView.getTestApi().evaluateCode("window.screen.width"),
              TestWindow.screen.geometry.width, msg);
      compare(webView.getTestApi().evaluateCode("window.screen.height"),
              TestWindow.screen.geometry.height, msg);
      compare(webView.getTestApi().evaluateCode("window.screen.availWidth"),
              TestWindow.screen.availableGeometry.width, msg);
      compare(webView.getTestApi().evaluateCode("window.screen.availHeight"),
              TestWindow.screen.availableGeometry.height, msg);
    }

    // Verify that screen geometry is reported correctly
    function test_Screen1_geometry() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      validateScreenGeometry();
    }

    // Verify that screen geometry is reported correctly after the window
    // is moved across screens
    function test_Screen2_geometry_after_screen_switch() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      validateScreenGeometry("Before screen switch");

      TestWindow.screen = TestSupport.screens[1];
      compare(TestWindow.screen, TestSupport.screens[1]);

      validateScreenGeometry("After screen switch");
    }

    // Verify that the screen geometry is reported correctly after the
    // screen geometry changes
    function test_Screen3_geometry_changes() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      validateScreenGeometry("With origin screen geometry");

      getMockQPAShim().setScreenGeometry(
          TestWindow.screen,
          Qt.rect(0, 0, 2160, 3840),
          Qt.rect(0, 100, 2160, 3740));
      compare(TestWindow.screen.geometry.width, 1080);

      validateScreenGeometry("With modified screen geometry");
    }
  }
}
