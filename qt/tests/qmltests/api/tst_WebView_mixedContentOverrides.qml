import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

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

    function init() {
      spy.clear();
      webView.preferences.canDisplayInsecureContent = false;
    }

    function test_WebView_mixedContentOverrides1_data() {
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

      function _did_display_mixed_content_in_subframe() {
        return webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode("
return document.getElementsByTagName(\"img\")[0].width;", true) == 150;
      }

      function _did_run_mixed_content_in_subframe() {
        return webView.getTestApiForFrame(webView.rootFrame.childFrames[0]).evaluateCode("
var elem = document.getElementsByTagName(\"p\")[0];
var style = window.getComputedStyle(elem);
return style.getPropertyValue(\"color\");", true) == "rgb(0, 128, 0)";
      }

      return [
        {
          url: "https://8786310.testsuite/tst_WebView_mixedContentOverrides.html", testMode: "display",
          checkMixedDisplay: _did_display_mixed_content, checkMixedScript: _did_run_mixed_content,
          hasMixedDisplay: true, hasMixedScript: true, blockedCount: [3,6,9,12,13,16]
        },
        {
          url: "https://98661.testsuite/tst_WebView_mixedContentOverrides.html", testMode: "run",
          checkMixedDisplay: _did_display_mixed_content, checkMixedScript: _did_run_mixed_content,
          hasMixedDisplay: true, hasMixedScript: true, blockedCount: [3,5,8,10,11,14]
        },
        {
          url: "https://testsuite/tst_WebView_mixedContentOverrides_in_subframe.html", testMode: "display",
          checkMixedDisplay: _did_display_mixed_content_in_subframe, checkMixedScript: _did_run_mixed_content_in_subframe,
          hasMixedDisplay: true, hasMixedScript: false, blockedCount: [2,4,6,8,9,11]
        }
      ];
    }

    function test_WebView_mixedContentOverrides1(data) {
      var blockedMask = WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript;

      var blockedContent = 0;
      if (data.hasMixedDisplay) {
        blockedContent |= WebView.ContentTypeMixedDisplay;
      }
      if (data.hasMixedScript) {
        blockedContent |= WebView.ContentTypeMixedScript;
      }

      // Load a URL with mixed content
      webView.url = data.url;
      verify(webView.waitForLoadSucceeded());

      // Verify all mixed content was blocked
      compare(webView.blockedContent & blockedMask, blockedContent);
      compare(spy.count, data.blockedCount[0]);
      verify(!data.hasMixedDisplay || !data.checkMixedDisplay());
      verify(!data.hasMixedScript || !data.checkMixedScript());

      // Temporarily allow the specified mixed content type
      var allowedContent = 0;
      if (data.testMode == "display") {
        allowedContent = WebView.ContentTypeMixedDisplay;
        webView.setCanTemporarilyDisplayInsecureContent(true);
      } else if (data.testMode == "run") {
        allowedContent = WebView.ContentTypeMixedDisplay | WebView.ContentTypeMixedScript;
        webView.setCanTemporarilyRunInsecureContent(true);
      } else {
        throw Error("Invalid test mode");
      }
      verify(webView.waitForLoadSucceeded());

      // Verify that the correct content is unblocked
      compare(webView.blockedContent & blockedMask, blockedContent & ~allowedContent);
      compare(spy.count, data.blockedCount[1]);
      if (data.testMode == "display") {
        verify(!data.hasMixedDisplay || data.checkMixedDisplay());
        verify(!data.hasMixedScript || !data.checkMixedScript());
      } else {
        verify(!data.hasMixedDisplay || data.checkMixedDisplay());
        verify(!data.hasMixedScript || data.checkMixedScript());
      }

      // Now disallow all mixed content
      if (data.testMode == "display") {
        webView.setCanTemporarilyDisplayInsecureContent(false);
      } else {
        webView.setCanTemporarilyRunInsecureContent(false);
      }
      verify(webView.waitForLoadSucceeded());

      // Verify all mixed content was blocked again
      compare(webView.blockedContent & blockedMask, blockedContent);
      compare(spy.count, data.blockedCount[2]);
      verify(!data.hasMixedDisplay || !data.checkMixedDisplay());
      verify(!data.hasMixedScript || !data.checkMixedScript());

      // Temporarily allow the specified mixed content type again
      if (data.testMode == "display") {
        webView.setCanTemporarilyDisplayInsecureContent(true);
      } else {
        webView.setCanTemporarilyRunInsecureContent(true);
      }
      verify(webView.waitForLoadSucceeded());

      // Verify that the correct content is unblocked again
      compare(webView.blockedContent & blockedMask, blockedContent & ~allowedContent);
      compare(spy.count, data.blockedCount[3]);
      if (data.testMode == "display") {
        verify(!data.hasMixedDisplay || data.checkMixedDisplay());
        verify(!data.hasMixedScript || !data.checkMixedScript());
      } else {
        verify(!data.hasMixedDisplay || data.checkMixedDisplay());
        verify(!data.hasMixedScript || data.checkMixedScript());
      }

      // Navigate to another URL and then go back
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
      compare(webView.blockedContent, WebView.ContentTypeNone);
      compare(spy.count, data.blockedCount[4]);
      webView.goBack();
      verify(webView.waitForLoadSucceeded());

      // Verify all mixed content was blocked again
      compare(webView.blockedContent & blockedMask, blockedContent);
      compare(spy.count, data.blockedCount[5]);
      verify(!data.hasMixedDisplay || !data.checkMixedDisplay());
      verify(!data.hasMixedScript || !data.checkMixedScript());
    }

    function test_WebView_mixedContentOverrides2_no_content_blocked() {
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
