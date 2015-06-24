import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import "TestUtils.js" as TestUtils

WebView {
  id: webView

  readonly property alias loadsStartedCount: webView.qtest_loadsStartedCount
  readonly property alias loadsSucceededCount: webView.qtest_loadsSucceededCount
  readonly property alias loadsFailedCount: webView.qtest_loadsFailedCount
  readonly property alias loadsStoppedCount: webView.qtest_loadsStoppedCount
  readonly property alias loadsCommittedCount: webView.qtest_loadsCommittedCount

  function clearLoadEventCounters() {
    qtest_loadsStartedCount = 0;
    qtest_loadsSucceededCount = 0;
    qtest_loadsFailedCount = 0;
    qtest_loadsStoppedCount = 0;
    qtest_loadsCommittedCount = 0;

    qtest_expectedLoadsStartedCount = 0;
    qtest_expectedLoadsSucceededCount = 0;
    qtest_expectedLoadsFailedCount = 0;
    qtest_expectedLoadsStoppedCount = 0;
    qtest_expectedLoadsCommittedCount = 0;
  }

  property var qtest_testApiHosts: new Object()

  function getTestApi() {
    if (!(rootFrame in qtest_testApiHosts)) {
      qtest_testApiHosts[rootFrame] = new TestUtils.TestApiHost(rootFrame);
    }

    return qtest_testApiHosts[rootFrame];
  }

  function getTestApiForFrame(frame) {
    if (!(frame in qtest_testApiHosts)) {
      qtest_testApiHosts[frame] = new TestUtils.TestApiHost(frame);
    }

    return qtest_testApiHosts[frame];
  }

  function waitForLoadStarted(timeout) {
    var expected = ++qtest_expectedLoadsStartedCount;
    return TestUtils.waitFor(
        function() { return expected == qtest_loadsStartedCount; },
        timeout);
  }

  function waitForLoadSucceeded(timeout) {
    var expected = ++qtest_expectedLoadsSucceededCount;
    return TestUtils.waitFor(
        function() { return expected == qtest_loadsSucceededCount; },
        timeout);
  }

  function waitForLoadStopped(timeout, gcDuringWait) {
    var expected = ++qtest_expectedLoadsStoppedCount;
    return TestUtils.waitFor(
        function() { return expected == qtest_loadsStoppedCount; },
        timeout, gcDuringWait);
  }

  function waitForLoadFailed(timeout) {
    var expected = ++qtest_expectedLoadsFailedCount;
    return TestUtils.waitFor(
        function() { return expected == qtest_loadsFailedCount; },
        timeout);
  }

  function waitForLoadCommitted(timeout) {
    var expected = ++qtest_expectedLoadsCommittedCount;
    return TestUtils.waitFor(
        function() { return expected == qtest_loadsCommittedCount; },
        timeout);
  }

  property int qtest_loadsStartedCount: 0
  property int qtest_loadsSucceededCount: 0
  property int qtest_loadsFailedCount: 0
  property int qtest_loadsStoppedCount: 0
  property int qtest_loadsCommittedCount: 0

  property int qtest_expectedLoadsStartedCount: 0
  property int qtest_expectedLoadsSucceededCount: 0
  property int qtest_expectedLoadsFailedCount: 0
  property int qtest_expectedLoadsStoppedCount: 0
  property int qtest_expectedLoadsCommittedCount: 0

  context: TestWebContext {}

  Connections {
    onFrameRemoved: {
      delete webView.qtest_testApiHosts[frame];
    }

    onLoadEvent: {
      if (event.type == LoadEvent.TypeStarted) {
        webView.qtest_loadsStartedCount++;
      } else if (event.type == LoadEvent.TypeSucceeded) {
        webView.qtest_loadsSucceededCount++;
      } else if (event.type == LoadEvent.TypeStopped) {
        webView.qtest_loadsStoppedCount++;
      } else if (event.type == LoadEvent.TypeFailed) {
        webView.qtest_loadsFailedCount++;
      } else if (event.type == LoadEvent.TypeCommitted) {
        webView.qtest_loadsCommittedCount++;
      }
    }
  }
}
