import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    name: "bug1267543"
    when: windowShown

    function test_bug1267543() {
      webView.url = "http://localhost:8080/geolocation.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
    }
  }
}
