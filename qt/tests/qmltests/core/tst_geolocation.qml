import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  function geolocation_succeeded() {
    var result = webView.getTestApi().evaluateCode(
        "document.querySelector(\"#location\").innerHTML");
    return (result === "OK");
  }

  TestCase {
    name: "geolocation"
    when: windowShown

    function test_geolocation() {
      webView.url = "http://localhost:8080/tst_geolocation.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      verify(webView.waitFor(webView.geolocation_succeeded),
             "Geolocation failed");
    }
  }
}
