import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  readonly property alias frameEvents: webView.qtest_frameEvents

  function clearFrameEvents() {
    qtest_frameEvents = [];
  }

  onFrameAdded: {
    console.log("Frame added: " + frame.toString());
    qtest_frameEvents.push({type: "added", frame: frame.toString()});
  }

  onFrameRemoved: {
    console.log("Frame removed: " + frame.toString());
    qtest_frameEvents.push({type: "removed", frame: frame.toString()});
  }

  property var qtest_frameEvents: []

  TestCase {
    id: test
    name: WebFrame_tree
    when: windowShown

    function init() {
      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      webView.clearFrameEvents();
    }

    function verify_events(data) {
      compare(webView.frameEvents.length, data.length, "Unexpected number of events");
      for (var i = 0; i < data.length; ++i) {
        if (i >= webView.frameEvents.length) {
          break;
        }
        compare(webView.frameEvents[i].type, data[i].type, "Unexpected event type");
        compare(webView.frameEvents[i].frame, data[i].frame, "Unexpected frame");
      }

      webView.clearFrameEvents();
    }

    function verify_tree() {
      compare(webView.rootFrame.parentFrame, null,
              "Root frame should have no parent");
      compare(Utils.qObjectParent(webView.rootFrame), webView,
              "Root frame should be a qobject child of the webview");

      var queue = [];
      queue.push(webView.rootFrame);

      while(queue.length > 0) {
        var frame = queue.shift();
        for (var i = 0; i < frame.childFrames.length; ++i) {
          compare(frame.childFrames[i].parentFrame, frame,
                  "Incorrect parent");
          compare(Utils.qObjectParent(frame.childFrames[i]), frame,
                  "Incorrect qobject parent");
          queue.push(frame.childFrames[i]);
        }
      }
    }

    function test_WebFrame_tree1_navigation() {
      webView.url = "http://localhost:8080/tst_WebFrame_tree1.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var frames1 = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString()
      ];

      verify_events([
        { type: "added", frame: frames1[0] },
        { type: "added", frame: frames1[1] }
      ]);
      verify_tree();

      webView.url = "http://localhost:8080/tst_WebFrame_tree2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var frames2 = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString(),
        webView.rootFrame.childFrames[0].childFrames[0].toString(),
        webView.rootFrame.childFrames[0].childFrames[1].toString(),
      ];

      verify_events([
        { type: "removed", frame: frames1[1] },
        { type: "removed", frame: frames1[0] },
        { type: "added", frame: frames2[0] },
        { type: "added", frame: frames2[1] },
        { type: "added", frame: frames2[2] },
        { type: "added", frame: frames2[3] }
      ]);
      verify_tree();

      webView.goBack();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      frames1 = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString()
      ];

      verify_events([
        { type: "removed", frame: frames2[1] },
        { type: "removed", frame: frames2[3] },
        { type: "removed", frame: frames2[2] },
        { type: "removed", frame: frames2[0] },
        { type: "added", frame: frames1[0] },
        { type: "added", frame: frames1[1] }
      ]);
      verify_tree();
    }

    function test_WebFrame_tree2_crossRVHNavigation() {
      webView.url = "http://localhost:8080/tst_WebFrame_tree2.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var frames = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString(),
        webView.rootFrame.childFrames[0].childFrames[0].toString(),
        webView.rootFrame.childFrames[0].childFrames[1].toString(),
      ];

      verify_events([
        { type: "added", frame: frames[0] },
        { type: "added", frame: frames[1] },
        { type: "added", frame: frames[2] },
        { type: "added", frame: frames[3] }
      ]);
      verify_tree();

      webView.url = "about:blank";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      verify_events([
        { type: "removed", frame: frames[2] },
        { type: "removed", frame: frames[3] },
        { type: "removed", frame: frames[0] },
        { type: "removed", frame: frames[1] }
      ]);
      verify_tree();

      webView.goBack();
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      frames = [
        webView.rootFrame.childFrames[0].toString(),
        webView.rootFrame.childFrames[1].toString(),
        webView.rootFrame.childFrames[0].childFrames[0].toString(),
        webView.rootFrame.childFrames[0].childFrames[1].toString(),
      ];

      verify_events([
        { type: "added", frame: frames[0] },
        { type: "added", frame: frames[1] },
        { type: "added", frame: frames[2] },
        { type: "added", frame: frames[3] }
      ]);
      verify_tree();
    }
  }
}
