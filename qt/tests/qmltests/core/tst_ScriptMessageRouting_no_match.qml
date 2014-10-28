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

  TestCase {
    id: test
    name: "ScriptMessageRouting_direct"
    when: windowShown

    function init() {
      while (webView.rootFrame.messageHandlers.length > 0) {
        webView.rootFrame.removeMessageHandler(webView.rootFrame.messageHandlers[0]);
      }
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");
    }

    function test_ScriptMessageRouting_no_match_data() {
      return [
        { handler: { msgId: "TEST", contexts: [ "oxide://testutils/" ], callback: function(msg) { msg.reply({}); } }, expectError: false },
        { handler: { msgId: "TEST", contexts: [ "http://foo/" ], callback: function(msg) { msg.reply({}); } }, expectError: true },
        { handler: { msgId: "FOO", contexts: [ "oxide://testutils/" ], callback: function(msg) { msg.reply({}); } }, expectError: true },
        { handler: { msgId: "TEST", contexts: [ "oxide://testutils/" ], callback: null }, expectError: true }
      ];
    }

    function test_ScriptMessageRouting_no_match(data) {
      var handler = messageHandler.createObject(webView.rootFrame, data.handler);

      try {
        webView.getTestApi().sendMessageToSelf("TEST", {});
        verify(!data.expectError, "Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, ScriptMessageRequest.ErrorNoHandler,
                "Unexpected error type");
      }
    }
  }
}
