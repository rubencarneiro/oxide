import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import com.canonical.Oxide.Testing 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    name: "WebView_webProcessStatus"
    when: windowShown

    function test_WebView_webProcessStatus_data() {
      return [
        { signal: 9, status: WebView.WebProcessKilled },
        { signal: 11, status: WebView.WebProcessCrashed }
      ];
    }

    function test_WebView_webProcessStatus(data) {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(webView.webProcessStatus, WebView.WebProcessRunning);

      Utils.killWebProcesses(data.signal);
      tryCompare(webView, "webProcessStatus", data.status);

      webView.reload();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(webView.webProcessStatus, WebView.WebProcessRunning);
    }
  }
}
