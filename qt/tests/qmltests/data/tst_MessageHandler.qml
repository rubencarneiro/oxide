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
      msgId: "TEST-THROW"
      worldIds: [ "TestUtils" ]
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
    name: "MessageHandler"
    when: windowShown

    function test_MessageHandler1_throw() {
      webView.url = "http://localhost:8080/tst_MessageHandler.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      try {
        webView.getTestApi().sendMessageToSelf("TEST-THROW", {});
        fail("Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorUncaughtException,
                "Unexpected error type");
        compare(e.message, "Error: This is an exception", "Unexpected error message");
      }
    }

    function test_MessageHandler2_msgId() {
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

    function test_MessageHandler3_worldIds() {
      spy.signalName = "worldIdsChanged";
      spy.clear();

      var orig = webView.messageHandlers[0].worldIds;

      webView.messageHandlers[0].worldIds = [ "Foo", "Bar" ];
      compare(spy.count, 1, "Should have had a signal");
      compare(webView.messageHandlers[0].worldIds.length, 2,
              "Should have 2 IDs");
      compare(webView.messageHandlers[0].worldIds[0], "Foo",
              "Unexpected ID");
      compare(webView.messageHandlers[0].worldIds[1], "Bar",
              "Unexpected ID");

      webView.messageHandlers[0].worldIds = orig;
    }

    function test_MessageHandler4_callback() {
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
