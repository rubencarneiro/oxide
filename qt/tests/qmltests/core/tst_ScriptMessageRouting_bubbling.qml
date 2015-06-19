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
        msg.reply({ out: msg.payload, handler: "view" });
      }
    }
  ]

  TestCase {
    id: test
    name: "ScriptMessageRouting_direct"
    when: windowShown

    function init() {
      webView.url = "http://testsuite/tst_ScriptMessageRouting_bubbling.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
    }

    function test_ScriptMessageRouting_bubbling1_to_parent_frame() {
      var handler = messageHandler.createObject(
          webView.rootFrame,
          { msgId: "TEST", contexts: [ "oxide://testutils/" ],
            callback: function(msg) {
              msg.reply({ out: msg.payload, handler: "frame" });
            }
          });

      var frame = webView.rootFrame.childFrames[0];

      var res = webView.getTestApiForFrame(frame).sendMessageToSelf("TEST", 10);
      compare(res.handler, "frame", "Got response from wrong handler");
      compare(res.out, 10, "Invalid result");

      webView.rootFrame.removeMessageHandler(handler);
    }

    function test_ScriptMessageRouting_bubbling2_to_webview() {
      var frame = webView.rootFrame.childFrames[0];

      var res = webView.getTestApiForFrame(frame).sendMessageToSelf("TEST", 10);
      compare(res.handler, "view", "Got response from wrong handler");
      compare(res.out, 10, "Invalid result");
    }
  }
}
