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
        msg.reply({ out: msg.args.in * 2 });
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
    },
    MessageHandler {
      msgId: "TEST-THROW"
      worldIds: [ "TestUtils" ]
      callback: function(msg, frame) {
        webView.lastMessageFrameSource = frame;
        webView.lastMessageWorldId = msg.worldId;
        throw Error("This is an exception");
      }
    },
    MessageHandler {
      msgId: "TEST-WRONG-WORLD"
      worldIds: [ "Yaaaaaaaaa" ]
      callback: function(msg, frame) {
        msg.reply({});
      }
    }
  ]

  TestCase {
    id: test
    name: "WebView_messageHandlers"
    when: windowShown

    function init() {
      webView.lastMessageFrameSource = null;
      webView.lastMessageWorldId = "";
    }

    function test_WebView_messageHandlers1_valid() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
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

    function test_WebView_messageHandlers2_valid_subframe() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.rootFrame.childFrames.length, 1,
              "Expected 1 subframe");

      var frame = webView.rootFrame.childFrames[0];

      compare(webView.getTestApiForFrame(frame).sendMessageToSelf(
                  "TEST-REPLY", { in: 10 }).out, 20,
              "Invalid response from message handler");
      compare(webView.lastMessageFrameSource, frame,
              "Invalid source frame for message");
      compare(webView.lastMessageWorldId, "TestUtils",
              "Invalid world ID for message");
    }

    function test_WebView_messageHandlers3_error() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      try {
        webView.getTestApi().sendMessageToSelf("TEST-ERROR", { in: 10 });
        verify(false, "Should have thrown");
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

    function test_WebView_messageHandlers4_throw() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      try {
        webView.getTestApi().sendMessageToSelf("TEST-THROW", { in: 10 });
        verify(false, "Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorUncaughtException,
                "Unexpected error type");
        compare(e.message, "Error: This is an exception", "Unexpected error message");
      }

      compare(webView.lastMessageFrameSource, webView.rootFrame,
              "Invalid source frame for message");
      compare(webView.lastMessageWorldId, "TestUtils",
              "Invalid world ID for message");
    }

    function test_WebView_messageHandlers5_noHandler() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      try {
        webView.getTestApi().sendMessageToSelf("TEST-NO-HANDLER", { in: 10 });
        verify(false, "Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorNoHandler,
                "Unexpected error type");
      }
    }

    function test_WebView_messageHandlers6_noWorldMatch() {
      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      try {
        webView.getTestApi().sendMessageToSelf("TEST-WRONG-WORLD", { in: 10 });
        verify(false, "Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorNoHandler,
                "Unexpected error type");
      }
    }
  }
}
