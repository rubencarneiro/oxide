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

      verify(webView.minimumZoomFactor < 1.0);
      verify(webView.minimumZoomFactor > 0.0);

      verify(webView.maximumZoomFactor > 1.0);

      for (var i = webView.minimumZoomFactor;
           i <= webView.maximumZoomFactor;
           i += 0.1) {
        webView.zoomFactor = i;
        compare(spy.count, 1);
        verify_zoomFactor(i);
        spy.clear();
      }
      webView.zoomFactor = 1.0;
      spy.clear();

      var badValues = [0.0, webView.minimumZoomFactor - 0.01,
                       -1.0, webView.maximumZoomFactor + 0.01];
      for (var i in badValues) {
        var value = badValues[i];
        webView.zoomFactor = value;
        compare(spy.count, 0);
        verify_zoomFactor(1.0);
      }
    }
  }
}
