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
      property string value: model.defaultValue
      anchors.fill: parent
      Binding {
        target: model
        property: "currentValue"
        value: customDialog.value
      }
      MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
          if (mouse.button == Qt.LeftButton) {
            model.accept(customDialog.value);
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
    name: "WebView_promptDialog"
    when: windowShown

    function compareResultAndValue(expectedResult, expectedValue) {
      var result = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#result\").innerHTML");
      compare(result, expectedResult);
      var value = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#value\").innerHTML");
      compare(value, expectedValue);
    }

    function test_noDialogComponent() {
      webView.promptDialog = null;
      webView.url = "http://localhost:8080/tst_WebView_promptDialog.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compareResultAndValue("NOK", "");
    }

    function test_customDialogComponent_data() {
      return [
        { button: Qt.LeftButton, param: null, result: "OK", value: "DEFAULT VALUE"},
        { button: Qt.LeftButton, param: "", result: "OK", value: ""},
        { button: Qt.LeftButton, param: "hello world", result: "OK", value: "HELLO WORLD"},
        { button: Qt.RightButton, param: null, result: "NOK", value: ""}
      ];
    }

    function test_customDialogComponent(data) {
      webView.promptDialog = customDialogComponent;
      webView.url = "http://localhost:8080/tst_WebView_promptDialog.html";
      verify(webView.waitFor(webView.dialogShown), "Prompt dialog not shown");
      var dialog = webView.currentDialog;
      compare(dialog.width, webView.width);
      compare(dialog.height, webView.height);
      compare(dialog.message, "JavaScript prompt dialog");
      compare(dialog.value, "default value");
      if (data.param != null) {
        dialog.value = data.param;
      }
      mouseClick(dialog, dialog.width / 2, dialog.height / 2, data.button);
      verify(webView.waitFor(webView.dialogDismissed),
             "Confirm dialog not dismissed");
      compareResultAndValue(data.result, data.value);
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }
  }
}
