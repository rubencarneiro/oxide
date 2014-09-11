import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "blockedContentChanged"
  }

  TestCase {
    id: test
    name: "WebView_mixedContentOverrides"
    when: windowShown

    function _did_display_mixed_content() {
      return webView.getTestApi().evaluateCode("
return document.getElementsByTagName(\"img\")[0].width;", true) == 150;
    }

    function _did_run_mixed_content() {
      return webView.getTestApi().evaluateCode("
var elem = document.getElementsByTagName(\"p\")[0];
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true) == "rgb(0, 128, 0)";
    }

    function init() {
      spy.clear();
      webView.preferences.canDisplayInsecureContent = false;
    }

    function test_WebView_mixedContentOverrides1_display() {
      // Load a URL with mixed content
      webView.url = "https://testsuite/tst_WebView_mixedContentOverrides.html";
      verify(webView.waitForLoadSucceeded());

      // Verify WebView.blockedContent indicates both were blocked
      compare(webView.blockedContent & (WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript),
              WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript);
      compare(spy.count, 3);

      // Verify that mixed content was not displayed or ran
      verify(!_did_display_mixed_content());
      verify(!_did_run_mixed_content());

      // Temporarily allow displaying mixed content
      webView.setCanTemporarilyDisplayInsecureContent(true);
      verify(webView.waitForLoadSucceeded());

      // Verify that WebView.blockedContent indicates only mixed script is blocked
      compare(webView.blockedContent & (WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript),
              WebView.ContentTypeMixedScript);
      compare(spy.count, 6);

      // Verify only mixed script is blocked
      verify(_did_display_mixed_content());
      verify(!_did_run_mixed_content());

      // Now disallow displaying mixed content
      webView.setCanTemporarilyDisplayInsecureContent(false);
      verify(webView.waitForLoadSucceeded());

      // Verify WebView.blockedContent indicates both were blocked again
      compare(webView.blockedContent & (WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript),
              WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript);
      compare(spy.count, 9);

      // Verify that no mixed content was displayed or ran
      verify(!_did_display_mixed_content());
      verify(!_did_run_mixed_content());

      // Temporarily allow displaying mixed content again
      webView.setCanTemporarilyDisplayInsecureContent(true);
      verify(webView.waitForLoadSucceeded());

      // Verify that WebView.blockedContent indicates only mixed script is blocked
      compare(webView.blockedContent & (WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript),
              WebView.ContentTypeMixedScript);
      compare(spy.count, 12);

      // Verify that only mixed script is blocked
      verify(_did_display_mixed_content());
      verify(!_did_run_mixed_content());

      // Navigate to another URL and then go back
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
      compare(webView.blockedContent, WebView.ContentTypeNone);
      webView.goBack();
      verify(webView.waitForLoadSucceeded());

      // Verify WebView.blockedContent indicates both were blocked again
      compare(webView.blockedContent & (WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript),
              WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript);
      compare(spy.count, 16);

      // Verify that no mixed content was displayed or ran
      verify(!_did_display_mixed_content());
      verify(!_did_run_mixed_content());
    }

    function test_WebView_mixedContentOverrides2_run() {
      // Load a URL with mixed content
      webView.url = "https://testsuite/tst_WebView_mixedContentOverrides.html";
      verify(webView.waitForLoadSucceeded());

      // Verify WebView.blockedContent indicates both were blocked
      compare(webView.blockedContent & (WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript),
              WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript);
      compare(spy.count, 3);

      // Verify that mixed content was not displayed or ran
      verify(!_did_display_mixed_content());
      verify(!_did_run_mixed_content());

      // Temporarily allow running mixed content
      webView.setCanTemporarilyRunInsecureContent(true);
      verify(webView.waitForLoadSucceeded());

      // Verify that WebView.blockedContent indicates no content is blocked
      compare(webView.blockedContent, WebView.ContentTypeNone);
      compare(spy.count, 5);

      // Verify that no content is blocked
      verify(_did_display_mixed_content());
      verify(_did_run_mixed_content());

      // Now disallow running mixed content
      webView.setCanTemporarilyRunInsecureContent(false);
      verify(webView.waitForLoadSucceeded());

      // Verify WebView.blockedContent indicates both were blocked again
      compare(webView.blockedContent & (WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript),
              WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript);
      compare(spy.count, 8);

      // Verify that no mixed content was displayed or ran
      verify(!_did_display_mixed_content());
      verify(!_did_run_mixed_content());

      // Temporarily allow running mixed content again
      webView.setCanTemporarilyRunInsecureContent(true);
      verify(webView.waitForLoadSucceeded());

      // Verify that WebView.blockedContent indicates no content is blocked
      compare(webView.blockedContent, WebView.ContentTypeNone);
      compare(spy.count, 10);

      // Verify that no mixed content is blocked
      verify(_did_display_mixed_content());
      verify(_did_run_mixed_content());

      // Navigate to another URL and then go back
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
      compare(webView.blockedContent, WebView.ContentTypeNone);
      webView.goBack();
      verify(webView.waitForLoadSucceeded());

      // Verify WebView.blockedContent indicates both were blocked again
      compare(webView.blockedContent & (WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript),
              WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript);
      compare(spy.count, 14);

      // Verify that no mixed content was displayed or ran
      verify(!_did_display_mixed_content());
      verify(!_did_run_mixed_content());
    }

    function test_WebView_mixedContentOverrides3_no_content_blocked() {
      webView.url = "http://testsuite/tst_WebView_mixedContentOverrides.html";
      verify(webView.waitForLoadSucceeded());

      webView.clearLoadEventCounters();

      webView.setCanTemporarilyDisplayInsecureContent(true);
      webView.setCanTemporarilyRunInsecureContent(true);
      wait(200);

      compare(webView.loadsStartedCount, 0);

      webView.setCanTemporarilyDisplayInsecureContent(false);
      webView.setCanTemporarilyRunInsecureContent(false);
      wait(200);

      compare(webView.loadsStartedCount, 2);
    }
  }
}
