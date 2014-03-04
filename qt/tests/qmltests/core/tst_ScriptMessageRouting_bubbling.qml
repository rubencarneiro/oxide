import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

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
        msg.reply({ out: msg.args.in, handler: "view" });
      }
    }
  ]

  TestCase {
    id: test
    name: "ScriptMessageRouting_direct"
    when: windowShown

    function init() {
      webView.url = "http://localhost:8080/tst_ScriptMessageRouting_bubbling.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
    }

    function test_ScriptMessageRouting_bubbling1_to_parent_frame() {
      var handler = messageHandler.createObject(
          webView.rootFrame,
          { msgId: "TEST", contexts: [ "oxide://testutils/" ],
            callback: function(msg) {
              msg.reply({ out: msg.args.in, handler: "frame" });
            }
          });

      var frame = webView.rootFrame.childFrames[0];

      var res = webView.getTestApiForFrame(frame).sendMessageToSelf("TEST", { in: 10 });
      compare(res.handler, "frame", "Got response from wrong handler");
      compare(res.out, 10, "Invalid result");

      webView.rootFrame.removeMessageHandler(handler);
    }

    function test_ScriptMessageRouting_bubbling2_to_webview() {
      var frame = webView.rootFrame.childFrames[0];

      var res = webView.getTestApiForFrame(frame).sendMessageToSelf("TEST", { in: 10 });
      compare(res.handler, "view", "Got response from wrong handler");
      compare(res.out, 10, "Invalid result");
    }
  }
}
