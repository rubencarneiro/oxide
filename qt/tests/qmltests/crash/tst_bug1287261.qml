import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

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
      webView.url = "http://localhost:8080/empty.html"
      verify(webView.waitForLoadSucceeded());

      var hadError = false;
      var req = webView.rootFrame.sendMessage("oxide://testutils/", "GET-DOCUMENT-URI", {});
      req.onerror = function(code, msg) {
        hadError = true;
      };
      webView.waitFor(function() { return false; });
      verify(!hadError, "Wasn't expecting an error");
    }
  }
}
