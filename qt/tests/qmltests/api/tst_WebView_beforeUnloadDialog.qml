import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property var currentDialog: null

  Component {
    id: customDialogComponent
    Item {
      id: customDialog
      readonly property string message: model.message
      anchors.fill: parent
      MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
          if (mouse.button == Qt.LeftButton) {
            model.accept();
          } else if (mouse.button == Qt.RightButton) {
            model.reject();
          }
        }
      }
      Component.onCompleted: {
        WebView.view.currentDialog = customDialog;
      }
      Component.onDestruction: {
        WebView.view.currentDialog = null;
      }
    }
  }

  function dialogShown() {
    return (currentDialog != null);
  }

  function dialogDismissed() {
    return (currentDialog == null);
  }

  function checkContents() {
    return (webView.getTestApi().evaluateCode(
        "document.querySelector(\"#contents\").innerHTML") === "OK");
  }

  TestCase {
    id: test
    name: "WebView_beforeUnloadDialog"
    when: windowShown

    function test_noDialogComponent() {
      webView.beforeUnloadDialog = null;
      webView.url = "http://testsuite/tst_WebView_beforeUnloadDialog.html";
      tryCompare(webView, "url",
                 "http://testsuite/tst_WebView_beforeUnloadDialog2.html");
    }

    function test_customDialogComponent_data() {
      return [
        { button: Qt.LeftButton, leave: true},
        { button: Qt.RightButton, leave: false}
      ];
    }

    function test_customDialogComponent(data) {
      webView.beforeUnloadDialog = customDialogComponent;
      webView.url = "http://testsuite/tst_WebView_beforeUnloadDialog.html";
      verify(webView.waitFor(webView.dialogShown),
             "Before unload dialog not shown");
      var dialog = webView.currentDialog;
      compare(dialog.width, webView.width);
      compare(dialog.height, webView.height);
      compare(dialog.message, "Confirm navigation");
      mouseClick(dialog, dialog.width / 2, dialog.height / 2, data.button);
      verify(webView.waitFor(webView.dialogDismissed),
             "Before unload dialog not dismissed");
      if (data.leave) {
        tryCompare(webView, "url",
                   "http://testsuite/tst_WebView_beforeUnloadDialog2.html");
      } else {
        verify(webView.waitFor(webView.checkContents));
      }
    }
  }
}
