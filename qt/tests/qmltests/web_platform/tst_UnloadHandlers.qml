import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Column {
  WebPreferences {
    id: p
    localStorageEnabled: true
  }

  TestWebContext {
    id: c
  }

  TestWebView {
    id: webView1
    width: 200
    height: 200

    context: c
    preferences: p
  }

  TestWebView {
    id: webView2
    width: 200
    height: 200

    context: c
    preferences: p
  }

  TestCase {
    id: test
    name: "UnloadHandlers"
    when: windowShown

    function test_UnloadHandlers1() {
      webView1.url = "http://testsuite/tst_UnloadHandlers.html";
      verify(webView1.waitForLoadSucceeded());

      webView2.url = "http://testsuite/empty.html";
      verify(webView2.waitForLoadSucceeded());

      TestSupport.destroyQObjectNow(webView1);

      compare(webView2.getTestApi().evaluateCode("window.localStorage.getItem(\"oxide-shutdown-foo\");", false), "baaa");
    }
  }
}
