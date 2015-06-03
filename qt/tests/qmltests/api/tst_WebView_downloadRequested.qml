import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property string latestDownloadUrl: ""
  property string latestSuggestedFilename: ""
  property var latestCookies
  property string latestMimeType: ""
  property string latestReferrer: ""
  property string latestUserAgent: ""

  context.userAgentOverrideDelegate: WebContextDelegateWorker {
    source: Qt.resolvedUrl("tst_WebView_downloadRequested.js");
  }

  onDownloadRequested: {
    latestDownloadUrl = request.url;
    latestCookies = [].slice.call(request.cookies).join(",");
    latestMimeType = request.mimeType;
    latestSuggestedFilename = request.suggestedFilename;
    latestReferrer = request.referrer;
    latestUserAgent = request.userAgent;
  }

  function cleanLatestData() {
    latestDownloadUrl = "";
    latestCookies = "";
    latestMimeType = "";
    latestSuggestedFilename = "";
    latestReferrer = "";
    latestUserAgent = "";
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "downloadRequested"
  }

  TestCase {
    name: "WebView_downloadRequest"
    when: windowShown

    function init() {
      spy.clear();
      cleanLatestData();
    }

    function test_WebView_downloadRequestWithContentDisposition() {
      webView.url = "http://testsuite/tst_WebView_downloadRequested.py"
      verify(webView.waitForLoadStopped(),
             "Timed out waiting for a successful load");

      compare(spy.count, 1)
      compare(webView.latestMimeType, "text/html")
      compare(webView.latestCookies, "foo=bar, bar=bazz")
      compare(webView.latestSuggestedFilename, "001.html")
    }

    function test_WebView_downloadRequestWithContentDispositionWithUserAgentOverride() {
      webView.url = "http://testsuite/tst_WebView_downloadRequested.py?override"
      verify(webView.waitForLoadStopped(),
             "Timed out waiting for a successful load");

      compare(spy.count, 1)
      compare(webView.latestMimeType, "text/html")
      compare(webView.latestCookies, "foo=bar, bar=bazz")
      compare(webView.latestSuggestedFilename, "001.html")
      compare(webView.latestUserAgent, "Override download user agent string");
    }

    function test_WebView_downloadAnchorRequest() {
      webView.url = "http://testsuite/tst_WebView_downloadRequestedAnchor.html"
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for a successful load");

      var r = webView.getTestApi().getBoundingClientRectForSelector("#content");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.LeftButton);
      spy.wait();

      compare(spy.count, 1)
      compare(webView.latestSuggestedFilename, "MyDownload.html")
      compare(webView.latestMimeType, "text/html")
    }

    function test_WebView_downloadRequestUnhandledMimeType() {
      webView.url = "http://testsuite/tst_WebView_downloadRequestedUnhandledMimeType.py"
      verify(webView.waitForLoadStopped(),
             "Timed out waiting for a successful load");

      compare(spy.count, 1)
      compare(webView.latestMimeType, "application/pdf")
      compare(webView.latestCookies, "foo=bar")
      compare(webView.latestSuggestedFilename, "")
    }
  }
}
