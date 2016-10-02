import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.5
import Oxide.testsupport 1.0

Item {
  id: top
  focus: true

  Component {
    id: webViewFactory
    TestWebView {
      id: webView
      anchors.fill: parent

      Rectangle {
        id: locationBar
        anchors.left: parent.left
        anchors.right: parent.right
        height: webView.locationBarController.height + webView.locationBarController.offset
        color: "black"
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

    readonly property alias missingSignal: locationBarSpy.qtest_missingSignal
    readonly property alias inconsistentPropertiesSeen: locationBarSpy.qtest_inconsistentPropertiesSeen
    readonly property alias shown: locationBarSpy.qtest_shown
    readonly property alias hidden: locationBarSpy.qtest_hidden
    readonly property alias animating: locationBarSpy.qtest_animating
    readonly property alias animationCount: locationBarSpy.qtest_animationCount
    readonly property alias lastAnimationDuration: locationBarSpy.qtest_lastAnimationDuration

    function clear() {
      qtest_waitingForNewContentOffset = false;
      qtest_missingSignal = false;
      qtest_inconsistentPropertiesSeen = false;
      qtest_animationStartTime = null;
      qtest_lastAnimationDuration = null;

      qtest_update();

      qtest_animationCount = 0;
    }

    function waitUntilShown() {
      return TestUtils.waitFor(function() { return qtest_shown; });
    }

    function waitUntilHidden() {
      return TestUtils.waitFor(function() { return qtest_hidden; });
    }

    property bool qtest_waitingForNewContentOffset: false
    property bool qtest_missingSignal: false
    property bool qtest_inconsistentPropertiesSeen: false
    property bool qtest_shown: false
    property bool qtest_hidden: false
    property bool qtest_animating: false

    function qtest_update() {
      if (!target) {
        return;
      }

      var l = target.locationBarController;

      var good = Math.abs(l.contentOffset - l.offset - l.height) <= 1;
      if (!good) {
        console.log("Bad properties, contentOffset: " + l.contentOffset + ", offset: " + l.offset + ", height: " + l.height);
      }
      qtest_inconsistentPropertiesSeen |= !good

      qtest_shown = l.offset == 0 && l.height > 0;
      qtest_hidden = l.contentOffset == 0 && l.height > 0;
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
        qtest_missingSignal = true;
      }

      qtest_waitingForNewContentOffset = true;

      qtest_update();
    }

    function qtest_contentOffsetChanged() {
      if (!qtest_waitingForNewContentOffset) {
        qtest_missingSignal = true;
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
  }

  TestCase {
    id: test
    name: "LocationBarController"
    when: windowShown

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
      compare(webView.locationBarController.height, 0,
              "Default height should be 0");
      compare(webView.locationBarController.mode, LocationBarController.ModeAuto,
              "Default mode should be auto");
      compare(webView.locationBarController.animated, true,
              "Should default to animated");
      compare(webView.locationBarController.offset, 0,
              "Default offset should be 0");
      compare(webView.locationBarController.contentOffset, 0,
              "Default contentOffset should be 0");

      TestSupport.destroyQObjectNow(webView);
    }

    // Ensure that height cannot be set to an invalid value, and verify the
    // notify signal works correctly
    function test_LocationBarController2_height() {
      spy.signalName = "heightChanged";

      var webView = webViewFactory.createObject(top, {});
      spy.target = webView.locationBarController;

      webView.locationBarController.height = 80;

      compare(webView.locationBarController.height, 80,
              "Height should be set correctly");
      compare(spy.count, 1, "Should have had a signal when setting the height");

      webView.locationBarController.height = -80;

      compare(webView.locationBarController.height, 80,
              "Setting an invalid height should be ignored");
      compare(spy.count, 1,
              "Setting an invalid height should result in no signal");

      webView.locationBarController.height = 0;

      compare(webView.locationBarController.height, 0,
              "Height should be set correctly");
      compare(spy.count, 2, "Should have had a signal when setting the height");
    }

    // Ensure that changing the mode and calling show() / hide() has no effect
    //  when not in use
    function test_LocationBarController3_mode_off() {
      var webView = webViewFactory.createObject(top, {});
      locationBarSpy.target = webView;
      spy.target = webView.locationBarController;
      spy.signalName = "modeChanged";

      webView.url = "http://testsuite/tst_LocationBarController.html";
      verify(webView.waitForLoadSucceeded());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(webView.locationBarController.offset, 0);
      compare(webView.locationBarController.contentOffset, 0);

      webView.locationBarController.mode = LocationBarController.ModeShown;
      compare(spy.count, 1);
      compare(webView.locationBarController.mode, LocationBarController.ModeShown);

      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(webView.locationBarController.offset, 0);
      compare(webView.locationBarController.contentOffset, 0);

      webView.locationBarController.mode = LocationBarController.ModeHidden;
      compare(spy.count, 2);
      compare(webView.locationBarController.mode, LocationBarController.ModeHidden);

      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(webView.locationBarController.offset, 0);
      compare(webView.locationBarController.contentOffset, 0);

      webView.locationBarController.mode = LocationBarController.ModeAuto;
      compare(spy.count, 3);
      compare(webView.locationBarController.mode, LocationBarController.ModeAuto);

      webView.locationBarController.show(true);

      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(webView.locationBarController.offset, 0);
      compare(webView.locationBarController.contentOffset, 0);

      webView.locationBarController.hide(true);

      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(webView.locationBarController.offset, 0);
      compare(webView.locationBarController.contentOffset, 0);
    }

    function test_LocationBarController4_mode_on_data() {
      return [
        { animated: true },
        { animated: false }
      ];
    }

    // Ensure that changing the mode does have an effect when in use
    function test_LocationBarController4_mode_on(data) {
      var webView = webViewFactory.createObject(top, {
          "locationBarController.height": 60,
          "locationBarController.animated": data.animated
      });

      locationBarSpy.target = webView;
      spy.target = webView.locationBarController;
      spy.signalName = "modeChanged";

      webView.url = "http://testsuite/tst_LocationBarController.html";
      verify(webView.waitForLoadSucceeded());

      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      webView.locationBarController.mode = LocationBarController.ModeHidden;
      compare(spy.count, 1);
      compare(webView.locationBarController.mode, LocationBarController.ModeHidden);

      verify(locationBarSpy.waitUntilHidden());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, data.animated ? 1 : 0);
      if (data.animated) {
        verify(locationBarSpy.lastAnimationDuration > 150 &&
               locationBarSpy.lastAnimationDuration < 250);
      }

      webView.locationBarController.mode = LocationBarController.ModeShown;
      compare(spy.count, 2);
      compare(webView.locationBarController.mode, LocationBarController.ModeShown);

      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, data.animated ? 2 : 0);
      if (data.animated) {
        verify(locationBarSpy.lastAnimationDuration > 150 &&
               locationBarSpy.lastAnimationDuration < 250);
      }

      TestSupport.destroyQObjectNow(webView);
    }

    function test_LocationBarController5_on_show_data() {
      return [
        { animated: true },
        { animated: false }
      ];
    }

    // Test that show() behaves as expected
    function test_LocationBarController5_on_show(data) {
      var webView = webViewFactory.createObject(top, {
          "locationBarController.height": 60,
          "locationBarController.animated": data.animated,
          "locationBarController.mode": LocationBarController.ModeHidden
      });

      locationBarSpy.target = webView;
      spy.target = webView.locationBarController;
      spy.signalName = "modeChanged";

      webView.url = "http://testsuite/tst_LocationBarController.html";
      verify(webView.waitForLoadSucceeded());

      verify(locationBarSpy.waitUntilHidden());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      webView.locationBarController.show(data.animated);
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      webView.locationBarController.mode = LocationBarController.ModeAuto;
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      webView.locationBarController.show(data.animated);
      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, data.animated ? 1 : 0);
    }

    function test_LocationBarController6_on_hide_data() {
      return [
        { animated: true },
        { animated: false }
      ];
    }

    // Test that show() behaves as expected
    function test_LocationBarController6_on_hide(data) {
      var webView = webViewFactory.createObject(top, {
          "locationBarController.height": 60,
          "locationBarController.animated": data.animated,
          "locationBarController.mode": LocationBarController.ModeShown
      });

      locationBarSpy.target = webView;
      spy.target = webView.locationBarController;
      spy.signalName = "modeChanged";

      webView.url = "http://testsuite/tst_LocationBarController.html";
      verify(webView.waitForLoadSucceeded());

      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      webView.locationBarController.hide(data.animated);
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      webView.locationBarController.mode = LocationBarController.ModeAuto;
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      webView.locationBarController.hide(data.animated);
      verify(locationBarSpy.waitUntilHidden());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, data.animated ? 1 : 0);
    }

    function test_LocationBarController7_fullscreen_data() {
      return [
        { initial: LocationBarController.ModeShown },
        { initial: LocationBarController.ModeAuto }
      ];
    }

    function test_LocationBarController7_fullscreen(data) {
      var webView = webViewFactory.createObject(top, {
          "locationBarController.height": 60,
          "locationBarController.animated": true,
          "locationBarController.mode": data.initial
      });

      locationBarSpy.target = webView;
      spy.target = webView;
      spy.signalName = "fullscreenRequested";

      webView.url = "http://testsuite/tst_LocationBarController_fullscreen.html";
      verify(webView.waitForLoadSucceeded());

      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
      mouseClick(webView, r.x + r.width / 2, r.y + 60 + r.height / 2, Qt.LeftButton);
      spy.wait();

      webView.fullscreen = true;

      verify(locationBarSpy.waitUntilHidden());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      webView.fullscreen = false;

      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
    }
  }
}
