import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

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
        { url: "chrome://kill/", status: WebView.WebProcessKilled, needsReload: true, signals: 2 },
        { url: "chrome://crash/", status: WebView.WebProcessCrashed, needsReload: true, signals: 2 },
        { url: "chrome://hang/", status: WebView.WebProcessUnresponsive, needsReload: true, signals: 4 },
        { url: "chrome://shorthang/", status: WebView.WebProcessUnresponsive, needsReload: false, signals: 2 }
      ];
    }

    function test_WebView_webProcessStatus(data) {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      compare(webView.webProcessStatus, WebView.WebProcessRunning);

      spy.clear();

      webView.url = data.url;

      // Send some events, as we need those to start the hang monitor
      for (var i = 0; i < 100; i++) {
        keyClick("A");
        mouseClick(webView, webView.width / 2, webView.height / 2, Qt.LeftButton);
      }

      // It takes 5 seconds for the hang monitor to fire
      spy.wait(10000);

      tryCompare(webView, "webProcessStatus", data.status);

      if (data.needsReload) {
        if (webView.webProcessStatus == WebView.WebProcessUnresponsive) {
          webView.terminateWebProcess();
          TestSupport.wait(1000);
        }
        webView.reload();
        verify(webView.waitForLoadSucceeded(),
               "Timed out waiting for successful load");
      } else {
        // Wait for the hung process to start responding
        spy.wait(30000);
      }

      compare(webView.webProcessStatus, WebView.WebProcessRunning);
      compare(spy.count, data.signals);
    }
  }
}
