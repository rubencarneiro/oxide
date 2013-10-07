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
    MessageHandler {}
  }

  TestCase {
    id: test
    name: "WebView_messageHandlers_dynamic"
    when: windowShown

    function test_WebView_messageHandlers_dynamic1() {
      compare(webView.messageHandlers.length, 0,
              "Should be no handlers yet");

      var handler = messageHandler.createObject(
          null,
          { msgId: "TEST", worldIds: [ "TestUtils" ],
            callback: function(msg, frame) {
              msg.reply({ out: msg.args.in * 2 });
            }
          });
      webView.addMessageHandler(handler);
      compare(webView.messageHandlers.length, 1, "Should have a handler now");
      compare(webView.messageHandlers[0], handler,
              "Got the wrong handler back");

      handler = null;
      gc();

      compare(webView.messageHandlers.length, 1,
              "Should still have one handler");

      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().sendMessageToSelf(
                  "TEST", { in: 10 }).out, 20,
              "Invalid response from message handler");

      handler = webView.messageHandlers[0];
      webView.removeMessageHandler(handler);
      compare(webView.messageHandlers.length, 0,
              "Should have no handlers again");

      try {
        webView.getTestApi().sendMessageToSelf("TEST", { in: 10 });
        verify(false, "Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorNoHandler,
                "Unexpected error type");
      }
    }

    function test_WebView_messageHandlers_dynamic2() {
      compare(webView.messageHandlers.length, 0,
              "Should be no handlers yet");

      var handler = messageHandler.createObject(
          webView,
          { msgId: "TEST", worldIds: [ "TestUtils" ],
            callback: function(msg, frame) {
              msg.reply({ out: msg.args.in * 2 });
            }
          });
      compare(webView.messageHandlers.length, 1, "Should have a handler now");
      compare(webView.messageHandlers[0], handler,
              "Got the wrong handler back");

      handler = null;
      gc();

      compare(webView.messageHandlers.length, 1,
              "Should still have one handler");

      webView.url = "http://localhost:8080/tst_WebView_messageHandlers1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().sendMessageToSelf(
                  "TEST", { in: 10 }).out, 20,
              "Invalid response from message handler");

      handler = webView.messageHandlers[0];
      webView.removeMessageHandler(handler);

      compare(webView.messageHandlers.length, 0,
              "Should have no handlers again");

      try {
        webView.getTestApi().sendMessageToSelf("TEST", { in: 10 });
        verify(false, "Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorNoHandler,
                "Unexpected error type");
      }
    }
  }
}
