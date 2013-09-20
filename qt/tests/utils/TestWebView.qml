import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import "TestUtils.js" as TestUtils

WebView {
  property var _testApi: null

  property var loadsStartedCount: 0
  property var loadsSucceededCount: 0
  property var loadsFailedCount: 0
  property var loadsStoppedCount: 0
  property var _lastLoadingState: false

  function resetLoadCounters() {
    loadsStartedCount = 0;
    loadsSucceededCount = 0;
    loadsFailedCount = 0;
    loadsStoppedCount = 0;
  }

  function getTestApi() {
    if (!_testApi) {
      _testApi = new TestUtils.TestApiHost(this);
    }
    return _testApi;
  }

  function waitForLoadStarted(count) {
    if (count === undefined) {
      count = loadsStartedCount + 1;
    }
    return waitFor(function() { return loadsStartedCount == count; });
  }

  function waitForLoadSucceeded(count) {
    if (count === undefined) {
      count = loadsSucceededCount + 1;
    }
    return waitFor(function() { return loadsSucceededCount == count; });
  }

  function waitFor(predicate, timeout) {
    timeout = timeout || 5000;
    var i = 0;
    while (i < timeout && !predicate()) {
      _testResult.wait(50);
      i += 50;
    }
    return predicate();
  }

  context: WebViewContext {
    userScripts: [
      UserScript {
        worldId: "TestUtils"
        url: Qt.resolvedUrl("TestUtilsSlave.js")
        incognitoEnabled: true
      }
    ]
  }

  function loadingStateChanged() {}

  onLoadingChanged: {
    if (loadStatus.status == LoadStatus.LoadStatusStarted) {
      loadsStartedCount++;
    } else if (loadStatus.status == LoadStatus.LoadStatusSucceeded) {
      loadsSucceededCount++;
    } else if (loadStatus.status == LoadStatus.LoadStatusStopped) {
      loadsStoppedCount++;
    } else if (loadStatus.status == LoadStatus.LoadStatusFailed) {
      loadsFailedCount++;
    }

    if (loading != _lastLoadingState) {
      _lastLoadingState = loading;
      loadingStateChanged();
    }
  }

  Component.onCompleted: {
    _lastLoadingState = loading;
  }

  TestResult { id: _testResult }
}
