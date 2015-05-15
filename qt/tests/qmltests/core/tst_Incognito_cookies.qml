import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

Column {

  TestWebContext {
    id: context
  }

  TestWebView {
    id: webView1
    width: 200
    height: 100
    context: context
  }

  TestWebView {
    id: webView2
    width: 200
    height: 100
    context: context
  }

  TestWebView {
    id: webView3
    width: 200
    height: 200
    context: context
    incognito: true
  }

  TestWebView {
    id: webView4
    width: 200
    height: 200
    context: context
    incognito: true
  }

  TestCase {
    id: test
    name: "Incognito_cookies"
    when: windowShown

    function init() {
      webView1.clearLoadEventCounters();
      webView2.clearLoadEventCounters();
      webView3.clearLoadEventCounters();
      webView4.clearLoadEventCounters();
      context.deleteAllCookies();
    }

    // Verify that a cookie set in a normal webview is not accessible in an
    // incognito webview
    function test_Incognito_cookies1() {
      webView1.url = "http://testsuite/empty.html";
      verify(webView1.waitForLoadSucceeded());

      webView1.getTestApi().evaluateCode("document.cookie = \"foo=bar\"", false);

      webView2.url = "http://testsuite/empty.html";
      verify(webView2.waitForLoadSucceeded());

      compare(webView2.getTestApi().evaluateCode("document.cookie", false),
              "foo=bar");

      webView3.url = "http://testsuite/empty.html";
      verify(webView3.waitForLoadSucceeded());

      compare(webView3.getTestApi().evaluateCode("document.cookie", false), "");
    }

    // Verify that a cookie set in an incognito webview is not accessible in
    // a normal webview
    function test_Incognito_cookies2() {
      webView3.url = "http://testsuite/empty.html";
      verify(webView3.waitForLoadSucceeded());

      webView3.getTestApi().evaluateCode("document.cookie = \"foo2=bar\"", false);

      webView4.url = "http://testsuite/empty.html";
      verify(webView4.waitForLoadSucceeded());

      compare(webView4.getTestApi().evaluateCode("document.cookie", false),
              "foo2=bar");

      webView1.url = "http://testsuite/empty.html";
      verify(webView1.waitForLoadSucceeded());

      compare(webView1.getTestApi().evaluateCode("document.cookie", false), "");
    }
  }
}
