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

    function test_Vibration() {
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      var mockApi;

      // Prime QtFeedback so that it creates our mock backend
      webView.getTestApi().evaluateCode("navigator.vibrate(1000);");
      TestUtils.waitFor(function() {
        mockApi = TestSupport.getAppProperty("_oxide_feedback_mock_api");
        return !!mockApi;
      });

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
  }
}
