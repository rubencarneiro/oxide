import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

Column {
  WebPreferences {
    id: p
    localStorageEnabled: true
  }

  TestWebView {
    id: webView1
    width: 200
    height: 200

    preferences: p
  }

  TestWebView {
    id: webView2
    width: 200
    height: 200

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

      var obs = OxideTestingUtils.createDestructionObserver(webView1);
      webView1.destroy(0);
      webView2.waitFor(function() { return obs.destroyed; });

      compare(webView2.getTestApi().evaluateCode("window.localStorage.getItem(\"oxide-shutdown-foo\");", false), "baaa");
    }
  }
}
