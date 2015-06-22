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

  TestCase {
    id: test
    name: "WebView_confirmDialog"
    when: windowShown

    function compareResultValue(expected) {
      var result = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#result\").innerHTML");
      compare(result, expected);
    }

    function test_noDialogComponent() {
      webView.confirmDialog = null;
      webView.url = "http://testsuite/tst_WebView_confirmDialog.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compareResultValue("NOK");
    }

    function test_customDialogComponent_data() {
      return [
        { button: Qt.LeftButton, result: "OK"},
        { button: Qt.RightButton, result: "NOK"}
      ];
    }

    function test_customDialogComponent(data) {
      webView.confirmDialog = customDialogComponent;
      webView.url = "http://testsuite/tst_WebView_confirmDialog.html";
      verify(TestUtils.waitFor(webView.dialogShown), "Confirm dialog not shown");
      var dialog = webView.currentDialog;
      compare(dialog.width, webView.width);
      compare(dialog.height, webView.height);
      compare(dialog.message, "JavaScript confirm dialog");
      mouseClick(dialog, dialog.width / 2, dialog.height / 2, data.button);
      verify(TestUtils.waitFor(webView.dialogDismissed),
             "Confirm dialog not dismissed");
      compareResultValue(data.result);
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }
  }
}
