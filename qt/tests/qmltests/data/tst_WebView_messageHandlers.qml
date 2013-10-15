import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  messageHandlers: [
    MessageHandler {
      msgId: "TEST"
      worldIds: [ "Foo", "TestUtils" ]
      callback: function(msg, frame) {
        msg.reply({ out: msg.args.in * 2 });
      }
    }
  ]

  TestCase {
    id: test
    name: "WebView_messageHandlers"
    when: windowShown

    function test_WebView_messageHandlers1_valid() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().sendMessageToSelf(
                  "TEST", { in: 10 }).out, 20,
              "Invalid response from message handler");
    }

    function test_WebView_messageHandlers2_valid_subframe() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.rootFrame.childFrames.length, 1,
              "Expected 1 subframe");

      var frame = webView.rootFrame.childFrames[0];

      compare(webView.getTestApiForFrame(frame).sendMessageToSelf(
                  "TEST", { in: 10 }).out, 20,
              "Invalid response from message handler");
    }

    function test_WebView_messageHandlers5_noMatchingMsgId() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      function sendMessage() {
        return webView.getTestApi().sendMessageToSelf("TEST", { in: 10 }).out;
      }

      compare(sendMessage(), 20, "Should have had a response");

      webView.messageHandlers[0].msgId = "TEST2";

      try {
        sendMessage();
        fail("Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorNoHandler,
                "Unexpected error type");
      }

      webView.messageHandlers[0].msgId = "TEST";
    }

    function test_WebView_messageHandlers6_noMatchingWorld() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      function sendMessage() {
        return webView.getTestApi().sendMessageToSelf("TEST", { in: 10 }).out;
      }

      compare(sendMessage(), 20, "Should have had a response");

      webView.messageHandlers[0].worldIds = [ "Yaaaaaa" ];

      try {
        sendMessage();
        fail("Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorNoHandler,
                "Unexpected error type");
      }

      webView.messageHandlers[0].worldIds = [ "Foo", "TestUtils" ];
    }
  }
}
