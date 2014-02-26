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
    ScriptMessageHandler {
      msgId: "TEST"
      contexts: [ "oxide://testutils/" ]
      callback: function(msg, frame) {
        webView.lastMessageFrameSource = frame;
        msg.reply({ out: msg.args.in * 2 });
      }
    }
  ]

  Component {
    id: messageHandler
    ScriptMessageHandler {}
  }

  SignalSpy {
    id: spy
    signalName: "messageHandlersChanged"
  }

  TestCase {
    id: test
    name: "WebFrame_messageHandlers"
    when: windowShown

    function init() {
      webView.lastMessageFrameSource = null;
      spy.clear();
    }

    function test_WebFrame_messageHandlers1() {
      webView.url = "http://localhost:8080/tst_WebFrame_messageHandlers.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      var frame = webView.rootFrame.childFrames[0];
      spy.target = frame;

      var handler = messageHandler.createObject(
          null,
          { msgId: "TEST", contexts: [ "http://foo/", "oxide://testutils/" ],
            callback: function(msg, frame) {
              webView.lastMessageFrameSource = frame;
              msg.reply({ out: msg.args.in * 5 });
            }
          });

      frame.addMessageHandler(handler);
      compare(spy.count, 1, "Should have had a messageHandlersChanged signal");
      compare(frame.messageHandlers.length, 1, "Should have one handler now");
      compare(frame.messageHandlers[0], handler,
              "Got the wrong handler back");

      function sendMessage() {
        return webView.getTestApiForFrame(frame).sendMessageToSelf(
            "TEST", { in: 10 }).out;
      }

      compare(sendMessage(), 50,
              "Should have had a response from the frame handler");
      compare(webView.lastMessageFrameSource, frame, "Invalid frame");

      handler = null;
      gc();
      compare(frame.messageHandlers.length, 1, "Should still have one handler");

      handler = frame.messageHandlers[0];
      frame.removeMessageHandler(handler);
      compare(spy.count, 2, "Should have had a messageHandlersChanged signal");
      compare(frame.messageHandlers.length, 0, "Should have no handlers now");

      compare(sendMessage(), 20,
              "Should have had a response from the webview handler");
      compare(webView.lastMessageFrameSource, frame, "Invalid frame");
    }

    function test_WebFrame_messageHandlers2() {
      webView.url = "http://localhost:8080/tst_WebFrame_messageHandlers.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      var frame = webView.rootFrame.childFrames[0];
      spy.target = frame;

      var handler = messageHandler.createObject(
          frame,
          { msgId: "TEST", contexts: [ "http://foo/", "oxide://testutils/" ],
            callback: function(msg, frame) {
              msg.reply({ out: msg.args.in * 5 });
            }
          });

      compare(spy.count, 1, "Should have had a messageHandlersChanged signal");
      compare(frame.messageHandlers.length, 1, "Should have one handler now");
      compare(frame.messageHandlers[0], handler,
              "Got the wrong handler back");

      function sendMessage() {
        return webView.getTestApiForFrame(frame).sendMessageToSelf(
            "TEST", { in: 10 }).out;
      }

      compare(sendMessage(), 50,
              "Should have had a response from the frame handler");

      handler = null;
      gc();
      compare(frame.messageHandlers.length, 1, "Should still have one handler");

      handler = frame.messageHandlers[0];
      frame.removeMessageHandler(handler);
      compare(spy.count, 2, "Should have had a messageHandlersChanged signal");
      compare(frame.messageHandlers.length, 0, "Should have no handlers now");

      compare(sendMessage(), 20,
              "Should have had a response from the webview handler");
    }

    function test_WebFrame_messageHandlers3_noMatchingId() {
      webView.url = "http://localhost:8080/tst_WebFrame_messageHandlers.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      var frame = webView.rootFrame.childFrames[0];

      var handler = messageHandler.createObject(
          null,
          { msgId: "TEST", contexts: [ "oxide://testutils/" ],
            callback: function(msg, frame) {
              msg.reply({ out: msg.args.in * 5 });
            }
          });

      frame.addMessageHandler(handler);
      compare(frame.messageHandlers.length, 1, "Should have one handler now");

      function sendMessage() {
        return webView.getTestApiForFrame(frame).sendMessageToSelf(
            "TEST", { in: 10 }).out;
      }

      compare(sendMessage(), 50,
              "Should have had a response from the frame handler");

      frame.messageHandlers[0].msgId = "TEST2";
      compare(sendMessage(), 20,
              "Should have had a response from the webview handler");
    }

    function test_WebFrame_messageHandlers4_noMatchingContext() {
      webView.url = "http://localhost:8080/tst_WebFrame_messageHandlers.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      var frame = webView.rootFrame.childFrames[0];

      var handler = messageHandler.createObject(
          null,
          { msgId: "TEST", contexts: [ "http://foo/", "oxide://testutils/" ],
            callback: function(msg, frame) {
              msg.reply({ out: msg.args.in * 5 });
            }
          });

      frame.addMessageHandler(handler);
      compare(frame.messageHandlers.length, 1, "Should have one handler now");

      function sendMessage() {
        return webView.getTestApiForFrame(frame).sendMessageToSelf(
            "TEST", { in: 10 }).out;
      }

      compare(sendMessage(), 50,
              "Should have had a response from the frame handler");

      frame.messageHandlers[0].contexts = [ "http://foo/" ];
      compare(sendMessage(), 20,
              "Should have had a response from the webview handler");

    }

    function test_WebFrame_messageHandlers5_noCallback() {
      webView.url = "http://localhost:8080/tst_WebFrame_messageHandlers.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      var frame = webView.rootFrame.childFrames[0];

      var handler = messageHandler.createObject(
          null,
          { msgId: "TEST", contexts: [ "http://foo/", "oxide://testutils/" ],
            callback: function(msg, frame) {
              msg.reply({ out: msg.args.in * 5 });
            }
          });

      frame.addMessageHandler(handler);
      compare(frame.messageHandlers.length, 1, "Should have one handler now");

      function sendMessage() {
        return webView.getTestApiForFrame(frame).sendMessageToSelf(
            "TEST", { in: 10 }).out;
      }
      
      compare(sendMessage(), 50,
              "Should have had a response from the frame handler");

      frame.messageHandlers[0].callback = null;
      compare(sendMessage(), 20,
              "Should have had a response from the webview handler");

    }
  }
}
