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
        onClicked: model.accept()
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
    name: "WebView_alertDialog"
    when: windowShown

    function test_noDialogComponent() {
      webView.alertDialog = null;
      webView.url = "http://localhost:8080/tst_WebView_alertDialog.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }

    function test_customDialogComponent() {
      webView.alertDialog = customDialogComponent;
      webView.url = "http://localhost:8080/tst_WebView_alertDialog.html";
      verify(webView.waitFor(webView.dialogShown), "Alert dialog not shown");
      var dialog = webView.currentDialog;
      compare(dialog.width, webView.width);
      compare(dialog.height, webView.height);
      compare(dialog.message, "JavaScript alert dialog");
      mouseClick(dialog, dialog.width / 2, dialog.height / 2);
      verify(webView.waitFor(webView.dialogDismissed),
             "Alert dialog not dismissed");
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }
  }
}
