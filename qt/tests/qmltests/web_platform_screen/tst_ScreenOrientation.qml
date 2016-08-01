import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView

  property int orientationEvents: 0

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "ORIENTATION-EVENT"
      contexts: [ "oxide://screentest/" ]
      callback: function(msg) {
        ++webView.orientationEvents;
      }
    }
  ]

  Component.onCompleted: {
    SingletonTestWebContext.addTestUserScript({
        context: "oxide://screentest/",
        url: Qt.resolvedUrl("tst_ScreenOrientation.js"),
        incognitoEnabled: true,
        matchAllFrames: true
    });
  }

  TestCase {
    name: "ScreenOrientation"
    when: TestSupport.testLoaded

    function getMockQPAShim() {
      return TestSupport.getAppProperty("_oxide_mock_qpa_shim_api");
    }

    function init() {
      getMockQPAShim().resetScreens();
      TestWindow.screen = TestSupport.screens[0];
      webView.orientationEvents = 0;
    }

    function cleanupTestCase() {
      getMockQPAShim().resetScreens();
      SingletonTestWebContext.clearTestUserScripts();
    }

    function validateOrientation(msg) {
      var types = [ "portrait-primary",
                    "landscape-primary",
                    "portrait-secondary",
                    "landscape-secondary" ];

      // QML doesn't have Math.log2
      var ln2 = function(a) {
        return Math.log(a) / Math.log(2);
      };

      var angles = [ 0, 90, 180, 270 ];

      compare(webView.getTestApi().evaluateCode("window.screen.orientation.type"),
              types[ln2(TestWindow.screen.orientation)], msg);
      var rotation = ln2(TestWindow.screen.orientation) - ln2(TestWindow.screen.nativeOrientation) % 3;
      if (rotation < 0) {
        rotation += 4;
      }

      compare(webView.getTestApi().evaluateCode("window.screen.orientation.angle"),
              angles[rotation], msg);
    }

    // Test that the orientation is reported correctly
    function test_ScreenOrientation1_orientation() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      validateOrientation();
    }

    function test_ScreenOrientation2_events() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      [2, 4, 8, 1].forEach(function(i) {
        webView.orientationEvents = 0;
        getMockQPAShim().setScreenOrientation(TestWindow.screen, i);
        verify(TestUtils.waitFor(function() { return webView.orientationEvents > 0; }));
        validateOrientation("Primary screen with orientation " + i);
      });

      TestWindow.screen = TestSupport.screens[1];
      compare(TestWindow.screen, TestSupport.screens[1]);

      [1, 2, 4, 8].forEach(function(i) {
        webView.orientationEvents = 0;
        getMockQPAShim().setScreenOrientation(TestWindow.screen, i);
        verify(TestUtils.waitFor(function() { return webView.orientationEvents > 0; }));
        validateOrientation("Second screen with orientation " + i);
      });
    }

    function test_ScreenOrientation3_switch_screens() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      validateOrientation("Before screen switch");

      TestWindow.screen = TestSupport.screens[1];
      compare(TestWindow.screen, TestSupport.screens[1]);

      verify(TestUtils.waitFor(function() { return webView.orientationEvents > 0; }));

      validateOrientation("After screen switch");
    }
  }
}
