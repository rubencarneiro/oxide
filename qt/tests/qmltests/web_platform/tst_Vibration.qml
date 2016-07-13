import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

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

      function effectStarted(id, duration, intensity) {
        effectRan = true;
        effectDuration = duration;
        effectIntensity = intensity;
      }
      mockApi.effectStarted.connect(effectStarted);

      webView.getTestApi().evaluateCode("navigator.vibrate(2000);");

      TestUtils.waitFor(function() { return effectRan; });
      compare(effectDuration, 2000);
      compare(effectIntensity, 1.0);

      mockApi.effectStarted.disconnect(effectStarted);
    }

    function test_Vibration2_sanitize() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      var mockApi = _getMockApi();

      var effectRan = false;
      var effectDuration;
      var effectIntensity;

      function effectStarted(id, duration, intensity) {
        effectRan = true;
        effectDuration = duration;
        effectIntensity = intensity;
      }
      mockApi.effectStarted.connect(effectStarted);

      webView.getTestApi().evaluateCode("navigator.vibrate(200000000);");

      TestUtils.waitFor(function() { return effectRan; });
      compare(effectDuration, 10000);
      compare(effectIntensity, 1.0);

      mockApi.effectStarted.disconnect(effectStarted);
    }

    function test_Vibration3_cancel() {
      webView.url = "http://testsuite/tst_Vibration_subframe.html";
      verify(webView.waitForLoadSucceeded());

      var mockApi = _getMockApi();

      var effectId;
      var gotEffectCancel = false;

      function effectStarted(id, duration, intensity) {
        effectId = id;
      }
      function effectStopped(id) {
        if (id == effectId) {
          gotEffectCancel = true;
        }
      }
      mockApi.effectStarted.connect(effectStarted);
      mockApi.effectStopped.connect(effectStopped);

      var frame = webView.rootFrame.childFrames[0];
      webView.getTestApiForFrame(frame).evaluateCode("navigator.vibrate(10000);");
      webView.getTestApi().evaluateCode("
var e = document.getElementById(\"subframe\");
e.parentNode.removeChild(e);");

      TestUtils.waitFor(function() { return gotEffectCancel; })
      verify(gotEffectCancel);

      mockApi.effectStarted.disconnect(effectStarted);
      mockApi.effectStopped.disconnect(effectStopped);
    }

    function test_Vibration4_pattern() {
      webView.url = "http://testsuite/tst_Vibration_subframe.html";
      verify(webView.waitForLoadSucceeded());

      var mockApi = _getMockApi();

      var effectSequence = [];

      function effectStarted(id, duration, intensity) {
        effectSequence.push({ duration: duration, intensity: intensity });
      }
      mockApi.effectStarted.connect(effectStarted);

      var startTime = Date.now();
      webView.getTestApi().evaluateCode("navigator.vibrate([100, 300, 200, 500, 400]);");

      TestUtils.waitFor(function() { return effectSequence.length == 3; });
      fuzzyCompare(Date.now() - startTime, 1100, 150);
      compare(effectSequence[0].duration, 100);
      compare(effectSequence[0].intensity, 1.0);
      compare(effectSequence[1].duration, 200);
      compare(effectSequence[1].intensity, 1.0);
      compare(effectSequence[2].duration, 400);
      compare(effectSequence[2].intensity, 1.0);

      mockApi.effectStarted.disconnect(effectStarted);
    }
  }
}
