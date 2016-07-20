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

    function validateScreenGeometry() {
      compare(webView.getTestApi().evaluateCode("window.screen.width"),
              TestWindow.screen.geometry.width);
      compare(webView.getTestApi().evaluateCode("window.screen.height"),
              TestWindow.screen.geometry.height);
      compare(webView.getTestApi().evaluateCode("window.screen.availWidth"),
              TestWindow.screen.availableGeometry.width);
      compare(webView.getTestApi().evaluateCode("window.screen.availHeight"),
              TestWindow.screen.availableGeometry.height);
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

      validateScreenGeometry();

      TestWindow.screen = TestSupport.screens[1];
      compare(TestWindow.screen, TestSupport.screens[1]);

      validateScreenGeometry();
    }

    // Verify that the screen geometry is reported correctly after the
    // screen geometry changes
    function test_Screen3_geometry_changes() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      validateScreenGeometry();

      getMockQPAShim().overrideScreenGeometry(
          TestWindow.screen,
          Qt.rect(0, 0, 1080, 1920),
          Qt.rect(0, 100, 1080, 1820));
      compare(TestWindow.screen.geometry.width, 1080);

      validateScreenGeometry();
    }
  }
}
