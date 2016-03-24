import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property bool picked: false

  filePicker: Item {
    Component.onCompleted: model.accept([""])
    Component.onDestruction: webView.picked = true
  }

  TestCase {
    name: "bug1352952"
    when: windowShown

    function test_bug1352952() {
      webView.url = "http://testsuite/tst_bug1352952.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      mouseClick(webView, webView.width / 2, webView.height / 2);
      verify(TestUtils.waitFor(function() { return webView.picked; }),
             "Timed out waiting for filepicker to be dismissed");

      // Check that the renderer doesn’t crash. If it does,
      // it might not be right away, so give it some time.
      for (var i = 0; i < 5; ++i) {
        Utils.wait(500);
        // Calling into the test API will raise an exception
        // if the renderer process has crashed.
        webView.getTestApi().documentURI;
      }
    }
  }
}
