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

    property bool alertCompleted: false

    function waitForAlertToComplete() {
      return TestUtils.waitFor(function() { return alertCompleted; });
    }

    messageHandlers: [
      ScriptMessageHandler {
        msgId: "alert-response"
        contexts: [ "oxide://dialogtest/" ]
        callback: function(msg) {
          webView.alertCompleted = true;
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
        url: Qt.resolvedUrl("tst_WebViewAlertDialog.js")
    });
  }

  Component {
    id: webViewFactory
    UbuntuTestWebView {}
  }

  TestCase {
    id: test
    name: "WebViewAlertDialog"
    when: windowShown

    function getAlertDialog() {
      return TestSupport.findItemInScene(TestWindow.rootItem, "webView_JavaScriptDialog");
    }

    function waitForAlertDialogToClose() {
      var dialog = getAlertDialog();
      if (!dialog) {
        return true;
      }

      var helper = TestSupport.createQObjectTestHelper(dialog);

      return TestUtils.waitFor(function() { return !getAlertDialog(); }) &&
             TestUtils.waitFor(function() { return helper.destroyed; });
    }

    function waitForAlertDialog() {
      return TestUtils.waitFor(function() { return !!getAlertDialog() && getAlertDialog().visible; });
    }

    function getAlertDialogElement(element) {
      return TestSupport.findItemInScene(getAlertDialog(), "webView_JavaScriptDialog_" + element);
    }

    function cleanupTestCase() {
      SingletonTestWebContext.clearTestUserScripts();
    }

    function init() {
      webView.z = 0;
      dummyWebView.z = -1;
      dummyWebView.visible = false;
      webView.forceActiveFocus();
      webView.alertCompleted = false;
      webView.clearLoadEventCounters();
    }

    function test_WebViewAlertDialog1_alert_foreground() {
      webView.url = "http://testsuite/tst_WebViewAlertDialog.html";
      verify(webView.waitForLoadSucceeded());
      verify(waitForAlertDialog());

      var dialog = getAlertDialog();
      compare(dialog.type, "alert");
      compare(dialog.text, "Foo Alert!");

      var okButton = getAlertDialogElement("okButton");
      verify(okButton.visible);

      var cancelButton = getAlertDialogElement("cancelButton");
      verify(!cancelButton.visible);

      var input = getAlertDialogElement("input");
      verify(!input.visible);

      mouseClick(okButton);
      verify(webView.waitForAlertToComplete());
      verify(waitForAlertDialogToClose());
    }

    function test_WebViewAlertDialog2_alert_background() {
      dummyWebView.visible = true;
      webView.z = -1;
      dummyWebView.z = 0;
      dummyWebView.forceActiveFocus();

      webView.url = "http://testsuite/tst_WebViewAlertDialog.html";
      verify(webView.waitForLoadSucceeded());
      verify(webView.waitForAlertToComplete());

      dummyWebView.z = -1;
      webView.z = 0;
      dummyWebView.visible = false;
      webView.forceActiveFocus();

      verify(waitForAlertDialog());

      var dialog = getAlertDialog();
      compare(dialog.type, "alert");
      compare(dialog.text, "Foo Alert!");

      var okButton = getAlertDialogElement("okButton");
      verify(okButton.visible);

      var cancelButton = getAlertDialogElement("cancelButton");
      verify(!cancelButton.visible);

      var input = getAlertDialogElement("input");
      verify(!input.visible);

      mouseClick(okButton);
      verify(waitForAlertDialogToClose());
    }

    function test_WebViewAlertDialog3_auto_dismiss() {
      webView.url = "http://testsuite/tst_WebViewAlertDialog.html";
      verify(webView.waitForLoadSucceeded());
      verify(waitForAlertDialog());

      dummyWebView.visible = true;
      webView.z = -1;
      dummyWebView.z = 0;
      dummyWebView.forceActiveFocus();

      verify(webView.waitForAlertToComplete());
      verify(waitForAlertDialogToClose());
    }

    function test_WebViewAlertDialog4_destroy_on_webview_close() {
      var webView2 = webViewFactory.createObject(root, { "anchors.fill": parent, objectName: "webView" });
      webView.z = -1;
      webView2.z = 0;
      webView2.forceActiveFocus();
      webView2.url = "http://testsuite/tst_WebViewAlertDialog.html";
      verify(webView2.waitForLoadSucceeded());
      verify(waitForAlertDialog());

      var dialog = getAlertDialog();
      var helper = TestSupport.createQObjectTestHelper(dialog);

      webView2.destroy();
      verify(TestUtils.waitFor(function() { return helper.destroyed; }));
    }
  }
}
