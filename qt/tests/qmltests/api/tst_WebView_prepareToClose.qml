import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.4
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "prepareToCloseResponse"
  }

  property bool proceedToClose: false

  onPrepareToCloseResponse: {
    proceedToClose = proceed;
  }

  property bool autoAllowUnload: false
  property var currentDialog: null

  beforeUnloadDialog: Item {
    readonly property string message: model.message

    function respond(accept) {
      if (accept) {
        model.accept();
      } else {
        model.reject();
      }
    }

    Component.onCompleted: {
      WebView.view.currentDialog = this;
      if (WebView.view.autoAllowUnload) {
        respond(true);
      }
    }
    Component.onDestruction: {
      WebView.view.currentDialog = null;
    }
  }

  TestCase {
    id: test
    name: "WebView_prepareToClose"
    when: windowShown

    function _waitForDialog() {
      verify(TestUtils.waitFor(function() { return webView.currentDialog != null; }));
    }

    function init() {
      webView.autoAllowUnload = true;
      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded());
      webView.autoAllowUnload = false;

      spy.clear();
      webView.proceedToClose = false;
    }

    // Test a page that has no beforeunload handler
    function test_WebView_prepareToClose1_no_handler() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.prepareToClose();
      spy.wait();

      verify(webView.proceedToClose);
    }

    // Test a page that has a beforeunload handler but doesn't return a value
    function test_WebView_prepareToClose2_handler() {
      webView.url = "http://testsuite/tst_WebView_prepareToClose1.html";
      verify(webView.waitForLoadSucceeded());

      webView.prepareToClose();
      spy.wait();

      verify(webView.proceedToClose);
    }

    // Load a page with a beforeunload handler that wants to block unloading,
    // and allow the unload
    function test_WebView_prepareToClose3_handler_blocking_allow() {
      webView.url = "http://testsuite/tst_WebView_prepareToClose2.html";
      verify(webView.waitForLoadSucceeded());

      webView.prepareToClose();
      _waitForDialog();

      compare(webView.currentDialog.message, "Foo");
      webView.currentDialog.respond(true);
      spy.wait();

      verify(webView.proceedToClose);
    }

    // Load a page with a beforeunload handler that wants to block unloading,
    // and deny the unload
    function test_WebView_prepareToClose4_handler_blocking_reject() {
      webView.url = "http://testsuite/tst_WebView_prepareToClose2.html";
      verify(webView.waitForLoadSucceeded());

      webView.prepareToClose();
      _waitForDialog();

      compare(webView.currentDialog.message, "Foo");
      webView.currentDialog.respond(false);
      spy.wait();

      verify(!webView.proceedToClose);
    }
  }
}
