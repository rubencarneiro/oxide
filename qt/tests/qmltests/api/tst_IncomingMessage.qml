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
  property string lastMessageWorldId: ""

  messageHandlers: [
    MessageHandler {
      msgId: "TEST-REPLY"
      worldIds: [ "TestUtils" ]
      callback: function(msg, frame) {
        webView.lastMessageFrameSource = frame;
        webView.lastMessageWorldId = msg.worldId;
        msg.reply({ out: msg.payload.in * 2 });
      }
    },
    MessageHandler {
      msgId: "TEST-ERROR"
      worldIds: [ "TestUtils" ]
      callback: function(msg, frame) {
        webView.lastMessageFrameSource = frame;
        webView.lastMessageWorldId = msg.worldId;
        msg.error("This is an error");
      }
    }
  ]

  TestCase {
    id: test
    name: "IncomingMessage"
    when: windowShown

    function init() {
      webView.lastMessageFrameSource = null;
      webView.lastMessageWorldId = "";
    }

    function test_IncomingMessage1_reply() {
      webView.url = "http://localhost:8080/tst_IncomingMessage.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().sendMessageToSelf(
                  "TEST-REPLY", { in: 10 }).out, 20,
              "Invalid response from message handler");
      compare(webView.lastMessageFrameSource, webView.rootFrame,
              "Invalid source frame for message");
      compare(webView.lastMessageWorldId, "TestUtils",
              "Invalid world ID for message");
    }

    function test_IncomingMessage1_error() {
      webView.url = "http://localhost:8080/tst_IncomingMessage.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      try {
        webView.getTestApi().sendMessageToSelf("TEST-ERROR", { in: 10 });
        fail("Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorHandlerReportedError,
                "Unexpected error type");
        compare(e.message, "This is an error", "Unexpected error message");
      }

      compare(webView.lastMessageFrameSource, webView.rootFrame,
              "Invalid source frame for message");
      compare(webView.lastMessageWorldId, "TestUtils",
              "Invalid world ID for message");
    }
  }
}
