import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView

  WebContextDelegateWorker {
    id: worker
    source: Qt.resolvedUrl("tst_UserAgentOverrideRequest.js");
  }

  Component.onCompleted: {
    context.userAgentOverrideDelegate = worker;
    context.userAgent = "Oxide Test";
  }

  TestCase {
    id: test
    name: "UserAgentOverrideRequest"
    when: windowShown

    function test_UserAgentOverrideRequest() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              "Oxide Test", "Unexpected default user agent string");

      webView.url = "http://testsuite/empty.html?override";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().evaluateCode("return navigator.userAgent", true),
              "Override user agent string", "Failed to override user agent string");
    }
  }
}
