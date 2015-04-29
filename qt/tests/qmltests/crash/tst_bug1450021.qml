import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  TestCase {
    id: test
    name: "bug1450021"
    when: windowShown

    function test_bug1450021() {
      webView.url = "http://foo.testsuite/tst_bug1450021_1.html";
      verify(webView.waitForLoadSucceeded());

      // Initiate a process swap so that the frame tree is torn down on
      // the browser side
      webView.url = "http://bar.testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
    }
  }
}
