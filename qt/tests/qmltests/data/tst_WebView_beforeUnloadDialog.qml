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
      webView.url = "http://localhost:8080/tst_WebView_beforeUnloadDialog.html";
      tryCompare(webView, "url",
                 "http://localhost:8080/tst_WebView_beforeUnloadDialog2.html");
    }

    function test_customDialogComponent_data() {
      return [
        { button: Qt.LeftButton, leave: true},
        { button: Qt.RightButton, leave: false}
      ];
    }

    function test_customDialogComponent(data) {
      webView.beforeUnloadDialog = customDialogComponent;
      webView.url = "http://localhost:8080/tst_WebView_beforeUnloadDialog.html";
      verify(webView.waitFor(webView.dialogShown),
             "Before unload dialog not shown");
      var dialog = webView.getDialogInstance();
      compare(dialog.width, webView.width);
      compare(dialog.height, webView.height);
      compare(dialog.message, "Confirm navigation");
      mouseClick(dialog, dialog.width / 2, dialog.height / 2, data.button);
      verify(webView.waitFor(webView.dialogDismissed),
             "Before unload dialog not dismissed");
      if (data.leave) {
        tryCompare(webView, "url",
                   "http://localhost:8080/tst_WebView_beforeUnloadDialog2.html");
      } else {
        verify(webView.waitFor(webView.checkContents));
      }
    }
  }
}
