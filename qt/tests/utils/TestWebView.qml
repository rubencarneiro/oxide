import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import "TestUtils.js" as TestUtils

WebView {
  property var testApi: null
  property var waitingFor: null

  property var loadsStartedCount: 0
  property var loadsSucceededCount: 0
  property var loadsFailedCount: 0
  property var loadsStoppedCount: 0

  function resetLoadCounters() {
    loadsStartedCount = 0;
    loadsSucceededCount = 0;
    loadsFailedCount = 0;
    loadsStoppedCount = 0;
  }

  function getTestApi() {
    if (!testApi) {
      testApi = new TestUtils.TestApiHost(this);
    }
    return testApi;
  }

  function waitForLoadStarted() {
    waitingFor = LoadStatus.LoadStatusStarted;
    var success = waitFor(function() { return waitingFor === null; });
    waitingFor = null;

    return success;
  }

  function waitForLoadSucceeded() {
    waitingFor = LoadStatus.LoadStatusSucceeded;
    var success = waitFor(function() { return waitingFor === null; });
    waitingFor = null;

    return success;
  }

  function waitFor(predicate, timeout) {
    timeout = timeout || 5000;
    var i = 0;
    while (i < timeout && !predicate()) {
      testResult.wait(50);
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

  onLoadingChanged: {
    if (loadStatus.status == waitingFor) {
      waitingFor = null;
    }

    if (loadStatus.status == LoadStatus.LoadStatusStarted) {
      loadsStartedCount++;
    } else if (loadStatus.status == LoadStatus.LoadStatusSucceeded) {
      loadsSucceededCount++;
    } else if (loadStatus.status == LoadStatus.LoadStatusStopped) {
      loadsStoppedCount++;
    } else if (loadStatus.status == LoadStatus.LoadStatusFailed) {
      loadsFailedCount++;
    }
  }

  TestResult { id: testResult }
}
