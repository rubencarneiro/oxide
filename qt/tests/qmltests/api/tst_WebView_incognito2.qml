import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200
  incognito: true

  TestCase {
    id: test
    name: "WebView_incognito2"
    when: windowShown

    function test_WebView_incognito2() {
      if (!webView.context.dataPath) {
        console.log("Skipping this test because there is no data path");
        return;
      }

      console.log("Data path: " + webView.context.dataPath);

      webView.url = "http://localhost:8080/get-cookies.py"
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var cookies = JSON.parse(webView.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));
      verify(!("foo" in cookies), "Cookie should not be sent in incognito mode");
    }
  }
}
