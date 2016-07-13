import QtQuick 2.0
import QtTest 1.0
import Oxide.testsupport 1.0

Item {
  TestWebView {
    id: webView1
    anchors.fill: parent
  }

  TestWebView {
    id: webView2
    anchors.fill: parent
  }

  TestWebView {
    id: webView3
    anchors.fill: parent
    incognito: true
  }

  TestWebView {
    id: webView4
    anchors.fill: parent
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
      SingletonTestWebContext.deleteAllCookies();
      webView1.z = 0;
      webView2.z = 0;
      webView3.z = 0;
      webView4.z = 0;
    }

    // Verify that a cookie set in a normal webview is not accessible in an
    // incognito webview
    function test_Incognito_cookies1() {
      webView1.z = 1;
      webView1.url = "http://testsuite/empty.html";
      verify(webView1.waitForLoadSucceeded());

      webView1.getTestApi().evaluateCode("document.cookie = \"foo=bar\"", false);

      webView1.z = 0;
      webView2.z = 1;

      webView2.url = "http://testsuite/empty.html";
      verify(webView2.waitForLoadSucceeded());

      compare(webView2.getTestApi().evaluateCode("document.cookie", false),
              "foo=bar");

      webView2.z = 0;
      webView3.z = 1;

      webView3.url = "http://testsuite/empty.html";
      verify(webView3.waitForLoadSucceeded());

      compare(webView3.getTestApi().evaluateCode("document.cookie", false), "");
    }

    // Verify that a cookie set in an incognito webview is not accessible in
    // a normal webview
    function test_Incognito_cookies2() {
      webView3.z = 1;
      webView3.url = "http://testsuite/empty.html";
      verify(webView3.waitForLoadSucceeded());

      webView3.getTestApi().evaluateCode("document.cookie = \"foo2=bar\"", false);

      webView3.z = 0;
      webView4.z = 1;

      webView4.url = "http://testsuite/empty.html";
      verify(webView4.waitForLoadSucceeded());

      compare(webView4.getTestApi().evaluateCode("document.cookie", false),
              "foo2=bar");

      webView4.z = 0;
      webView1.z = 1;

      webView1.url = "http://testsuite/empty.html";
      verify(webView1.waitForLoadSucceeded());

      compare(webView1.getTestApi().evaluateCode("document.cookie", false), "");
    }
  }
}
