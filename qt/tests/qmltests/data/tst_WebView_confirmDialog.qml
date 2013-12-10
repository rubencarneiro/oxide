import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  Component {
    id: customDialogComponent
    Item {
      objectName: "customDialog"
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
    }
  }

  function getDialogInstance() {
    for (var i in webView.children) {
      var child = webView.children[i];
      if (child.objectName === "customDialog") {
        return child;
      }
    }
    return null;
  }

  function dialogShown() {
    return (getDialogInstance() != null);
  }

  function dialogDismissed() {
    return (getDialogInstance() == null);
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
      webView.url = "http://localhost:8080/tst_WebView_confirmDialog.html";
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
      webView.url = "http://localhost:8080/tst_WebView_confirmDialog.html";
      verify(webView.waitFor(webView.dialogShown), "Confirm dialog not shown");
      var dialog = webView.getDialogInstance();
      compare(dialog.width, webView.width);
      compare(dialog.height, webView.height);
      compare(dialog.message, "JavaScript confirm dialog");
      mouseClick(dialog, dialog.width / 2, dialog.height / 2, data.button);
      verify(webView.waitFor(webView.dialogDismissed),
             "Confirm dialog not dismissed");
      compareResultValue(data.result);
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }
  }
}
