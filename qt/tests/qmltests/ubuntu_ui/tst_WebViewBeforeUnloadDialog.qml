import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.19
import Oxide.Ubuntu 1.0
import Oxide.testsupport 1.0
import Ubuntu.Components 1.3

UbuntuTestWebView {
  id: webView
  objectName: "webView"

  width: 800
  height: 600

  Component {
    id: webViewFactory
    UbuntuTestWebView {}
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "prepareToCloseResponse"
  }

  TestCase {
    id: test
    name: "WebViewBeforeUnloadDialog"
    when: windowShown

    function getDialog() {
      return TestSupport.findItemInScene(TestWindow.rootItem, "webView_BeforeUnloadDialog");
    }

    function waitForDialogToClose() {
      var dialog = getDialog();
      if (!dialog) {
        return true;
      }

      var helper = TestSupport.createQObjectTestHelper(dialog);

      return TestUtils.waitFor(function() { return !getDialog(); }) &&
             TestUtils.waitFor(function() { return helper.destroyed; });
    }

    function waitForDialog() {
      return TestUtils.waitFor(function() { return !!getDialog() && getDialog().visible; });
    }

    function getDialogElement(element) {
      return TestSupport.findItemInScene(getDialog(), "webView_BeforeUnloadDialog_" + element);
    }

    function init() {
      spy.clear();
      webView.clearLoadEventCounters();
    }

    function test_WebViewBeforeUnloadDialog1_browser_initiated() {
      webView.url = "http://testsuite/tst_WebViewBeforeUnloadDialog.html";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/empty.html";
      verify(waitForDialog());

      mouseClick(getDialogElement("stayButton"));
      verify(waitForDialogToClose());
      tryCompare(webView, "url", "http://testsuite/tst_WebViewBeforeUnloadDialog.html");

      webView.url = "http://testsuite/empty.html";
      verify(waitForDialog());

      mouseClick(getDialogElement("leaveButton"));
      verify(waitForDialogToClose());
      verify(webView.waitForLoadSucceeded());
    }

    function test_WebViewBeforeUnloadDialog2_script_initiated() {
      webView.url = "http://testsuite/tst_WebViewBeforeUnloadDialog.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.location = \"empty.html\"", false);
      verify(webView.waitForLoadSucceeded());
    }

    function test_WebViewBeforeUnloadDialog3_gesture_initiated() {
      webView.url = "http://testsuite/tst_WebViewBeforeUnloadDialog.html";
      verify(webView.waitForLoadSucceeded());

      var r = webView.getTestApi().getBoundingClientRectForSelector("#link");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      verify(waitForDialog());

      mouseClick(getDialogElement("leaveButton"));
      verify(waitForDialogToClose());
      verify(webView.waitForLoadSucceeded());
    }

    function test_WebViewBeforeUnloadDialog4_api_initiated() {
      webView.url = "http://testsuite/tst_WebViewBeforeUnloadDialog.html";
      verify(webView.waitForLoadSucceeded());

      webView.prepareToClose();
      verify(waitForDialog());

      mouseClick(getDialogElement("stayButton"));
      verify(waitForDialogToClose());
      spy.wait();
      verify(!spy.signalArguments[0][0]);

      webView.prepareToClose();
      verify(waitForDialog());

      mouseClick(getDialogElement("leaveButton"));
      verify(waitForDialogToClose());
      spy.wait();
      verify(spy.signalArguments[1][0]);
      compare(spy.signalArguments.length, 2);
    }

    function test_WebViewBeforeUnloadDialog5_destroy_on_webview_close() {
      var webView2 = webViewFactory.createObject(webView, { "anchors.fill": parent, objectName: "webView" });
      webView2.url = "http://testsuite/tst_WebViewBeforeUnloadDialog.html";
      verify(webView2.waitForLoadSucceeded());

      webView2.url = "http://testsuite/empty.html";
      verify(waitForDialog());

      var dialog = getDialog();
      var helper = TestSupport.createQObjectTestHelper(dialog);

      webView2.destroy();
      verify(TestUtils.waitFor(function() { return helper.destroyed; }));
    }
  }
}
