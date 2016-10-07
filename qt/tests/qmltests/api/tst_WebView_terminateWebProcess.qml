import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.19
import Oxide.testsupport 1.0

TestWebView {
  id: webView

  SignalSpy {
    id: spy
    target: webView
    signalName: "webProcessStatusChanged"
  }

  TestCase {
    name: "WebView_terminateWebProcess"
    when: windowShown

    function test_WebView_terminateWebProcess() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      compare(webView.webProcessStatus, WebView.WebProcessRunning);
      spy.clear();

      webView.terminateWebProcess();
      spy.wait();

      compare(webView.webProcessStatus, WebView.WebProcessKilled);
    }
  }
}
