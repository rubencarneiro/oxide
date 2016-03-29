import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "webProcessStatusChanged"
  }

  TestCase {
    name: "WebView_webProcessStatus"
    when: windowShown

    function test_WebView_webProcessStatus_data() {
      return [
        { crash: false, status: WebView.WebProcessKilled },
        { crash: true, status: WebView.WebProcessCrashed }
      ];
    }

    function test_WebView_webProcessStatus(data) {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(webView.webProcessStatus, WebView.WebProcessRunning);

      spy.clear();

      webView.killWebProcess(data.crash);
      spy.wait();

      tryCompare(webView, "webProcessStatus", data.status);

      webView.reload();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(webView.webProcessStatus, WebView.WebProcessRunning);
      compare(spy.count, 2);
    }
  }
}
