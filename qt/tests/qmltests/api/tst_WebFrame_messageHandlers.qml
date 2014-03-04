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
    ScriptMessageHandler {}
  ]

  Component {
    id: messageHandler
    ScriptMessageHandler {}
  }

  SignalSpy {
    id: spy
    target: webView.rootFrame
    signalName: "messageHandlersChanged"
  }

  TestCase {
    id: test
    name: "WebFrame_messageHandlers"
    when: windowShown

    function init() {
      spy.clear();
    }

    function test_WebFrame_messageHandlers1_add_unowned() {
      var frame = webView.rootFrame;

      var handler = messageHandler.createObject(null, {});
      frame.addMessageHandler(handler);

      compare(spy.count, 1, "Should have had a messageHandlersChanged signal");
      compare(frame.messageHandlers.length, 1, "Should have one handler now");
      compare(frame.messageHandlers[0], handler,
              "Got the wrong handler back");
      compare(OxideTestingUtils.qObjectParent(handler), frame,
              "Message handler should be owned by the frame");

      handler = null;
      gc();
      compare(frame.messageHandlers.length, 1, "Should still have one handler");

      handler = frame.messageHandlers[0];
      frame.removeMessageHandler(handler);
      compare(spy.count, 2, "Should have had a messageHandlersChanged signal");
      compare(frame.messageHandlers.length, 0, "Should have no handlers now");
    }

    function test_WebFrame_messageHandlers2_create_with_parent_frame() {
      var frame = webView.rootFrame

      var handler = messageHandler.createObject(frame, {});
      compare(spy.count, 1, "Should have had a messageHandlersChanged signal");
      compare(frame.messageHandlers.length, 1, "Should have one handler now");
      compare(frame.messageHandlers[0], handler,
              "Got the wrong handler back");
      compare(OxideTestingUtils.qObjectParent(handler), frame,
              "Message handler should be owned by the frame");

      handler = null;
      gc();
      compare(frame.messageHandlers.length, 1, "Should still have one handler");

      handler = frame.messageHandlers[0];
      frame.removeMessageHandler(handler);
      compare(spy.count, 2, "Should have had a messageHandlersChanged signal");
      compare(frame.messageHandlers.length, 0, "Should have no handlers now");
    }

    function test_WebFrame_messageHandlers3_add_already_owned() {
      webView.url = "http://localhost:8080/tst_WebFrame_messageHandlers.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var frame = webView.rootFrame.childFrames[0];

      var handler = messageHandler.createObject(webView.rootFrame, {});
      compare(spy.count, 1, "Should have had a signal");
      compare(webView.rootFrame.messageHandlers.length, 1,
              "Should have 1 handler on the root frame");
      compare(frame.messageHandlers.length, 0,
              "Should have no handlers on the child frame");

      frame.addMessageHandler(handler);
      compare(spy.count, 2, "Should have had a signal");
      compare(webView.rootFrame.messageHandlers.length, 0,
              "Should have no handlers on the root frame");
      compare(frame.messageHandlers.length, 1,
              "Should have 1 handler on the child frame");
      compare(OxideTestingUtils.qObjectParent(handler), frame,
              "Incorrect parent");

      frame.removeMessageHandler(handler);
    }

    function test_WebFrame_messageHandlers4_add_already_owned_by_view() {
      var frame = webView.rootFrame;
      spy.target = webView;

      var handler = webView.messageHandlers[0];
      frame.addMessageHandler(handler);
      compare(spy.count, 1, "Should have had a signal");
      compare(webView.messageHandlers.length, 0,
              "WebView should have no handlers now");
      compare(frame.messageHandlers.length, 1,
              "Frame should have adopted message handler");
      compare(OxideTestingUtils.qObjectParent(handler), frame,
              "Incorrect parent");
    }
  }
}
