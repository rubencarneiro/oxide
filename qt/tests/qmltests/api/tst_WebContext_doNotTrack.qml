import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.9
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  SignalSpy {
    id: dntSignalSpy
    signalName: "doNotTrackEnabledChanged"
  }

  Component.onCompleted: dntSignalSpy.target = webView.context

  TestCase {
    id: test
    name: "WebContext_doNotTrack"
    when: windowShown

    function init() {
      dntSignalSpy.clear();
    }
 
    function test_dnt_data() {
      return [
        {expectedHeaderDnt: "0", expectedNavigatorDnt: "0"}, /* default */
        {expectedHeaderDnt: "0", expectedNavigatorDnt: "0", dnt: false},
        {expectedHeaderDnt: "1", expectedNavigatorDnt: "1", dnt: true}
      ]
    }

    function test_dnt(data) {
      if (data.dnt != null) {
        webView.context.doNotTrackEnabled = data.dnt
      }

      webView.url = "http://testsuite/tst_WebContext_doNotTrack.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().evaluateCode(
              "document.querySelector(\"#dnt\").innerHTML"),
              data.expectedHeaderDnt);
      compare(webView.getTestApi().evaluateCode(
              "window.navigator.doNotTrack && window.navigator.doNotTrack === '1' ? '1' : '0'"),
              data.expectedNavigatorDnt);
    }

    function test_dntNavigatorUpdate(data) {
      webView.context.doNotTrackEnabled = false

      webView.url = "http://testsuite/tst_WebContext_doNotTrack.py";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      compare(webView.getTestApi().evaluateCode(
              "window.navigator.doNotTrack && window.navigator.doNotTrack === '1' ? '1' : '0'"),
              "0");

      webView.context.doNotTrackEnabled = true;

      compare(webView.getTestApi().evaluateCode(
              "window.navigator.doNotTrack && window.navigator.doNotTrack === '1' ? '1' : '0'"),
              "1");

      compare(dntSignalSpy.count, 2, "Should have had 1 doNotTrackEnabledChanged signal");
    }
  }
}
