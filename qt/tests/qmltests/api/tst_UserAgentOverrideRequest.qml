import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView

  context.userAgentOverrideDelegate: WebContextDelegateWorker {
    source: Qt.resolvedUrl("tst_UserAgentOverrideRequest.js");
  }

  context.userAgent: "Oxide Test"

  TestCase {
    id: test
    name: "UserAgentOverrideRequest"
    when: windowShown

    function test_UserAgentOverrideRequest() {
      webView.url = "http://localhost:8080/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              "Oxide Test", "Unexpected default user agent string");

      webView.url = "http://localhost:8080/empty.html?override";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              "Override user agent string", "Failed to override user agent string");
    }
  }
}
