import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  Component {
    id: messageHandler
    ScriptMessageHandler {}
  }

  messageHandlers: [
    ScriptMessageHandler {
      msgId: "TEST"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg) {
        msg.error("Message caught by WebView handler");
      }
    }
  ]

  TestCase {
    id: test
    name: "ScriptMessageRouting_direct"
    when: windowShown

    function init() {
      webView.url = "http://testsuite/tst_ScriptMessageRouting_direct.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
    }

    function _test_direct(frame) {
      var handler = messageHandler.createObject(
          frame,
          { msgId: "TEST", contexts: [ "oxide://testutils/" ],
            callback: function(msg) {
              msg.reply(msg.payload);
            }
          });

      var res = webView.getTestApiForFrame(frame).sendMessageToSelf("TEST", 10);
      compare(res, 10, "Unexpected return value from handler");
      frame.removeMessageHandler(handler);
    }

    function test_ScriptMessageRouting_direct1_rootFrame() {
      _test_direct(webView.rootFrame);
    }

    function test_ScriptMessageRouting_direct2_childFrame() {
      _test_direct(webView.rootFrame.childFrames[0]);
    }
  }
}
