import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 250
  height: 250

  SignalSpy {
    id: spy
    target: webView
    signalName: "zoomFactorChanged"
  }

  TestCase {
    name: "WebView_zoomFactor"
    when: windowShown

    readonly property int originalSize: 40
    readonly property color red: "red"
    readonly property color white: "white"

    function verify_zoomFactor(factor) {
      compare(webView.zoomFactor, factor);
      var image = grabImage(webView);
      var zoomedSize = originalSize * factor;
      for (var i = 0; i < zoomedSize; ++i) {
        compare(image.pixel(i, i), red);
      }
      compare(image.pixel(zoomedSize + 1, zoomedSize + 1), white);
    }

    function test_WebView_zoomFactor() {
      webView.url = "http://testsuite/tst_WebView_zoomFactor.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      verify_zoomFactor(1.0);
      compare(spy.count, 0);

      var goodValues = [1.5, 2.0, 3.7, 4.873, 5.0, 0.25, 0.9, 0.5, 1.0];
      for (var i in goodValues) {
        var value = goodValues[i];
        webView.zoomFactor = value;
        compare(spy.count, 1);
        verify_zoomFactor(value);
        spy.clear();
      }

      var badValues = [0.1, 0.0, -1.0, 5.1, 100.0];
      for (var i in badValues) {
        var value = badValues[i];
        webView.zoomFactor = value;
        compare(spy.count, 0);
        verify_zoomFactor(1.0);
      }
    }
  }
}
