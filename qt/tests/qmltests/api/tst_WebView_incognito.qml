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
    incognito: true
  }

  SignalSpy {
    id: spy
    signalName: "incognitoChanged"
  }

  TestCase {
    id: test
    name: "WebView_incognito"
    when: windowShown

    function init() {
      webView1.z = 0;
      webView2.z = 0;
    }

    // Verify WebView.incognito is as expected after construction
    function test_WebView_incognito1() {
      verify(!webView1.incognito);
      verify(webView2.incognito);
    }

    // Verify that a cookie set in a normal webview isn't visible in an
    // incognito webview
    function test_WebView_incognito2() {
      webView1.z = 1;
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

      webView1.z = 0;
      webView2.z = 1;

      webView2.url = "http://testsuite/get-cookies.py"
      verify(webView2.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var cookies = JSON.parse(webView2.getTestApi().evaluateCode(
          "return document.body.children[0].innerHTML", true));
      verify(!("foo" in cookies), "Cookie should not be sent in incognito mode");
    }

    // Verify that WebView.incognito is read-only on a constructed webview
    function test_WebView_incognito3() {
      spy.target = webView1;
      webView1.incognito = true;
      compare(spy.count, 0);

      spy.target = webView2;
      webView2.incognito = false;
      compare(spy.count, 0);
    }
  }
}
