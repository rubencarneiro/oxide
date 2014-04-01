import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

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
      spy.target = webView.rootFrame;
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
      compare(spy.count, 1, "Should have had a signal");
      compare(webView.rootFrame.messageHandlers.length, 1,
              "Should still have a handler on the root frame");
      compare(frame.messageHandlers.length, 0,
              "Should have no handlers on the child frame");
      compare(OxideTestingUtils.qObjectParent(handler), webView.rootFrame,
              "Incorrect parent");

      webView.rootFrame.removeMessageHandler(handler);
    }

    function test_WebFrame_messageHandlers4_add_already_owned_by_view() {
      var frame = webView.rootFrame;
      spy.target = webView;

      var handler = webView.messageHandlers[0];
      frame.addMessageHandler(handler);
      compare(spy.count, 0, "Should not have had a signal");
      compare(webView.messageHandlers.length, 1,
              "WebView should still have a handler");
      compare(frame.messageHandlers.length, 0,
              "Frame should not have adopted message handler");
      compare(OxideTestingUtils.qObjectParent(handler), webView,
              "Incorrect parent");
    }

    function test_WebFrame_messageHandlers5_remove_on_destroy() {
      var frame = webView.rootFrame;
      var handler = messageHandler.createObject(frame, {});

      compare(spy.count, 1, "Should have had a signal");
      compare(frame.messageHandlers.length, 1,
              "WebFrame should have a handler");

      var obs = OxideTestingUtils.createDestructionObserver(handler);
      handler.destroy();
      verify(webView.waitFor(function() { return obs.destroyed; }),
             "Timed out waiting for handler to be destroyed");

      compare(spy.count, 2, "Should have had a signal");
      compare(frame.messageHandlers.length, 0,
              "Expected no handlers on the WebFrame now");
    }
  }
}
