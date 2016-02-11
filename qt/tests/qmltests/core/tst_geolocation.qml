import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  function expect_result(selector, expected) {
    var result = webView.getTestApi().evaluateCode(
        "document.querySelector(\"#" + selector + "\").innerHTML");
    return (result === expected);
  }

  onGeolocationPermissionRequested: {
    request.accept();
  }

  TestCase {
    name: "geolocation"
    when: windowShown

    function test_geolocation_get_data() {
      return [
        { testcase: "", result: "OK" },
        { testcase: "invaliddata", result: "OK" },
        { testcase: "timeout", result: "TIMEOUT" },
        { testcase: "error-permission", result: "PERMISSION DENIED" },
        { testcase: "error-unavailable", result: "POSITION UNAVAILABLE" }
      ];
    }

    function test_geolocation_get(data) {
      Utils.setAppProperty("_oxide_geo_testcase", data.testcase);
      webView.url = "https://testsuite/tst_geolocation_get.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      verify(TestUtils.waitFor(function() { return expect_result("location", data.result); }));
    }

    function test_geolocation_watch() {
      Utils.removeAppProperty("_oxide_geo_testcase");
      webView.url = "https://testsuite/tst_geolocation_watch.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      verify(TestUtils.waitFor(function() { return expect_result("updates", "5"); }, 15000));
      verify(expect_result("errors", "0"));
    }
  }
}
