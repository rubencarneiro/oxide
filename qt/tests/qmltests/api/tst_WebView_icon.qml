import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    name: "WebView_icon"
    when: windowShown

    function test_WebView_icon_data() {
      return [
        { url: "", icon: "" },
        { url: "http://localhost:8080/empty.html", icon: "http://localhost:8080/favicon.ico" },
        { url: "http://localhost:8080/tst_WebView_icon.html", icon: "http://localhost:8080/icon.ico" }
      ];
    }

    function test_WebView_icon(data) {
      if (data.url) {
        webView.url = data.url;
        verify(webView.waitForLoadSucceeded(),
               "Timed out waiting for a successful load");
      }
      compare(webView.icon.toString(), data.icon);
    }
  }
}
