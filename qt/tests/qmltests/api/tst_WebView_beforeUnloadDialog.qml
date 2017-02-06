import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

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

  TestCase {
    id: test
    name: "WebView_beforeUnloadDialog"
    when: windowShown

    function init() {
      webView.clearLoadEventCounters();
    }

    function test_noDialogComponent() {
      webView.beforeUnloadDialog = null;
      webView.url = "http://testsuite/tst_WebView_beforeUnloadDialog.html";
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
      compare(webView.getTestApi().documentURI, "http://testsuite/empty.html");
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
      verify(webView.waitForLoadSucceeded());

      webView.url = "http://testsuite/empty.html";
      verify(TestUtils.waitFor(webView.dialogShown),
             "Before unload dialog not shown");

      var dialog = webView.currentDialog;
      compare(dialog.width, webView.width);
      compare(dialog.height, webView.height);
      verify(dialog.message);
      mouseClick(dialog, dialog.width / 2, dialog.height / 2, data.button);
      verify(TestUtils.waitFor(webView.dialogDismissed),
             "Before unload dialog not dismissed");
      if (data.leave) {
        verify(webView.waitForLoadSucceeded());
        compare(webView.getTestApi().documentURI, "http://testsuite/empty.html");
      } else {
        compare(webView.getTestApi().documentURI, "http://testsuite/tst_WebView_beforeUnloadDialog.html");
      }
    }
  }
}
