import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property QtObject lastMessageFrameSource: null

  messageHandlers: [
    MessageHandler {
      msgId: "TEST"
      callback: function(msg, frame) {
        webView.lastMessageFrameSource = frame;
        msg.reply({ out: msg.args.in * 2 });
      }
    }
  ]

  TestCase {
    id: test
    name: "WebView_messageHandlers1"
    when: windowShown

    function test_WebView_messageHandlers1_1() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1_1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().sendMessageToSelf("TEST", { in: 10 }).out, 20,
              "Invalid response from message handler");
      compare(webView.lastMessageFrameSource, webView.rootFrame,
              "Invalid source frame for message");
    }

    function test_WebView_messageHandlers1_2() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1_2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.rootFrame.childFrames.length, 1,
              "Expected 1 subframe");

      var frame = webView.rootFrame.childFrames[0];

      compare(webView.getTestApiForFrame(frame).sendMessageToSelf(
                  "TEST", { in: 10 }).out, 20,
              "Invalid response from message handler");
      compare(webView.lastMessageFrameSource, frame,
              "Invalid source frame for message");
    }
  }
}
