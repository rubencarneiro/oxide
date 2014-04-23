import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property string testcase: ""

  function expect_result(expected) {
    var result = webView.getTestApi().evaluateCode(
        "document.querySelector(\"#location\").innerHTML");
    return (result === expected);
  }

  TestCase {
    name: "geolocation"
    when: windowShown

    function test_geolocation_data() {
      return [
        { testcase: "", result: "OK" },
        { testcase: "timeout", result: "TIMEOUT" },
        { testcase: "error-permission", result: "PERMISSION DENIED" },
        { testcase: "error-unavailable", result: "POSITION UNAVAILABLE" }
      ];
    }

    function test_geolocation(data) {
      webView.testcase = data.testcase;
      webView.url = "http://localhost:8080/tst_geolocation.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      verify(webView.waitFor(function() { return expect_result(data.result); }));
    }
  }
}
