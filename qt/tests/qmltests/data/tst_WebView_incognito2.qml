import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
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

      webView.url = "http://localhost:8080/tst_WebView_incognito2.py"
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var res = webView.getTestApi().evaluateCode(
          "var el = document.querySelectorAll(\".cookie\"); \
           for (var i = 0; i < el.length; ++i) { \
             var name = el[i].innerHTML.split(\"=\")[0]; \
             var value = el[i].innerHTML.split(\"=\")[1]; \
             if (name == \"foo\") return value; \
           } \
           return undefined;",
           true);
      compare(res, undefined, "Cookie should not be sent in incognito mode");
    }
  }
}
