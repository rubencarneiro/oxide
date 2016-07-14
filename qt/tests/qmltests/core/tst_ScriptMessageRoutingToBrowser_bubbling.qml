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
        msg.reply("view");
      }
    }
  ]

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  // These tests verify that messages sent to the browser bubble correctly to an
  // ancestor frame or webview message handler
  TestCase {
    id: test
    name: "ScriptMessageRoutingToBrowser_bubbling"
    when: windowShown

    function cleanupTestCase() {
      webView.context.clearTestUserScripts();
    }

    function init() {
      webView.url = "http://testsuite/tst_ScriptMessageRoutingToBrowser_bubbling.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
    }

    function test_ScriptMessageRouting_bubbling1_to_parent_frame() {
      var handler = messageHandler.createObject(
          webView.rootFrame,
          { msgId: "TEST",
            callback: function(msg) {
              msg.reply("frame");
            }
          });

      var frame = webView.rootFrame.childFrames[0];

      var res = new ScriptMessageTestUtils.FrameHelper(frame).sendMessageToBrowser("TEST");
      compare(res, "frame", "Got response from wrong handler");

      webView.rootFrame.removeMessageHandler(handler);
    }

    function test_ScriptMessageRouting_bubbling2_to_webview() {
      var frame = webView.rootFrame.childFrames[0];

      var res = new ScriptMessageTestUtils.FrameHelper(frame).sendMessageToBrowser("TEST");
      compare(res, "view", "Got response from wrong handler");
    }
  }
}
