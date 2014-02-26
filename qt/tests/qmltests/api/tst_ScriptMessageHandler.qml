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
    ScriptMessageHandler {
      msgId: "TEST-THROW"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg, frame) {
        throw Error("This is an exception");
      }
    }
  ]

  SignalSpy {
    id: spy
    target: webView.messageHandlers[0]
  }

  TestCase {
    id: test
    name: "ScriptMessageHandler"
    when: windowShown

    function test_ScriptMessageHandler1_throw() {
      webView.url = "http://localhost:8080/tst_ScriptMessageHandler.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      try {
        webView.getTestApi().sendMessageToSelf("TEST-THROW", {});
        fail("Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, ScriptMessageRequest.ErrorUncaughtException,
                "Unexpected error type");
        compare(e.message, "Error: This is an exception", "Unexpected error message");
      }
    }

    function test_ScriptMessageHandler2_msgId() {
      spy.signalName = "msgIdChanged";
      spy.clear();

      var orig = webView.messageHandlers[0].msgId;

      webView.messageHandlers[0].msgId = orig;
      compare(spy.count, 0, "Should have had no signal");

      webView.messageHandlers[0].msgId = "FOO";
      compare(spy.count, 1, "Should have had a signal");
      compare(webView.messageHandlers[0].msgId, "FOO", "Unexpected msgId");

      webView.messageHandlers[0].msgId = orig;
    }

    function test_ScriptMessageHandler3_contexts() {
      spy.signalName = "contextsChanged";
      spy.clear();

      var orig = webView.messageHandlers[0].contexts;

      webView.messageHandlers[0].contexts = [ "http://foo/", "http://bar/" ];
      compare(spy.count, 1, "Should have had a signal");
      compare(webView.messageHandlers[0].contexts.length, 2,
              "Should have 2 contexts");
      compare(webView.messageHandlers[0].contexts[0], "http://foo/",
              "Unexpected context");
      compare(webView.messageHandlers[0].contexts[1], "http://bar/",
              "Unexpected context");

      webView.messageHandlers[0].contexts = orig;
    }

    function test_ScriptMessageHandler4_callback() {
      spy.signalName = "callbackChanged";
      spy.clear();

      var orig = webView.messageHandlers[0].callback;

      webView.messageHandlers[0].callback = "foo";
      compare(spy.count, 0, "Shouldn't have had a signal");
      compare(webView.messageHandlers[0].callback, orig, 
              "Callback shouldn't have changed");

      webView.messageHandlers[0].callback = null;
      compare(spy.count, 1, "Should have changed");
      compare(webView.messageHandlers[0].callback, null,
              "Callback should be empty");

      webView.messageHandlers[0].callback = orig;
      compare(spy.count, 2, "Should have had another signal");
      compare(webView.messageHandlers[0].callback, orig,
              "Should have the original callback again");
    }
  }
}
