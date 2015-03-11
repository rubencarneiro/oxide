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
    name: "bug1427828"
    when: windowShown

    function test_bug1427828() {
      webView.url = "http://testsuite/tst_bug1427828.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.rootFrame.childFrames[0].sendMessageNoReply(
        "oxide://testutils/", "DONT-RESPOND", {});

      webView.rootFrame.sendMessageNoReply(
        "oxide://testutils/", "DONT-RESPOND", {});

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }
  }
}
