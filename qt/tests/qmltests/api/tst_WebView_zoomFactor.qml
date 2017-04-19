import QtQuick 2.0
import QtTest 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  SignalSpy {
    id: spy
    target: webView
    signalName: "zoomFactorChanged"
  }

  Component {
    id: webViewFactory
    TestWebView {}
  }

  SignalSpy {
    id: otherSpy
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
      function testpixel(i, color) {
        var image = grabImage(webView);
        return (image.pixel(i, i) == color);
      }
      var zoomedSize = Math.round(originalSize * factor);
      verify(TestUtils.waitFor(function() { return testpixel(zoomedSize - 1, red); }));
      verify(TestUtils.waitFor(function() { return testpixel(zoomedSize, white); }));
      var image = grabImage(webView);
      for (var i = 0; i < zoomedSize; ++i) {
        compare(image.pixel(i, i), red);
      }
      compare(image.pixel(zoomedSize, zoomedSize), white);
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
        ignoreWarning("OxideQQuickWebView: invalid value for zoom factor, " +
                      "expected to be between 0.25 and 5");
        webView.zoomFactor = value;
        compare(spy.count, 0);
        verify_zoomFactor(1.0);
      }
    }

    function test_WebView_zoomFactor_is_per_host() {
      webView.url = "http://testsuite/tst_WebView_zoomFactor.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var otherWebView = webViewFactory.createObject(webView);
      otherWebView.url = "http://testsuite/empty.html";
      verify(otherWebView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      otherSpy.target = otherWebView;

      compare(spy.count, 0);
      compare(otherSpy.count, 0);

      var newZoom = 1.1;
      webView.zoomFactor = newZoom;
      compare(spy.count, 1);
      compare(otherSpy.count, 1);
      compare(webView.zoomFactor, newZoom);
      compare(otherWebView.zoomFactor, newZoom);

      otherSpy.target = null;
      otherWebView.destroy();
    }

    function test_WebView_zoomFactor_cannot_be_set_during_construction() {
      ignoreWarning("OxideQQuickWebView: zoom factor cannot be set during " +
                    "construction, it is a per-host value");
      var props = {'zoomFactor': 1.1};
      var otherWebView = webViewFactory.createObject(webView, props);
      compare(otherWebView.zoomFactor, 1.0);
      otherWebView.destroy();
    }
  }
}
