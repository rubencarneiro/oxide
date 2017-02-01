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

    property bool promptCompleted: false
    property var promptResult

    function waitForPromptToComplete() {
      return TestUtils.waitFor(function() { return promptCompleted; });
    }

    messageHandlers: [
      ScriptMessageHandler {
        msgId: "prompt-response"
        contexts: [ "oxide://dialogtest/" ]
        callback: function(msg) {
          webView.promptCompleted = true;
          webView.promptResult = msg.payload;
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
        url: Qt.resolvedUrl("tst_WebViewPromptDialog.js")
    });
  }

  Component {
    id: webViewFactory
    UbuntuTestWebView {}
  }

  TestCase {
    id: test
    name: "WebViewPromptDialog"
    when: windowShown

    function getPromptDialog() {
      return TestSupport.findItemInScene(TestWindow.rootItem, "webView_JavaScriptDialog");
    }

    function waitForPromptDialogToClose() {
      var dialog = getPromptDialog();
      if (!dialog) {
        return true;
      }

      var helper = TestSupport.createQObjectTestHelper(dialog);

      return TestUtils.waitFor(function() { return !getPromptDialog(); }) &&
             TestUtils.waitFor(function() { return helper.destroyed; });
    }

    function waitForPromptDialog() {
      return TestUtils.waitFor(function() { return !!getPromptDialog() && getPromptDialog().visible; });
    }

    function getPromptDialogElement(element) {
      return TestSupport.findItemInScene(getPromptDialog(), "webView_JavaScriptDialog_" + element);
    }

    function cleanupTestCase() {
      SingletonTestWebContext.clearTestUserScripts();
    }

    function init() {
      webView.z = 0;
      dummyWebView.z = -1;
      dummyWebView.visible = false;
      webView.forceActiveFocus();
      webView.promptCompleted = false;
      webView.clearLoadEventCounters();
    }

    function test_WebViewPromptDialog1_prompt_foreground_data() {
      return [
        { input: [ Qt.Key_Delete, Qt.Key_Delete, Qt.Key_Delete, "B", "a", "r" ], button: "okButton", result: "Bar" },
        { input: [], button: "okButton", result: "Zzz" },
        { input: [ Qt.Key_Delete, Qt.Key_Delete, Qt.Key_Delete, "B", "a", "r" ], button: "cancelButton", result: undefined },
        { input: [ Qt.Key_Delete, Qt.Key_Delete, Qt.Key_Delete, "F", "o", "o", Qt.Key_Return ], button: null, result: "Foo" }
      ];
    }

    function test_WebViewPromptDialog1_prompt_foreground(data) {
      webView.url = "http://testsuite/tst_WebViewPromptDialog.html";
      verify(webView.waitForLoadSucceeded());
      verify(waitForPromptDialog());

      var dialog = getPromptDialog();
      compare(dialog.type, "prompt");
      compare(dialog.text, "Test Prompt");

      var okButton = getPromptDialogElement("okButton");
      verify(okButton.visible);

      var cancelButton = getPromptDialogElement("cancelButton");
      verify(cancelButton.visible);

      var input = getPromptDialogElement("input");
      verify(input.visible);
      compare(input.text, "Zzz");

      for (var i = 0; i < data.input.length; ++i) {
        keyPress(data.input[i]);
      }

      if (data.button) {
        mouseClick(getPromptDialogElement(data.button));
      }

      verify(webView.waitForPromptToComplete());
      verify(waitForPromptDialogToClose());
      compare(webView.promptResult, data.result);
    }

    function test_WebViewPromptDialog2_prompt_background() {
      dummyWebView.visible = true;
      webView.z = -1;
      dummyWebView.z = 0;
      dummyWebView.forceActiveFocus();

      webView.url = "http://testsuite/tst_WebViewPromptDialog.html";
      verify(webView.waitForLoadSucceeded());
      verify(webView.waitForPromptToComplete());
      compare(webView.promptResult, undefined);

      dummyWebView.z = -1;
      webView.z = 0;
      dummyWebView.visible = false;
      webView.forceActiveFocus();

      TestSupport.wait(100);
      verify(!getPromptDialog());
    }

    function test_WebViewPromptDialog3_auto_dismiss() {
      webView.url = "http://testsuite/tst_WebViewPromptDialog.html";
      verify(webView.waitForLoadSucceeded());
      verify(waitForPromptDialog());

      dummyWebView.visible = true;
      webView.z = -1;
      dummyWebView.z = 0;
      dummyWebView.forceActiveFocus();

      verify(webView.waitForPromptToComplete());
      verify(waitForPromptDialogToClose());
      compare(webView.promptResult, undefined);
    }

    function test_WebViewPromptDialog4_destroy_on_webview_close() {
      var webView2 = webViewFactory.createObject(root, { "anchors.fill": parent, objectName: "webView" });
      webView.z = -1;
      webView2.z = 0;
      webView2.forceActiveFocus();
      webView2.url = "http://testsuite/tst_WebViewPromptDialog.html";
      verify(webView2.waitForLoadSucceeded());
      verify(waitForPromptDialog());

      var dialog = getPromptDialog();
      var helper = TestSupport.createQObjectTestHelper(dialog);

      webView2.destroy();
      verify(TestUtils.waitFor(function() { return helper.destroyed; }));
    }
  }
}
