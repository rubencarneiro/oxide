import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  SignalSpy {
    id: spy
    target: webView
    signalName: "preferencesChanged"
  }

  Component {
    id: webPreferences
    WebPreferences {}
  }

  TestCase {
    id: test
    name: "WebView_preferences"
    when: windowShown

    function test_WebView_preferences() {
      verify(webView.preferences, "No default preferences");
      compare(spy.count, 0,
              "Constructing the default shouldn't have caused a signal");
      compare(OxideTestingUtils.qObjectParent(webView.preferences), webView,
              "WebView should own its default preferences");

      var destructionObserver = OxideTestingUtils.createDestructionObserver(
          webView.preferences);
      var newPrefs = webPreferences.createObject(null, {});
      webView.preferences = newPrefs;

      compare(spy.count, 1,
              "Assigning a new preference object should have caused a signal");
      compare(webView.preferences, newPrefs,
              "Should have a new preference object");
      compare(OxideTestingUtils.qObjectParent(webView.preferences), webView,
              "WebView should own its default preferences");
      verify(destructionObserver.destroyed,
             "The default preference object should have been destroyed");

      var oldSetting = webView.preferences.javascriptEnabled = !webView.preferences.javascriptEnabled;
      OxideTestingUtils.destroyQObjectNow(webView.preferences);

      compare(spy.count, 2,
              "Deleting our preference object should have caused a signal");
      verify(webView.preferences, "Should have default preferences again");
      compare(webView.preferences.javascriptEnabled, !oldSetting,
              "We still have the old preferences object");
    }
  }
}
