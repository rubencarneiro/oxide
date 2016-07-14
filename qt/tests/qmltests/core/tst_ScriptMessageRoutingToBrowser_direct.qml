import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  Component {
    id: messageHandler
    ScriptMessageTestHandler {}
  }

  messageHandlers: [
    ScriptMessageTestHandler {
      msgId: "TEST"
      callback: function(msg) {
        msg.error("Message caught by WebView handler");
      }
    }
  ]

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  // Test that messages sent to the browser are delivered directly to frame
  // handlers
  TestCase {
    id: test
    name: "ScriptMessageRoutingToBrowser_direct"
    when: windowShown

    function init() {
      webView.url = "http://testsuite/tst_ScriptMessageRoutingToBrowser_direct.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
    }

    function cleanupTestCase() {
      webView.context.clearTestUserScripts();
    }

    function _test_direct(frame) {
      var handler = messageHandler.createObject(
          frame,
          { msgId: "TEST",
            callback: function(msg) {
              msg.reply(msg.payload);
            }
          });

      var res = new ScriptMessageTestUtils.FrameHelper(frame).sendMessageToBrowser("TEST", 10);
      compare(res, 10, "Unexpected return value from handler");
      frame.removeMessageHandler(handler);
    }

    function test_ScriptMessageRoutingToBrowser_direct1_childFrame() {
      _test_direct(webView.rootFrame.childFrames[0]);
    }

    function test_ScriptMessageRoutingToBrowser_direct2_rootFrame() {
      _test_direct(webView.rootFrame);
    }
  }
}
