import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    id: test
    name: "bug1231650"
    when: windowShown

    property var requests: []

    function test_bug1231650() {
      webView.url = "http://localhost:8080/tst_bug1231650.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      function sendMessage(frame) {
        requests.push(webView.rootFrame.childFrames[0].sendMessage("oxide://testutils/", "DONT-RESPOND", {}));
      }

      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();
      sendMessage();

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      while (requests.length > 0) {
        var req = requests.pop();
        OxideTestingUtils.destroyQObjectNow(req);
      }
    }
  }
}
