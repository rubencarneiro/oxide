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

  SignalSpy {
    id: spy
    target: webView
    signalName: "messageHandlersChanged"
  }

  TestCase {
    id: test
    name: "WebView_messageHandlers_dynamic"
    when: windowShown

    function init() {
      spy.clear();
    }

    function test_WebView_messageHandlers_dynamic1() {
      compare(webView.messageHandlers.length, 0,
              "Should be no handlers yet");

      var handler = messageHandler.createObject(
          null,
          { msgId: "TEST", worldIds: [ "TestUtils" ],
            callback: function(msg, frame) {
              msg.reply({ out: msg.payload.in * 2 });
            }
          });
      webView.addMessageHandler(handler);
      compare(spy.count, 1, "Should have had a messageHandlersChanged signal");
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

      function sendMessage() {
        return webView.getTestApi().sendMessageToSelf("TEST", { in: 10 }).out;
      }

      compare(sendMessage(), 20,
              "Invalid response from message handler");

      handler = webView.messageHandlers[0];
      webView.removeMessageHandler(handler);
      compare(spy.count, 2, "Should have had a messageHandlersChanged signal");
      compare(webView.messageHandlers.length, 0, "Should have no handlers again");

      try {
        sendMessage();
        fail("Should have thrown");
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
              msg.reply({ out: msg.payload.in * 2 });
            }
          });
      compare(spy.count, 1, "Should have had a messageHandlersChanged signal");
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

      function sendMessage() {
        return webView.getTestApi().sendMessageToSelf("TEST", { in: 10 }).out;
      }

      compare(sendMessage(), 20, "Invalid response from message handler");

      handler = webView.messageHandlers[0];
      webView.removeMessageHandler(handler);

      compare(spy.count, 2, "Should have had a messageHandlersChanged signal");
      compare(webView.messageHandlers.length, 0, "Should have no handlers again");

      try {
        sendMessage();
        fail("Should have thrown");
      } catch(e) {
        verify(e instanceof TestUtils.MessageError, "Invalid exception type");
        compare(e.error, OutgoingMessageRequest.ErrorNoHandler,
                "Unexpected error type");
      }
    }
  }
}
