import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.4
import com.canonical.Oxide.Testing 1.0 as Testing
import "TestUtils.js" as TestUtils

WebView {
  id: webView

  readonly property alias loadsStartedCount: webView.qtest_loadsStartedCount
  readonly property alias loadsSucceededCount: webView.qtest_loadsSucceededCount
  readonly property alias loadsFailedCount: webView.qtest_loadsFailedCount
  readonly property alias loadsStoppedCount: webView.qtest_loadsStoppedCount

  function clearLoadEventCounters() {
    qtest_loadsStartedCount = 0;
    qtest_loadsSucceededCount = 0;
    qtest_loadsFailedCount = 0;
    qtest_loadsStoppedCount = 0;

    qtest_expectedLoadsStartedCount = 0;
    qtest_expectedLoadsSucceededCount = 0;
    qtest_expectedLoadsFailedCount = 0;
    qtest_expectedLoadsStoppedCount = 0;
  }

  function getTestApi() {
    return new TestUtils.TestApiHost(this, rootFrame);
  }

  function getTestApiForFrame(frame) {
    return new TestUtils.TestApiHost(this, frame);
  }

  function waitForLoadStarted(timeout) {
    var expected = ++qtest_expectedLoadsStartedCount;
    return waitFor(
        function() { return expected == qtest_loadsStartedCount; },
        timeout);
  }

  function waitForLoadSucceeded(timeout) {
    var expected = ++qtest_expectedLoadsSucceededCount;
    return waitFor(
        function() { return expected == qtest_loadsSucceededCount; },
        timeout);
  }

  function waitForLoadStopped(timeout, gcDuringWait) {
    var expected = ++qtest_expectedLoadsStoppedCount;
    return waitFor(
        function() { return expected == qtest_loadsStoppedCount; },
        timeout, gcDuringWait);
  }

  function waitForLoadFailed(timeout) {
    var expected = ++qtest_expectedLoadsFailedCount;
    return waitFor(
        function() { return expected == qtest_loadsFailedCount; },
        timeout);
  }

  function waitFor(predicate, timeout, gcDuringWait) {
    timeout = timeout || 5000;
    var end = Date.now() + timeout;
    var i = Date.now();
    while (i < end && !predicate()) {
      qtest_testResult.wait(50);
      if (gcDuringWait) gc();
      i = Date.now();
    }
    return predicate();
  }

  property int qtest_loadsStartedCount: 0
  property int qtest_loadsSucceededCount: 0
  property int qtest_loadsFailedCount: 0
  property int qtest_loadsStoppedCount: 0

  property int qtest_expectedLoadsStartedCount: 0
  property int qtest_expectedLoadsSucceededCount: 0
  property int qtest_expectedLoadsFailedCount: 0
  property int qtest_expectedLoadsStoppedCount: 0

  context: TestWebContext {}

  onLoadEvent: {
    if (event.type == LoadEvent.TypeStarted) {
      webView.qtest_loadsStartedCount++;
    } else if (event.type == LoadEvent.TypeSucceeded) {
      webView.qtest_loadsSucceededCount++;
    } else if (event.type == LoadEvent.TypeStopped) {
      webView.qtest_loadsStoppedCount++;
    } else if (event.type == LoadEvent.TypeFailed) {
      webView.qtest_loadsFailedCount++;
    }

    on_load_event(event);
  }

  function on_load_event(event) {}

  TestResult { id: qtest_testResult }
}
