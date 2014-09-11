import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

Column {
  focus: true

  // XXX: Property assignment to work around https://launchpad.net/bugs/1292593
  TestWebContext {
    id: context
  }

  TestWebView {
    id: webView1
    width: 200
    height: 200
    context: context
  }

  TestWebView {
    id: webView2
    width: 200
    height: 200
    incognito: true
    context: context
  }

  TestWebView {
    id: webView3
    width: 200
    height: 200
    context: context
  }

  TestCase {
    id: test
    name: "WebView_incognito"
    when: windowShown

    function test_WebView_incognito1() {
      if (!webView1.context.dataPath) {
        console.log("Skipping this test because there is no data path");
        return;
      }

      webView1.url = "http://testsuite/tst_WebView_incognito.py"
      verify(webView1.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView1.url = "http://testsuite/get-cookies.py"
      verify(webView1.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      console.log(webView1.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));
      var cookies = JSON.parse(webView1.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));
      compare(cookies["foo"], "bar", "Cookie was not set correctly");

      webView2.url = "http://testsuite/get-cookies.py"
      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var cookies = JSON.parse(webView2.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));
      verify(!("foo" in cookies), "Cookie should not be sent in incognito mode");

      // Just to be sure, make sure we can access the original cookie from a third
      // (non-incognito) webview
      webView3.url = "http://testsuite/get-cookies.py"
      verify(webView3.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var cookies = JSON.parse(webView3.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));
      compare(cookies["foo"], "bar", "Cookie was not accessible in another webview");
    }
  }
}
