import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property bool popupMenuDestroyed: false

  popupMenu: Item {
    Component.onCompleted: {
      model.items.select(3)
      model.accept()
    }
    onVisibleChanged: {
      if (!visible) {
        model.cancel()
      }
    }
    Component.onDestruction: popupMenuDestroyed = true
  }

  TestCase {
    name: "bug1450243"
    when: windowShown

    function test_bug1450243() {
      webView.url = "http://testsuite/tst_bug1450243.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var r = webView.getTestApi().getBoundingClientRectForSelector("#test");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);

      tryCompare(webView, "popupMenuDestroyed", true);
    }
  }
}
