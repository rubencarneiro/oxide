import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

Item {
  WebPreferences {
    id: p
    localStorageEnabled: true
  }

  TestWebView {
    id: webView1
    anchors.fill: parent

    preferences: p
  }

  TestWebView {
    id: webView2
    anchors.fill: parent

    preferences: p
  }

  TestCase {
    id: test
    name: "UnloadHandlers"
    when: windowShown

    function test_UnloadHandlers1() {
      webView1.z = 1;
      webView1.url = "http://testsuite/tst_UnloadHandlers.html";
      verify(webView1.waitForLoadSucceeded());

      webView2.z = 2;
      webView2.url = "http://testsuite/empty.html";
      verify(webView2.waitForLoadSucceeded());

      TestSupport.destroyQObjectNow(webView1);

      compare(webView2.getTestApi().evaluateCode("window.localStorage.getItem(\"oxide-shutdown-foo\");", false), "baaa");
    }
  }
}
