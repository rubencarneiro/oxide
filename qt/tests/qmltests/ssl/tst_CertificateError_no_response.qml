import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  TestCase {
    id: test
    name: "CertificateError_no_resonse"
    when: windowShown

    // Verify that the default response is "deny" when there are no handlers
    function test_CertificateError_no_response1() {
      webView.url = "https://localhost:4444/tst_CertificateError_broken_iframe.html";
      verify(webView.waitForLoadStopped(null, true));

      try {
        webView.getTestApi().documentURI;
        verify(false);
      } catch(e) {
        compare(e.error, ScriptMessageRequest.ErrorDestinationNotFound);
      }
    }
  }
}
