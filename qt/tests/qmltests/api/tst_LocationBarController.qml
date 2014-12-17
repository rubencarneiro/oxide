import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.5
import com.canonical.Oxide.Testing 1.0

Item {
  id: top
  width: 200
  height: 200

  TestWebContext {
    id: testContext
  }

  Component {
    id: webViewFactory
    TestWebView {
      id: webView
      anchors.fill: parent
      context: testContext

      Rectangle {
        id: locationBar
        anchors.left: parent.left
        anchors.right: parent.right
        height: webView.locationBarController.maxHeight + webView.locationBarController.offset
        color: "black"
      }

      locationBarController.height: locationBar.height

      property var newViewRequestedDelegate: null
      onNewViewRequested: {
        newViewRequestedDelegate(request);
      }
    }
  }

  SignalSpy {
    id: spy
  }

  Item {
    id: locationBarSpy
    visible: false

    property var target: null

    readonly property alias unbalancedSignalsReceived: locationBarSpy.qtest_unbalancedSignalsReceived
    readonly property alias inconsistentPropertiesSeen: locationBarSpy.qtest_inconsistentPropertiesSeen
    readonly property alias shown: locationBarSpy.qtest_shown
    readonly property alias hidden: locationBarSpy.qtest_hidden
    readonly property alias animating: locationBarSpy.qtest_animating
    readonly property alias animationCount: locationBarSpy.qtest_animationCount
    readonly property alias lastAnimationDuration: locationBarSpy.qtest_lastAnimationDuration

    function clear() {
      qtest_waitingForNewContentOffset = false;
      qtest_unbalancedSignalsReceived = false;
      qtest_inconsistentPropertiesSeen = false;
      qtest_hadGoodUpdate = false;
      qtest_animationStartTime = null;
      qtest_lastAnimationDuration = null;

      qtest_update();

      qtest_animationCount = 0;
    }

    function waitUntilShown() {
      return qtest_waitUntil(function() { return qtest_shown; });
    }

    function waitUntilHidden() {
      return qtest_waitUntil(function() { return qtest_hidden; });
    }

    function waitForUpdate() {
      return qtest_waitUntil(function() { return qtest_hadGoodUpdate; });
    }

    property bool qtest_waitingForNewContentOffset: false
    property bool qtest_unbalancedSignalsReceived: false
    property bool qtest_inconsistentPropertiesSeen: false
    property bool qtest_hadGoodUpdate: false
    property bool qtest_shown: false
    property bool qtest_hidden: false
    property bool qtest_animating: false

    function qtest_update() {
      if (!target) {
        return;
      }

      var l = target.locationBarController;

      var good = Math.abs(l.contentOffset - l.offset - l.maxHeight) <= 1;
      qtest_hadGoodUpdate |= good;
      if (qtest_hadGoodUpdate) {
        qtest_inconsistentPropertiesSeen |= !good
      }

      qtest_shown = l.offset == 0 && l.maxHeight > 0;
      qtest_hidden = l.contentOffset == 0 && l.maxHeight > 0;
      qtest_animating = l.contentOffset > 0 && l.offset < 0;
    }

    property var qtest_prevTarget: null
    onTargetChanged: {
      if (qtest_prevTarget) {
        var l = qtest_prevTarget.locationBarController;
        l.offsetChanged.disconnect(qtest_offsetChanged);
        l.contentOffsetChanged.disconnect(qtest_contentOffsetChanged);
        qtest_prevTarget = null;
      }

      qtest_prevTarget = target;

      if (target) {
        var l = target.locationBarController;
        l.offsetChanged.connect(qtest_offsetChanged);
        l.contentOffsetChanged.connect(qtest_contentOffsetChanged);
      }

      qtest_update();
    }

    function qtest_offsetChanged() {
      if (qtest_waitingForNewContentOffset) {
        qtest_unbalancedSignalsReceived = true;
      }

      qtest_waitingForNewContentOffset = true;
    }

    function qtest_contentOffsetChanged() {
      if (!qtest_waitingForNewContentOffset) {
        qtest_unbalancedSignalsReceived = true;
      }

      qtest_waitingForNewContentOffset = false;

      qtest_update();
    }

    property var qtest_animationStartTime: null
    property int qtest_animationCount: 0
    property var qtest_lastAnimationDuration: null

    onQtest_animatingChanged: {
      if (!qtest_animating && qtest_animationStartTime) {
        qtest_animationCount++;
        qtest_lastAnimationDuration = Date.now() - qtest_animationStartTime;
        qtest_animationStartTime = null;
      } else if (qtest_animating && !qtest_animationStartTime) {
        qtest_animationStartTime = Date.now();
      }
    }

    function qtest_waitUntil(predicate) {
      var end = Date.now() + 5000;
      var i = Date.now();
      while (i < end && !predicate()) {
        qtest_testResult.wait(50);
        i = Date.now();
      }
      return predicate();
    }

    TestResult { id: qtest_testResult }
  }

  TestCase {
    id: test
    name: "LocationBarController"
    when: windowShown

    TestResult { id: testResult }

    function deleteWebView(webview) {
      var obs = OxideTestingUtils.createDestructionObserver(webview);
      webview.destroy();
      var end = Date.now() + 5000;
      var i = Date.now();
      while (i < end && !obs.destroyed) {
        testResult.wait(50);
        gc();
        i = Date.now();
      }
      verify(obs.destroyed);
    }

    function init() {
      spy.target = null;
      spy.signalName = "";
      spy.clear();

      locationBarSpy.target = null;
      locationBarSpy.clear();
    }

    // Ensure that the default are as expected
    function test_LocationBarController1_defaults() {
      var webView = webViewFactory.createObject(top, {});
      compare(webView.locationBarController.maxHeight, 0,
              "Default maxHeight should be 0");
      compare(webView.locationBarController.mode, LocationBarController.ModeAuto,
              "Default mode should be auto");
      compare(webView.locationBarController.offset, 0,
              "Default offset should be 0");
      compare(webView.locationBarController.contentOffset, 0,
              "Default contentOffset should be 0");

      deleteWebView(webView);
    }

    // Ensure that maxHeight cannot be changed after construction, and check
    // that it can't be set to an invalid value
    function test_LocationBarController2_maxHeight() {
      spy.signalName = "maxHeightChanged";

      var webView = webViewFactory.createObject(top, {});
      spy.target = webView.locationBarController;

      webView.locationBarController.maxHeight = "80";

      compare(webView.locationBarController.maxHeight, 0);
      compare(spy.count, 0);

      deleteWebView(webView);

      webView = webViewFactory.createObject(top, { "locationBarController.maxHeight": -80 });
      compare(webView.locationBarController.maxHeight, 0);

      deleteWebView(webView);
    }

    // Ensure that changing the mode has no effect when not in use
    function test_LocationBarController3_off() {
      var webView = webViewFactory.createObject(top, {});
      locationBarSpy.target = webView;
      spy.target = webView.locationBarController;
      spy.signalName = "modeChanged";

      webView.url = "http://testsuite/tst_LocationBarController.html";
      verify(webView.waitForLoadSucceeded());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.unbalancedSignalsReceived);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(webView.locationBarController.offset, 0);
      compare(webView.locationBarController.contentOffset, 0);

      webView.locationBarController.mode = LocationBarController.ModeShown;
      compare(spy.count, 0);
      compare(webView.locationBarController.mode, LocationBarController.ModeAuto);

      webView.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.unbalancedSignalsReceived);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(webView.locationBarController.offset, 0);
      compare(webView.locationBarController.contentOffset, 0);
    }

    // Ensure that changing the mode does have an effect when in use
    function test_LocationBarController4_on() {
      var webView = webViewFactory.createObject(top, { "locationBarController.maxHeight": 60 });
      locationBarSpy.target = webView;
      spy.target = webView.locationBarController;
      spy.signalName = "modeChanged";

      webView.url = "http://testsuite/tst_LocationBarController.html";
      verify(webView.waitForLoadSucceeded());

      verify(locationBarSpy.waitForUpdate());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.unbalancedSignalsReceived);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      webView.locationBarController.mode = LocationBarController.ModeHidden;
      compare(spy.count, 1);
      compare(webView.locationBarController.mode, LocationBarController.ModeHidden);

      verify(locationBarSpy.waitUntilHidden());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.unbalancedSignalsReceived);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 1);
      verify(locationBarSpy.lastAnimationDuration > 150 &&
             locationBarSpy.lastAnimationDuration < 250);

      webView.locationBarController.mode = LocationBarController.ModeShown;
      compare(spy.count, 2);
      compare(webView.locationBarController.mode, LocationBarController.ModeShown);

      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.unbalancedSignalsReceived);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 2);
      verify(locationBarSpy.lastAnimationDuration > 150 &&
             locationBarSpy.lastAnimationDuration < 250);

      deleteWebView(webView);
    }

    // Ensure that maxHeight is inherited by script opened webviews
    function test_LocationBarController5_script_opened() {
      var webView = webViewFactory.createObject(top, { "locationBarController.maxHeight": 60 });

      var created;
      function newViewRequestedDelegate(request) {
        created = webViewFactory.createObject(null, { "request": request, "locationBarController.maxHeight": 100 });
      }
      webView.newViewRequestedDelegate = newViewRequestedDelegate;

      webView.context.popupBlockerEnabled = false;
      webView.url = "http://testsuite/empty.html";
      verify(webView.waitForLoadSucceeded());

      webView.getTestApi().evaluateCode("window.open(\"tst_LocationBarController.html\");", true);

      verify(created);
      compare(created.locationBarController.maxHeight, 60);

      deleteWebView(created);
      deleteWebView(webView);
    }
  }
}
