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

  Component.onCompleted: {
    ScriptMessageTestUtils.init(webView.context);
  }

  // These tests verify that errors when sending messages to the browser are
  // handled correctly
  TestCase {
    id: test
    name: "ScriptMessageRoutingToBrowser_errors"
    when: windowShown

    function cleanupTestCase() {
      webView.context.clearTestUserScripts();
    }

    function init() {
      while (webView.rootFrame.messageHandlers.length > 0) {
        webView.rootFrame.removeMessageHandler(webView.rootFrame.messageHandlers[0]);
      }
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
    }

    function test_ScriptMessageRoutingToBrowser_errors1_no_match_data() {
      return [
        { handler: { msgId: "TEST", callback: function(msg) { msg.reply(5); } }, expectError: false },
        { handler: { msgId: "TEST", contexts: [ "http://foo/" ], callback: function(msg) { msg.reply({}); } }, expectError: true },
        { handler: { msgId: "FOO", callback: function(msg) { msg.reply({}); } }, expectError: true },
        { handler: { msgId: "TEST", callback: null }, expectError: true }
      ];
    }

    function test_ScriptMessageRoutingToBrowser_errors1_no_match(data) {
      var handler = messageHandler.createObject(webView.rootFrame, data.handler);

      var res = new ScriptMessageTestUtils.FrameHelper(webView.rootFrame).sendMessageToBrowser("TEST");
      if (data.expectError) {
        verify(res instanceof TestUtils.MessageError, "Invalid result type");
        compare(res.error, ScriptMessageRequest.ErrorNoHandler,
                "Unexpected error type");
      } else {
        compare(res, 5);
      }
    }

    function test_ScriptMessageRoutingToBrowser_errors2_handler_throws() {
      var handler = messageHandler.createObject(
          webView.rootFrame,
          { msgId: "TEST",
            callback: function(msg) { throw Error("This is an exception"); } });

      var res = new ScriptMessageTestUtils.FrameHelper(webView.rootFrame).sendMessageToBrowser("TEST");
      verify(res instanceof TestUtils.MessageError, "Invalid result type");
      compare(res.error, ScriptMessageRequest.ErrorUncaughtException);
      compare(res.message, "Error: This is an exception");
    }

    function test_ScriptMessageRoutingToBrowser_errors3_no_response() {
      var handler = messageHandler.createObject(
          webView.rootFrame,
          { msgId: "TEST",
            callback: function(msg) {} });

      var res = new ScriptMessageTestUtils.FrameHelper(webView.rootFrame).sendMessageToBrowser("TEST", null, true);
      verify(res instanceof TestUtils.MessageError, "Invalid result type");
      compare(res.error, ScriptMessageRequest.ErrorHandlerDidNotRespond,
              "Unexpected error type");
    }
  }
}
