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
    name: "bug1287261"
    when: windowShown

    function test_bug1287261() {
      webView.url = "http://testsuite/empty.html"
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      // We send 2 messages - the first one with on onreply handler, and the second one
      // with. We assume that the second one returns last, and thus we shouldn't see it
      // if the first one crashes

      var hadError = false;
      var req = webView.rootFrame.sendMessage("oxide://testutils/", "GET-DOCUMENT-URI", {});
      req.onerror = function(code, msg) {
        hadError = true;
      };

      verify(webView.getTestApi().documentURI, "Timed out waiting for response");
      verify(!hadError, "Wasn't expecting an error");
    }
  }
}
