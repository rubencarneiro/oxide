import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "messageHandlersChanged"
  }

  Component {
    id: messageHandler
    ScriptMessageHandler {}
  }

  Component {
    id: view
    WebView {}
  }

  TestCase {
    id: test
    name: "WebView_messageHandlers"
    when: windowShown

    function init() {
      spy.clear();
    }

    function test_WebView_messageHandlers1_add_unowned() {
      var handler = messageHandler.createObject(null, {});
      webView.addMessageHandler(handler);

      compare(spy.count, 1, "Should have had a messageHandlersChanged signal");
      compare(webView.messageHandlers.length, 1, "Should have one handler now");
      compare(webView.messageHandlers[0], handler,
              "Got the wrong handler back");
      compare(Utils.qObjectParent(handler), webView,
              "Message handler should be owned by the view");

      handler = null;
      gc();
      compare(webView.messageHandlers.length, 1, "Should still have one handler");

      handler = webView.messageHandlers[0];
      webView.removeMessageHandler(handler);
      compare(spy.count, 2, "Should have had a messageHandlersChanged signal");
      compare(webView.messageHandlers.length, 0, "Should have no handlers now");
    }

    function test_WebView_messageHandlers2_create_with_parent_view() {
      var handler = messageHandler.createObject(webView, {});
      compare(spy.count, 1, "Should have had a messageHandlersChanged signal");
      compare(webView.messageHandlers.length, 1, "Should have one handler now");
      compare(webView.messageHandlers[0], handler,
              "Got the wrong handler back");
      compare(Utils.qObjectParent(handler), webView,
              "Message handler should be owned by the view");

      handler = null;
      gc();
      compare(webView.messageHandlers.length, 1, "Should still have one handler");

      handler = webView.messageHandlers[0];
      webView.removeMessageHandler(handler);
      compare(spy.count, 2, "Should have had a messageHandlersChanged signal");
      compare(webView.messageHandlers.length, 0, "Should have no handlers now");
    }

    function test_WebView_messageHandlers3_add_already_owned() {
      var otherView = view.createObject(null, {});
      var handler = messageHandler.createObject(otherView, {});

      webView.addMessageHandler(handler);
      compare(spy.count, 0, "Should not have had a signal");
      compare(otherView.messageHandlers.length, 1,
              "Should still have a handler on the other view");
      compare(webView.messageHandlers.length, 0,
              "Should have no handlers on our view");
      compare(Utils.qObjectParent(handler), otherView,
              "Incorrect parent");

      webView.removeMessageHandler(handler);
    }

    function test_WebView_messageHandlers4_add_already_owned_by_frame() {
      var frame = webView.rootFrame;
      var handler = messageHandler.createObject(frame, {});

      webView.addMessageHandler(handler);
      compare(spy.count, 0, "Should not have had a signal");
      compare(frame.messageHandlers.length, 1,
              "Frame should still have a handler");
      compare(webView.messageHandlers.length, 0,
              "View should not have adopted message handler");
      compare(Utils.qObjectParent(handler), frame,
              "Incorrect parent");
    }

    function test_WebView_messageHandlers5_remove_on_destroy() {
      var handler = messageHandler.createObject(webView, {});

      compare(spy.count, 1, "Should have had a signal");
      compare(webView.messageHandlers.length, 1,
              "WebView should have a handler");

      Utils.destroyQObjectNow(handler);

      compare(spy.count, 2, "Should have had a signal");
      compare(webView.messageHandlers.length, 0,
              "Expected no handlers in the WebView now");
    }
  }
}
