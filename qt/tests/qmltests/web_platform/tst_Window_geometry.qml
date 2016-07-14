import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Item {
  id: top
  focus: true

  TestWebView {
    id: webView

    property int resizeEvents: 0

    messageHandlers: [
      ScriptMessageHandler {
        msgId: "RESIZE-EVENT"
        contexts: [ "oxide://geomtest/" ]
        callback: function(msg) {
          ++webView.resizeEvents;
        }
      }
    ]

    Component.onCompleted: {
      SingletonTestWebContext.addTestUserScript({
          context: "oxide://geomtest/",
          url: Qt.resolvedUrl("tst_Window_geometry.js"),
          incognitoEnabled: true,
          matchAllFrames: true
      });
    }
  }

  TestCase {
    name: "Window_geometry"
    when: windowShown

    function init() {
      webView.resizeEvents = 0;
    }

    function cleanupTestCase() {
      SingletonTestWebContext.clearTestUserScripts();
    }

    // Verify that Window.inner{Height,Width} work correctly
    function test_Window_geometry1_inner() {
      webView.x = 0;
      webView.y = 0;
      webView.width = top.width;
      webView.height = top.height;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      // This will fail on Unity 8 with a device scale other than 1
      compare(webView.getTestApi().evaluateCode("window.innerWidth"),
              webView.width);
      compare(webView.getTestApi().evaluateCode("window.innerHeight"),
              webView.height);

      webView.resizeEvents = 0;

      webView.width = top.width / 2;
      TestUtils.waitFor(function() {return webView.resizeEvents == 1; });

      compare(webView.getTestApi().evaluateCode("window.innerWidth"),
              webView.width);
      compare(webView.getTestApi().evaluateCode("window.innerHeight"),
              webView.height);

      webView.height = top.height / 2;
      TestUtils.waitFor(function() {return webView.resizeEvents == 2; });

      compare(webView.getTestApi().evaluateCode("window.innerWidth"),
              webView.width);
      compare(webView.getTestApi().evaluateCode("window.innerHeight"),
              webView.height);
    }

    // Verify that Window.outer{Height,Width} work as intended
    // XXX: Note, this behaviour is currently incorrect - outer{Height,Width}
    //  should reflect the top-level window dimensions
    function test_Window_geometry2_outer() {
      webView.x = 0;
      webView.y = 0;
      webView.width = top.width;
      webView.height = top.height;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      // This will fail on Unity 8 with a device scale other than 1
      compare(webView.getTestApi().evaluateCode("window.outerWidth"),
              webView.width);
      compare(webView.getTestApi().evaluateCode("window.outerHeight"),
              webView.height);

      webView.resizeEvents = 0;

      webView.width = top.width / 2;
      TestUtils.waitFor(function() {return webView.resizeEvents == 1; });

      compare(webView.getTestApi().evaluateCode("window.outerWidth"),
              webView.width);
      compare(webView.getTestApi().evaluateCode("window.outerHeight"),
              webView.height);

      webView.height = top.height / 2;
      TestUtils.waitFor(function() {return webView.resizeEvents == 2; });

      compare(webView.getTestApi().evaluateCode("window.outerWidth"),
              webView.width);
      compare(webView.getTestApi().evaluateCode("window.outerHeight"),
              webView.height);
    }

    // TODO: Add test for Window.screen{X,y}
  }
}
