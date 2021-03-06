import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  SignalSpy {
    id: spy
    target: webView
    signalName: "preferencesChanged"
  }

  Component {
    id: webPreferencesFactory
    WebPreferences {}
  }

  Component {
    id: webViewFactoryImpl1
    TestWebView {}
  }

  Component {
    id: webViewFactoryImpl2
    TestWebView {
      preferences.canRunInsecureContent: false
    }
  }

  property var webViewFactory: null
  property var created: null

  onNewViewRequested: {
    created = webViewFactory.createObject(null, {request: request});
  }

  TestCase {
    id: test
    name: "WebView_preferences"
    when: windowShown

    function init() {
      webView.preferences = null;
      spy.clear();
      if (created) {
        created.destroy();
        created = null;
      }
    }

    function cleanupTestCase() {
      webView.context.popupBlockerEnabled = true;
    }

    // Test we get a preference object by default, that it has the correct
    // ownership and gets destroyed when we assign a new preference object
    function test_WebView_preferences1() {
      verify(webView.preferences, "No default preferences");
      compare(spy.count, 0,
              "Constructing the default shouldn't have caused a signal");

      var prefHelper = TestSupport.createQObjectTestHelper(webView.preferences);
      compare(prefHelper.parent, webView, "WebView should own its default preferences");

      var newPrefs = webPreferencesFactory.createObject(null, {});
      webView.preferences = newPrefs;

      compare(spy.count, 1,
              "Assigning a new preference object should have caused a signal");
      compare(webView.preferences, newPrefs,
              "Should have a new preference object");
      compare(TestSupport.qObjectParent(webView.preferences), webView,
              "WebView should own the new preferences");
      verify(prefHelper.destroyed,
             "The default preference object should have been destroyed");
    }

    // Test that we can share preferences with another webview, and that
    // we handle the owning webview being deleted. Ideally, preferences would
    // be properly shared, but QML requires objects to have a parent if we
    // don't want them collected, so, bah
    function test_WebView_preferences2() {
      var webview2 = webViewFactoryImpl1.createObject(null, {});
      webView.preferences = webview2.preferences;

      compare(spy.count, 1,
              "Assigning a new preference object should have caused a signal");
      compare(webView.preferences, webview2.preferences,
              "Wrong preference object");

      compare(TestSupport.qObjectParent(webView.preferences), webview2,
              "WebView shouldn't have adopted preferences");

      var oldSetting = webView.preferences.javascriptEnabled = !webView.preferences.javascriptEnabled;
      TestSupport.destroyQObjectNow(webview2);

      compare(spy.count, 2, "Preferences should have been destroyed");
      verify(webView.preferences);
      compare(TestSupport.qObjectParent(webView.preferences), webView,
              "WebView should own the replacement preferences");
      compare(webView.preferences.javascriptEnabled, !oldSetting,
              "Expected default preference");
    }

    // Test that when we share preferences with another view and then delete it,
    // the preferences stay intact
    function test_WebView_preferences3() {
      var webview2 = webViewFactoryImpl1.createObject(null, {preferences: webView.preferences});

      compare(webView.preferences, webview2.preferences);
      compare(TestSupport.qObjectParent(webview2.preferences), webView);

      var oldSetting = webView.preferences.javascriptEnabled = !webView.preferences.javascriptEnabled;
      TestSupport.destroyQObjectNow(webview2);

      compare(spy.count, 0, "Preferences should not have been destroyed");
      compare(webView.preferences.javascriptEnabled, oldSetting,
              "Expected the settings to not change");
    }

    // Test that doing "WebView.preferences = null" results in the default preferences
    // being restored
    function test_WebView_preferences4() {
      var oldSetting = webView.preferences.javascriptEnabled = !webView.preferences.javascriptEnabled;
      webView.preferences = null;

      compare(spy.count, 1);
      compare(webView.preferences.javascriptEnabled, !oldSetting);
      compare(TestSupport.qObjectParent(webView.preferences), webView);
    }

    // Test that new webviews created via WebView.newViewRequested get a clone of the
    // opener preferences if no others are provided
    function test_WebView_preferences5() {
      webView.webViewFactory = webViewFactoryImpl1;
      webView.context.popupBlockerEnabled = false;

      webView.preferences.allowScriptsToCloseWindows = !webView.preferences.allowScriptsToCloseWindows;
      webView.preferences.canDisplayInsecureContent = !webView.preferences.canDisplayInsecureContent;
      webView.preferences.localStorageEnabled = !webView.preferences.localStorageEnabled;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.open(\"empty.html\");", true);
      TestUtils.waitFor(function() { return webView.created != null; });

      webView.context.popupBlockerEnabled = true;

      verify(webView.preferences != created.preferences);
      compare(created.preferences.allowScriptsToCloseWindows,
              webView.preferences.allowScriptsToCloseWindows);
      compare(created.preferences.canDisplayInsecureContent,
              webView.preferences.canDisplayInsecureContent);
      compare(created.preferences.localStorageEnabled,
              webView.preferences.localStorageEnabled);
      compare(TestSupport.qObjectParent(created.preferences), created,
              "WebView should own its default preferences");
    }

    function test_WebView_preferences6() {
      webView.webViewFactory = webViewFactoryImpl2;
      webView.context.popupBlockerEnabled = false;

      webView.preferences.allowScriptsToCloseWindows = !webView.preferences.allowScriptsToCloseWindows;
      webView.preferences.canDisplayInsecureContent = !webView.preferences.canDisplayInsecureContent;
      webView.preferences.localStorageEnabled = !webView.preferences.localStorageEnabled;

      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.open(\"empty.html\");", true);
      TestUtils.waitFor(function() { return webView.created != null; });

      webView.context.popupBlockerEnabled = true;

      verify(webView.preferences != created.preferences);
      verify(created.preferences.allowScriptsToCloseWidows !=
             webView.preferences.allowScriptsToCloseWindows);
      verify(created.preferences.allowScriptsToCloseWindows !=
             webView.preferences.allowScriptsToCloseWindows);
      verify(created.preferences.canDisplayInsecureContent !=
             webView.preferences.canDisplayInsecureContent);
      verify(created.preferences.localStorageEnabled !=
             webView.preferences.localStorageEnabled);
      compare(TestSupport.qObjectParent(created.preferences), created,
              "WebView should own its default preferences");

    }
  }
}
