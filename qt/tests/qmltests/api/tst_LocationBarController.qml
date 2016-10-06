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

  Connections {
    property var lastError: null

    id: certificateErrorSink
    onCertificateError: lastError = error;
  }

  Item {
    id: locationBarSpy
    visible: false

    property var target: null

    readonly property alias missingSignal: locationBarSpy.qtest_missingSignal
    readonly property alias inconsistentPropertiesSeen: locationBarSpy.qtest_inconsistentPropertiesSeen
    readonly property alias shown: locationBarSpy.qtest_shown
    readonly property alias hidden: locationBarSpy.qtest_hidden
    readonly property alias transitionCount: locationBarSpy.qtest_transitionCount
    readonly property alias animating: locationBarSpy.qtest_animating
    readonly property alias animationCount: locationBarSpy.qtest_animationCount
    readonly property alias lastAnimationDuration: locationBarSpy.qtest_lastAnimationDuration

    function clear() {
      qtest_waitingForNewContentOffset = false;
      qtest_missingSignal = false;
      qtest_inconsistentPropertiesSeen = false;
      qtest_animationStartTime = null;
      qtest_lastAnimationDuration = null;
      qtest_shown = false;
      qtest_hidden = false;
      qtest_animating = false;

      qtest_update();

      qtest_wasShown = qtest_shown;
      qtest_wasHidden = qtest_hidden;
      qtest_animationCount = 0;
      qtest_transitionCount = 0;
    }

    function waitUntilShown(timeout) {
      return TestUtils.waitFor(function() { return qtest_shown; }, timeout);
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

    property int qtest_transitionCount: 0
    property bool qtest_wasShown: false
    property bool qtest_wasHidden: false

    function qtest_updateTransitionCount() {
      if (qtest_shown) {
        if (!qtest_wasShown) {
          qtest_transitionCount++;
        }
        qtest_wasShown = true;
        qtest_wasHidden = false;
      } else if (qtest_hidden) {
        if (!qtest_wasHidden) {
          qtest_transitionCount++;
        }
        qtest_wasHidden = true;
        qtest_wasShown = false;
      }
    }
    onQtest_shownChanged: {
      qtest_updateTransitionCount();
    }

    onQtest_hiddenChanged: {
      qtest_updateTransitionCount();
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

      certificateErrorSink.target = null;
      certificateErrorSink.lastError = null;
    }

    // Ensure that the defaults are as expected
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

      TestSupport.destroyQObjectNow(webView);
    }

    // Ensure that changing the mode and calling show() / hide() has no effect
    // when not in use (height == 0)
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
      compare(locationBarSpy.transitionCount, 0);
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
      compare(locationBarSpy.transitionCount, 0);
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
      compare(locationBarSpy.transitionCount, 0);
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
      compare(locationBarSpy.transitionCount, 0);
      compare(webView.locationBarController.offset, 0);
      compare(webView.locationBarController.contentOffset, 0);

      webView.locationBarController.hide(true);

      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(locationBarSpy.transitionCount, 0);
      compare(webView.locationBarController.offset, 0);
      compare(webView.locationBarController.contentOffset, 0);

      TestSupport.destroyQObjectNow(webView);
    }

    function test_LocationBarController4_initial_shown_data() {
      return [
        { mode: LocationBarController.ModeAuto },
        { mode: LocationBarController.ModeShown }
      ];
    }

    // Test that the API initializes to the correct value before the
    // webview is used (https://launchpad.net/bugs/1625484)
    function test_LocationBarController4_initial_shown() {
      var webView = webViewFactory.createObject(top, {
          "locationBarController.height": 60,
          "locationBarController.animated": true,
          "locationBarController.mode": data.mode
      });

      locationBarSpy.target = webView;

      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      // This is 2 because we get a transition:
      // - When we set the spy target
      // - When the locationbar is shown (it's initially hidden until the
      //   webview compositor produces a frame)
      compare(locationBarSpy.transitionCount, 2);

      TestSupport.destroyQObjectNow(webView);
    }

    // Test that the API initializes to the correct value before the
    // webview is used (https://launchpad.net/bugs/1625484)
    function test_LocationBarController5_initial_hidden() {
      var webView = webViewFactory.createObject(top, {
          "locationBarController.height": 60,
          "locationBarController.animated": true,
          "locationBarController.mode": LocationBarController.ModeHidden
      });

      locationBarSpy.target = webView;

      verify(locationBarSpy.waitUntilHidden());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(locationBarSpy.transitionCount, 1);

      TestSupport.destroyQObjectNow(webView);
    }

    function test_LocationBarController6_mode_on_data() {
      return [
        { animated: true },
        { animated: false }
      ];
    }

    // Ensure that changing the mode does have an effect when in use
    // (height > 0)
    function test_LocationBarController6_mode_on(data) {
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
      compare(locationBarSpy.transitionCount, 2);

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
      compare(locationBarSpy.transitionCount, 3);

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
      compare(locationBarSpy.transitionCount, 4);

      TestSupport.destroyQObjectNow(webView);
    }

    function test_LocationBarController7_on_show_data() {
      return [
        { animated: true },
        { animated: false }
      ];
    }

    // Test that show() behaves as expected
    function test_LocationBarController7_on_show(data) {
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
      compare(locationBarSpy.transitionCount, 1);

      webView.locationBarController.show(data.animated);
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(locationBarSpy.transitionCount, 1);

      webView.locationBarController.mode = LocationBarController.ModeAuto;
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(locationBarSpy.transitionCount, 1);

      webView.locationBarController.show(data.animated);
      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, data.animated ? 1 : 0);
      compare(locationBarSpy.transitionCount, 2);

      TestSupport.destroyQObjectNow(webView);
    }

    function test_LocationBarController8_on_hide_data() {
      return [
        { animated: true },
        { animated: false }
      ];
    }

    // Test that show() behaves as expected
    function test_LocationBarController8_on_hide(data) {
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
      compare(locationBarSpy.transitionCount, 2);

      webView.locationBarController.hide(data.animated);
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(locationBarSpy.transitionCount, 2);

      webView.locationBarController.mode = LocationBarController.ModeAuto;
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(locationBarSpy.transitionCount, 2);

      webView.locationBarController.hide(data.animated);
      verify(locationBarSpy.waitUntilHidden());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, data.animated ? 1 : 0);
      compare(locationBarSpy.transitionCount, 3);

      TestSupport.destroyQObjectNow(webView);
    }

    function test_LocationBarController9_show_blockers_data() {
      function _test_1_init(webView) {
        spy.target = webView;
        spy.signalName = "fullscreenRequested";

        webView.url = "http://testsuite/tst_LocationBarController_fullscreen.html";
        verify(webView.waitForLoadSucceeded());
        verify(locationBarSpy.waitUntilShown());

        var r = webView.getTestApi().getBoundingClientRectForSelector("#button");
        mouseClick(webView, r.x + r.width / 2, r.y + 60 + r.height / 2, Qt.LeftButton);
        spy.wait();

        webView.fullscreen = true;
      }
      function _test_1_post(webView) {
        webView.fullscreen = false;
      }

      return [
        { initial: LocationBarController.ModeShown, init: _test_1_init, post: _test_1_post },
        { initial: LocationBarController.ModeAuto, init: _test_1_init, post: _test_1_post }
      ];
    }

    function test_LocationBarController9_show_blockers(data) {
      var webView = webViewFactory.createObject(top, {
          "locationBarController.height": 60,
          "locationBarController.mode": data.initial
      });

      locationBarSpy.target = webView;

      data.init(webView);

      verify(locationBarSpy.waitUntilHidden());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      locationBarSpy.clear();

      webView.locationBarController.show(false);
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(locationBarSpy.transitionCount, 0);

      data.post(webView);

      webView.locationBarController.show(false);
      verify(locationBarSpy.waitUntilShown());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      TestSupport.destroyQObjectNow(webView);
    }

    function test_LocationBarController10_hide_blockers_data() {
      // Degraded security level
      function _test_1_init(webView) {
        webView.url = "https://testsuite/tst_LocationBarController_display_insecure.html";
        verify(webView.waitForLoadSucceeded());
        compare(webView.securityStatus.securityLevel, SecurityStatus.SecurityLevelWarning);
      }
      function _test_1_post(webView) {
        webView.url = "https://testsuite/tst_LocationBarController.html";
        verify(webView.waitForLoadSucceeded());
      }

      // Security error
      function _test_2_init(webView) {
        webView.preferences.canRunInsecureContent = true;
        webView.url = "https://jkjfgvklfd.testsuite/tst_LocationBarController_run_insecure.html";
        verify(webView.waitForLoadSucceeded());
        compare(webView.securityStatus.securityLevel, SecurityStatus.SecurityLevelError);
      }
      function _test_2_post(webView) {
        webView.url = "https://foo.testsuite/tst_LocationBarController.html";
        verify(webView.waitForLoadSucceeded());
      }

      // Certificate error
      function _test_3_init(webView) {
        certificateErrorSink.target = webView;

        webView.url = "http://testsuite/tst_LocationBarController.html";
        verify(webView.waitForLoadSucceeded());
        webView.locationBarController.hide(false);
        verify(locationBarSpy.waitUntilHidden());

        webView.url = "https://expired.testsuite/tst_LocationBarController.html";
        TestUtils.waitFor(function() { return certificateErrorSink.lastError != null; });
      }
      function _test_3_post(webView) {
        certificateErrorSink.lastError.deny();
      }

      // Render process crash
      function _test_4_init(webView) {
        webView.url = "http://testsuite/tst_LocationBarController.html";
        verify(webView.waitForLoadSucceeded());
        webView.locationBarController.hide(false);
        verify(locationBarSpy.waitUntilHidden());

        webView.killWebProcess(true);
      }
      function _test_4_post(webView) {
        webView.reload();
        verify(webView.waitForLoadSucceeded());
      }

      // Render process hang
      function _test_5_init(webView) {
        webView.url = "http://testsuite/tst_LocationBarController.html";
        verify(webView.waitForLoadSucceeded());
        webView.locationBarController.hide(false);
        verify(locationBarSpy.waitUntilHidden());

        webView.url = "chrome://hang/";
        for (var i = 0; i < 100; i++) {
          keyClick("A");
          mouseClick(webView, webView.width / 2, webView.height / 2, Qt.LeftButton);
        }
      }
      function _test_5_post(webView) {
        webView.killWebProcess(false);
        TestSupport.wait(1000);
        webView.url = "http://testsuite/tst_LocationBarController.html";
        verify(webView.waitForLoadSucceeded());
      }

      var data = [
        { init: _test_1_init, post: _test_1_post },
        { init: _test_2_init, post: _test_2_post },
        // { init: _test_3_init, post: _test_3_post },
      ];
      if (Oxide.processModel != Oxide.ProcessModelSingleProcess) {
        data = data.concat([
          { init: _test_4_init, post: _test_4_post },
          { init: _test_5_init, post: _test_5_post },
        ]);;
      }
      return data;
    }

    // Test various conditions that prevent hiding of the locationbar when in
    // auto mode. We test this by calling hide(), but this also prevents
    // autohide on scrolling
    function test_LocationBarController10_hide_blockers(data) {
      var webView = webViewFactory.createObject(top, {
          "locationBarController.height": 60
      });

      locationBarSpy.target = webView;

      data.init(webView);

      verify(locationBarSpy.waitUntilShown(20000));

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      locationBarSpy.clear();

      webView.locationBarController.hide(false);
      TestSupport.wait(500);

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(locationBarSpy.shown);
      verify(!locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);
      compare(locationBarSpy.transitionCount, 0);

      data.post(webView);

      webView.locationBarController.hide(false);
      verify(locationBarSpy.waitUntilHidden());

      verify(!locationBarSpy.inconsistentPropertiesSeen);
      verify(!locationBarSpy.missingSignal);
      verify(!locationBarSpy.shown);
      verify(locationBarSpy.hidden);
      verify(!locationBarSpy.animating);
      compare(locationBarSpy.animationCount, 0);

      TestSupport.destroyQObjectNow(webView);
    }
  }
}
