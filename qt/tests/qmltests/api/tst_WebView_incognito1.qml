import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    id: test
    name: "WebView_incognito1"
    when: windowShown

    function test_WebView_incognito1() {
      if (!webView.context.dataPath) {
        console.log("Skipping this test because there is no data path");
        return;
      }

      webView.url = "http://localhost:8080/tst_WebView_incognito1.html?name=foo&value=bar"
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.url = "http://localhost:8080/get-cookies.py"
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      console.log(webView.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));
      var cookies = JSON.parse(webView.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));
      compare(cookies["foo"], "bar", "Cookie was not set correctly");
    }
  }
}
