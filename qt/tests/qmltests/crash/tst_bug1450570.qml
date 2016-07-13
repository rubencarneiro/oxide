import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  property bool promptDialogDestroyed: false

  promptDialog: Item {
    Component.onCompleted: model.accept("")
    onVisibleChanged: {
      if (!visible) {
        model.reject()
      }
    }
    Component.onDestruction: promptDialogDestroyed = true
  }

  TestCase {
    name: "bug1450570"
    when: windowShown

    function test_bug1450570() {
      webView.url = "http://testsuite/tst_bug1450570.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      tryCompare(webView, "promptDialogDestroyed", true);
    }
  }
}
