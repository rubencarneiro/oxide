import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  width: 200
  height: 200

  TestCase {
    name: "Vibration"
    when: windowShown

    function _getMockApi() {
      return TestSupport.getAppProperty("_oxide_feedback_mock_api");
    }

    function initTestCase() {
      // Prime QtFeedback so that it creates our mock backend
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());
      webView.getTestApi().evaluateCode("navigator.vibrate(1000);");

      TestUtils.waitFor(function() {
        var mockApi = _getMockApi();
        return !!mockApi;
      });
    }

    function test_Vibration1_simple() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      var mockApi = _getMockApi();

      var effectRan = false;
      var effectDuration;
      var effectIntensity;

      mockApi.effectStarted.connect(function(id, duration, intensity) {
        effectRan = true;
        effectDuration = duration;
        effectIntensity = intensity;
      });

      webView.getTestApi().evaluateCode("navigator.vibrate(2000);");

      TestUtils.waitFor(function() { return effectRan; });
      compare(effectDuration, 2000);
      compare(effectIntensity, 1.0);
    }

    function test_Vibration2_sanitize() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      var mockApi = _getMockApi();

      var effectRan = false;
      var effectDuration;
      var effectIntensity;

      mockApi.effectStarted.connect(function(id, duration, intensity) {
        effectRan = true;
        effectDuration = duration;
        effectIntensity = intensity;
      });

      webView.getTestApi().evaluateCode("navigator.vibrate(200000000);");

      TestUtils.waitFor(function() { return effectRan; });
      compare(effectDuration, 10000);
      compare(effectIntensity, 1.0);
    }

    function test_Vibration3_cancel() {
      webView.url = "http://testsuite/tst_Vibration_subframe.html";
      verify(webView.waitForLoadSucceeded());

      var mockApi = _getMockApi();

      var effectId;
      var gotEffectCancel = false;
      mockApi.effectStarted.connect(function(id, duration, intensity) {
        effectId = id;
      });

      mockApi.effectStopped.connect(function(id) {
        if (id == effectId) {
          gotEffectCancel = true;
        }
      });

      var frame = webView.rootFrame.childFrames[0];
      webView.getTestApiForFrame(frame).evaluateCode("navigator.vibrate(10000);");
      webView.getTestApi().evaluateCode("
var e = document.getElementById(\"subframe\");
e.parentNode.removeChild(e);");

      TestUtils.waitFor(function() { return gotEffectCancel; })
      verify(gotEffectCancel);
    }
  }
}
