import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.19
import Oxide.Ubuntu 1.0
import Oxide.testsupport 1.0
import Ubuntu.Components 1.3

Item {
  id: root

  width: 800
  height: 600

  UbuntuTestWebView {
    id: webView
    objectName: "webView"

    anchors.fill: parent

    property bool confirmCompleted: false
    property bool confirmResult

    function waitForConfirmToComplete() {
      return TestUtils.waitFor(function() { return confirmCompleted; });
    }

    messageHandlers: [
      ScriptMessageHandler {
        msgId: "confirm-response"
        contexts: [ "oxide://dialogtest/" ]
        callback: function(msg) {
          webView.confirmCompleted = true;
          webView.confirmResult = msg.payload;
        }
      }
    ]
  }

  UbuntuTestWebView {
    id: dummyWebView
    // See https://launchpad.net/bugs/1661405
    url: "about:blank"

    anchors.fill: parent
  }

  Component.onCompleted: {
    SingletonTestWebContext.addTestUserScript({
        context: "oxide://dialogtest/",
        url: Qt.resolvedUrl("tst_WebViewConfirmDialog.js")
    });
  }

  Component {
    id: webViewFactory
    UbuntuTestWebView {}
  }

  TestCase {
    id: test
    name: "WebViewConfirmDialog"
    when: windowShown

    function getConfirmDialog() {
      return TestSupport.findItemInScene(TestWindow.rootItem, "webView_JavaScriptDialog");
    }

    function waitForConfirmDialogToClose() {
      var dialog = getConfirmDialog();
      if (!dialog) {
        return true;
      }

      var helper = TestSupport.createQObjectTestHelper(dialog);

      return TestUtils.waitFor(function() { return !getConfirmDialog(); }) &&
             TestUtils.waitFor(function() { return helper.destroyed; });
    }

    function waitForConfirmDialog() {
      return TestUtils.waitFor(function() { return !!getConfirmDialog() && getConfirmDialog().visible; });
    }

    function getConfirmDialogElement(element) {
      return TestSupport.findItemInScene(getConfirmDialog(), "webView_JavaScriptDialog_" + element);
    }

    function cleanupTestCase() {
      SingletonTestWebContext.clearTestUserScripts();
    }

    function init() {
      webView.z = 0;
      dummyWebView.z = -1;
      dummyWebView.visible = false;
      webView.forceActiveFocus();
      webView.confirmCompleted = false;
      webView.clearLoadEventCounters();
    }

    function test_WebViewConfirmDialog1_confirm_foreground_data() {
      return [
        { button: "okButton", result: true },
        { button: "cancelButton", result: false }
      ];
    }

    function test_WebViewConfirmDialog1_confirm_foreground(data) {
      webView.url = "http://testsuite/tst_WebViewConfirmDialog.html";
      verify(webView.waitForLoadSucceeded());
      verify(waitForConfirmDialog());

      var dialog = getConfirmDialog();
      compare(dialog.type, "confirm");
      compare(dialog.text, "Bar Confirm");

      var okButton = getConfirmDialogElement("okButton");
      verify(okButton.visible);

      var cancelButton = getConfirmDialogElement("cancelButton");
      verify(cancelButton.visible);

      var input = getConfirmDialogElement("input");
      verify(!input.visible);

      mouseClick(getConfirmDialogElement(data.button));
      verify(webView.waitForConfirmToComplete());
      verify(waitForConfirmDialogToClose());
      compare(webView.confirmResult, data.result);
    }

    function test_WebViewConfirmDialog2_confirm_background() {
      dummyWebView.visible = true;
      webView.z = -1;
      dummyWebView.z = 0;
      dummyWebView.forceActiveFocus();

      webView.url = "http://testsuite/tst_WebViewConfirmDialog.html";
      verify(webView.waitForLoadSucceeded());
      verify(webView.waitForConfirmToComplete());
      verify(!webView.confirmResult);

      dummyWebView.z = -1;
      webView.z = 0;
      dummyWebView.visible = false;
      webView.forceActiveFocus();

      TestSupport.wait(100);
      verify(!getConfirmDialog());
    }

    function test_WebViewConfirmDialog3_auto_dismiss() {
      webView.url = "http://testsuite/tst_WebViewConfirmDialog.html";
      verify(webView.waitForLoadSucceeded());
      verify(waitForConfirmDialog());

      dummyWebView.visible = true;
      webView.z = -1;
      dummyWebView.z = 0;
      dummyWebView.forceActiveFocus();

      verify(webView.waitForConfirmToComplete());
      verify(waitForConfirmDialogToClose());
      verify(!webView.confirmResult);
    }

    function test_WebViewConfirmDialog4_destroy_on_webview_close() {
      var webView2 = webViewFactory.createObject(root, { "anchors.fill": parent, objectName: "webView" });
      webView.z = -1;
      webView2.z = 0;
      webView2.forceActiveFocus();
      webView2.url = "http://testsuite/tst_WebViewConfirmDialog.html";
      verify(webView2.waitForLoadSucceeded());
      verify(waitForConfirmDialog());

      var dialog = getConfirmDialog();
      var helper = TestSupport.createQObjectTestHelper(dialog);

      webView2.destroy();
      verify(TestUtils.waitFor(function() { return helper.destroyed; }));
    }
  }
}
