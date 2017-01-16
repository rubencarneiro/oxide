import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.19
import Oxide.Ubuntu 1.0
import Oxide.testsupport 1.0
import Ubuntu.Components 1.3

UbuntuTestWebView {
  id: webView
  width: 800
  height: 600

  objectName: "webView"

  SignalSpy {
    id: downloadRequestSpy
    target: webView
    signalName: "downloadRequested"
  }

  Component {
    id: webViewFactory
    UbuntuTestWebView {}
  }

  property var newViewCreated: null
  property var newViewDisposition

  SignalSpy {
    id: navigationRequestSpy
    target: webView
    signalName: "navigationRequested"
  }

  Connections {
    id: newViewRequestedConnection

    target: null
    onNewViewRequested: {
      newViewDisposition = request.disposition;
      newViewCreated = webViewFactory.createObject(webView, { request: request });
    }
  }

  TestCase {
    id: test
    name: "WebViewContextMenuDesktop"
    when: windowShown

    function getContextMenu() {
      return TestSupport.findItemInScene(TestWindow.rootItem, "webView_WebContextMenu");
    }

    function waitForContextMenu() {
      var r = TestUtils.waitFor(function() { return !!getContextMenu() && getContextMenu().visible; });
      // This seems to be required to ensure that the menu is laid out
      TestSupport.wait(50);
      return r;
    }

    function waitForContextMenuToClose() {
      var menu = getContextMenu();
      if (!menu) {
        return true;
      }

      var helper = TestSupport.createQObjectTestHelper(menu);

      return TestUtils.waitFor(function() { return !getContextMenu(); }) &&
             TestUtils.waitFor(function() { return helper.destroyed; });
    }

    function dismissContextMenu() {
      if (!getContextMenu()) {
        return;
      }

      mouseClick(webView, 1, 1);
      verify(waitForContextMenuToClose());
    }

    function getContextMenuEntryAtIndex(index) {
      return TestSupport.findItemInScene(getContextMenu(), "webView_WebContextMenu_item_" + index);
    }

    function setLocationBarHeight(height) {
      webView.locationBarController.height = height;
      if (height > 0) {
        webView.locationBarController.show(false);
      }
      verify(TestUtils.waitFor(function() { return webView.locationBarController.contentOffset == height; }));
    }

    function verifyClipboardContents(type, value, msg) {
      verify(TestUtils.waitFor(function() { return ClipboardTestUtils.getFromClipboard(type) == value; }), msg);
    }

    function verifyClipboardHasImage(msg) {
      verify(TestUtils.waitFor(function() { return ClipboardTestUtils.hasImage(); }), msg);
    }

    function waitForCreatedView() {
      return TestUtils.waitFor(function() { return !!webView.newViewCreated; });
    }

    function getTextAreaContents(id) {
      return webView.getTestApi().evaluateCode("document.getElementById(\"" + id +  "\").value", false);
    }

    function init() {
      webView.locationBarController.mode = LocationBarController.ModeAuto;
      dismissContextMenu();
      setLocationBarHeight(0);
      if (webView.newViewCreated) {
        TestSupport.destroyQObjectNow(webView.newViewCreated);
        webView.newViewCreated = null;
      }
      newViewRequestedConnection.target = null;
      webView.clearLoadEventCounters();
      webView.url = "http://testsuite/tst_WebViewContextMenu.html";
      verify(webView.waitForLoadSucceeded());
      webView.clearLoadEventCounters();
      ClipboardTestUtils.clearClipboard();
      downloadRequestSpy.clear();
      navigationRequestSpy.clear();
    }

    function test_WebViewContextMenu01_position_data() {
      return [
        { selector: "#link", locationBarHeight: 0 },
        { selector: "#image1", locationBarHeight: 0 },
        { selector: "#image2", locationBarHeight: 0 },
        { selector: "#canvas", locationBarHeight: 0 },
        { selector: "#editable", locationBarHeight: 0 },
        { selector: "#imagelink", locationBarHeight: 0 },
        { selector: "#video", locationBarHeight: 0 },
        { selector: "#audio", locationBarHeight: 0 },
        { selector: "#link", locationBarHeight: 50 },
        { selector: "#image1", locationBarHeight: 50 },
        { selector: "#image2", locationBarHeight: 50 },
        { selector: "#canvas", locationBarHeight: 50 },
        { selector: "#editable", locationBarHeight: 50 },
        { selector: "#imagelink", locationBarHeight: 50 },
        { selector: "#video", locationBarHeight: 50 },
        { selector: "#audio", locationBarHeight: 50 }
      ];
    }

    // Test that the menu is positioned as expected
    function test_WebViewContextMenu01_position(data) {
      setLocationBarHeight(data.locationBarHeight);
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, (r.y + r.height / 2) + webView.locationBarController.contentOffset, Qt.RightButton);
      verify(waitForContextMenu());

      var contextMenu = getContextMenu();
      var expectedX = Math.max(units.gu(2), Math.min(TestWindow.width - units.gu(2), (r.x + r.width / 2) - contextMenu.__foreground.width / 2));
      // This doesn't cope with elements near the bottom of the window
      var expectedY = (r.y + r.height / 2) + units.gu(1) + webView.locationBarController.contentOffset;
      var point = contextMenu.__foreground.mapToItem(null, 0, 0);

      fuzzyCompare(point.x, expectedX, units.gu(1));
      fuzzyCompare(point.y, expectedY, units.gu(1));
    }

    function test_WebViewContextMenu02_contents_data() {
      return [
        { selector: "#link", canCreateWindows: false, count: 2 },
        { selector: "#link", canCreateWindows: true, count: 5 },
        { selector: "#image1", canCreateWindows: false, count: 3 },
        { selector: "#image1", canCreateWindows: true, count: 4 },
        { selector: "#image2", canCreateWindows: false, count: 3 },
        { selector: "#image2", canCreateWindows: true, count: 4 },
        { selector: "#canvas", canCreateWindows: false, count: 2 },
        { selector: "#canvas", canCreateWindows: true, count: 2 },
        { selector: "#editable", canCreateWindows: false, count: 7 },
        { selector: "#editable", canCreateWindows: true, count: 7 },
        { selector: "#imagelink", canCreateWindows: false, count: 5 },
        { selector: "#imagelink", canCreateWindows: true, count: 9 },
        { selector: "#audio", canCreateWindows: false, count: 2 },
        { selector: "#audio", canCreateWindows: true, count: 3 },
        { selector: "#video", canCreateWindows: false, count: 2 },
        { selector: "#video", canCreateWindows: true, count: 3 },
      ];
    }

    function test_WebViewContextMenu02_contents(data) {
      if (data.canCreateWindows) {
        newViewRequestedConnection.target = webView;
      }

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      compare(TestSupport.findItemsInScene(getContextMenu(), "webView_WebContextMenu_item_").length, data.count);
    }

    function test_WebViewContextMenu03_link_copy_location_data() {
      return [
        { selector: "#link" },
        { selector: "#imagelink" }
      ];
    }

    // Test Copy Link Location
    function test_WebViewContextMenu03_link_copy_location(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(0);
      verify(entry.visible);
      mouseClick(entry);

      verifyClipboardContents("text/plain", "http://testsuite/tst_WebViewContextMenu2.html");
    }

    function test_WebViewContextMenu04_link_save_data() {
      return test_WebViewContextMenu03_link_copy_location_data();
    }

    // Test Save Link
    function test_WebViewContextMenu04_link_save(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(1);
      verify(entry.visible);
      mouseClick(entry);

      downloadRequestSpy.wait();
      compare(downloadRequestSpy.signalArguments[0][0].url, "http://testsuite/tst_WebViewContextMenu2.html");
      compare(downloadRequestSpy.signalArguments[0][0].referrer, "http://testsuite/tst_WebViewContextMenu.html");
    }

    function test_WebViewContextMenu05_link_open_in_new_view_data() {
      return [
        { index: 0, disposition: NewViewRequest.DispositionNewForegroundTab, selector: "#link" },
        { index: 1, disposition: NewViewRequest.DispositionNewBackgroundTab, selector: "#link" },
        { index: 2, disposition: NewViewRequest.DispositionNewWindow, selector: "#link" },
        { index: 0, disposition: NewViewRequest.DispositionNewForegroundTab, selector: "#imagelink" },
        { index: 1, disposition: NewViewRequest.DispositionNewBackgroundTab, selector: "#imagelink" },
        { index: 2, disposition: NewViewRequest.DispositionNewWindow, selector: "#imagelink" }
      ];
    }

    // Test Open Link in new tab/background tab/window
    function test_WebViewContextMenu05_link_open_in_new_view(data) {
      newViewRequestedConnection.target = webView;

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(data.index);
      verify(entry.visible);
      mouseClick(entry);

      verify(waitForCreatedView());
      verify(webView.newViewCreated.waitForLoadSucceeded());
      compare(navigationRequestSpy.count, 0);

      compare(webView.newViewCreated.getTestApi().documentURI, "http://testsuite/tst_WebViewContextMenu2.html");
      var headers = JSON.parse(
          webView.newViewCreated.getTestApi().evaluateCode(
              "return document.body.children[0].innerHTML", true));
      compare(headers["referer"], "http://testsuite/tst_WebViewContextMenu.html");
      compare(webView.newViewDisposition, data.disposition);
    }

    function test_WebViewContextMenu06_image_copy_location_data() {
      return [
        { selector: "#image1", url: "http://testsuite/cof.svg", offset: 0, canCreateWindowsOffset: 0 },
        { selector: "#image2", url: "data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTgiPz4NCjwhLS0gR2VuZXJhdG9yOiBBZG9iZSBJbGx1c3RyYXRvciAxNC4wLjAsIFNWRyBFeHBvcnQgUGx1Zy1JbiAgLS0+DQo8IURPQ1RZUEUgc3ZnIFBVQkxJQyAiLS8vVzNDLy9EVEQgU1ZHIDEuMS8vRU4iICJodHRwOi8vd3d3LnczLm9yZy9HcmFwaGljcy9TVkcvMS4xL0RURC9zdmcxMS5kdGQiIFsNCgk8IUVOVElUWSBuc19mbG93cyAiaHR0cDovL25zLmFkb2JlLmNvbS9GbG93cy8xLjAvIj4NCl0+DQo8c3ZnIHZlcnNpb249IjEuMSINCgkgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB4bWxuczp4bGluaz0iaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayIgeG1sbnM6YT0iaHR0cDovL25zLmFkb2JlLmNvbS9BZG9iZVNWR1ZpZXdlckV4dGVuc2lvbnMvMy4wLyINCgkgeD0iMHB4IiB5PSIwcHgiIHdpZHRoPSIyODVweCIgaGVpZ2h0PSIyODVweCIgdmlld0JveD0iLTAuODY2IC0wLjg2NiAyODUgMjg1IiBlbmFibGUtYmFja2dyb3VuZD0ibmV3IC0wLjg2NiAtMC44NjYgMjg1IDI4NSINCgkgeG1sOnNwYWNlPSJwcmVzZXJ2ZSI+DQo8ZGVmcz4NCjwvZGVmcz4NCjxwYXRoIGZpbGw9IiNERDQ4MTQiIGQ9Ik0yODMuNDY1LDE0MS43MzRjMCw3OC4yNzMtNjMuNDU3LDE0MS43My0xNDEuNzM0LDE0MS43M1MwLDIyMC4wMDgsMCwxNDEuNzM0QzAsNjMuNDU1LDYzLjQ1MywwLDE0MS43MywwDQoJUzI4My40NjUsNjMuNDU1LDI4My40NjUsMTQxLjczNHoiLz4NCjxwYXRoIGZpbGw9IiNGRkZGRkYiIGQ9Ik00NS4zNTYsMTIyLjgxMmMtMTAuNDUzLDAtMTguOTIzLDguNDctMTguOTIzLDE4LjkyM2MwLDEwLjQ0OSw4LjQ3LDE4LjkyLDE4LjkyMywxOC45Mg0KCWMxMC40NDksMCwxOC45Mi04LjQ3MSwxOC45Mi0xOC45MkM2NC4yNzYsMTMxLjI4MSw1NS44MDYsMTIyLjgxMiw0NS4zNTYsMTIyLjgxMnogTTE4MC40NjMsMjA4LjgxNA0KCWMtOS4wNTEsNS4yMjUtMTIuMTQ5LDE2Ljc5My02LjkyNiwyNS44NGM1LjIyNiw5LjA1MSwxNi43OTMsMTIuMTUxLDI1Ljg0NCw2LjkyNmM5LjA0OC01LjIyNCwxMi4xNDgtMTYuNzkyLDYuOTIzLTI1Ljg0Mg0KCUMyMDEuMDgsMjA2LjY5MSwxODkuNTExLDIwMy41OSwxODAuNDYzLDIwOC44MTR6IE04Ni40NTgsMTQxLjczMmMwLTE4LjcwMSw5LjI5My0zNS4yMTksMjMuNTA0LTQ1LjIyMUw5Ni4xMjgsNzMuMzM4DQoJYy0xNi41NiwxMS4wNjQtMjguODc4LDI3Ljk3OC0zMy45OTUsNDcuNzg4YzUuOTc3LDQuODcyLDkuNzk2LDEyLjI5MSw5Ljc5NiwyMC42MDhjMCw4LjMxNS0zLjgxOSwxNS43MzQtOS43OTcsMjAuNjA1DQoJYzUuMTE2LDE5LjgxMiwxNy40MzUsMzYuNzI2LDMzLjk5NSw0Ny43ODlsMTMuODM1LTIzLjE3NUM5NS43NTEsMTc2Ljk1Myw4Ni40NTgsMTYwLjQzNiw4Ni40NTgsMTQxLjczMnogTTE0MS43MzMsODYuNDU3DQoJYzI4Ljg3NywwLDUyLjU2NCwyMi4xNDEsNTUuMDQ3LDUwLjM3M2wyNi45NjgtMC4zOTRjLTEuMzI3LTIwLjg0NC0xMC40MzItMzkuNTYyLTI0LjQyNS01My4zMTkNCgljLTcuMTk0LDIuNzE4LTE1LjUwNSwyLjMwNi0yMi42ODgtMS44NDJjLTcuMTkyLTQuMTUyLTExLjcwNS0xMS4xNTYtMTIuOTQxLTE4Ljc1N2MtNi45OTItMS45MzUtMTQuMzUxLTIuOTktMjEuOTYtMi45OQ0KCWMtMTMuMDg2LDAtMjUuNDQ5LDMuMDcyLTM2LjQzMSw4LjUxMmwxMy4xNDYsMjMuNTZDMTI1LjUyNiw4OC4zMDcsMTMzLjQxMiw4Ni40NTcsMTQxLjczMyw4Ni40NTd6IE0xNDEuNzMzLDE5Ny4wMDgNCgljLTguMzIyLDAtMTYuMjA3LTEuODUtMjMuMjg1LTUuMTQzTDEwNS4zLDIxNS40MjdjMTAuOTgzLDUuNDM4LDIzLjM0Nyw4LjUxMSwzNi40MzMsOC41MTFjNy42MDksMCwxNC45NjgtMS4wNTUsMjEuOTYxLTIuOTkNCgljMS4yMzYtNy42MDEsNS43NS0xNC42MDUsMTIuOTQzLTE4Ljc2YzcuMTgzLTQuMTQ2LDE1LjQ5NC00LjU1OCwyMi42ODgtMS44MzljMTMuOTkyLTEzLjc1OCwyMy4wOTctMzIuNDc2LDI0LjQyMi01My4zMg0KCWwtMjYuOTY4LTAuMzk0QzE5NC4yOTgsMTc0Ljg3MSwxNzAuNjEsMTk3LjAwOCwxNDEuNzMzLDE5Ny4wMDh6IE0xODAuNDYsNzQuNjQ5YzkuMDUsNS4yMjcsMjAuNjE5LDIuMTI2LDI1Ljg0Mi02LjkyMQ0KCWM1LjIyNi05LjA1MSwyLjEyOC0yMC42MTktNi45MjMtMjUuODQ1Yy05LjA0OS01LjIyNC0yMC42MTctMi4xMjQtMjUuODQzLDYuOTI3QzE2OC4zMTIsNTcuODU3LDE3MS40MTIsNjkuNDI2LDE4MC40Niw3NC42NDl6Ii8+DQo8L3N2Zz4NCg==", offset: 0, canCreateWindowsOffset: 0 },
        { selector: "#imagelink", url: "http://testsuite/cof.svg", offset: 2, canCreateWindowsOffset: 5 }
      ];
    }

    function test_WebViewContextMenu06_image_copy_location(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(0 + data.offset);
      verify(entry.visible);
      mouseClick(entry);

      verifyClipboardContents("text/plain", data.url);
    }

    function test_WebViewContextMenu07_image_save_data() {
      return test_WebViewContextMenu06_image_copy_location_data();
    }

    function test_WebViewContextMenu07_image_save(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(1 + data.offset);
      verify(entry.visible);
      mouseClick(entry);

      downloadRequestSpy.wait();
      compare(downloadRequestSpy.signalArguments[0][0].url, data.url);
      compare(downloadRequestSpy.signalArguments[0][0].referrer, data.url.toString().indexOf("data:") == 0 ? "" : "http://testsuite/tst_WebViewContextMenu.html");
    }

    function test_WebViewContextMenu08_image_copy_data() {
      return test_WebViewContextMenu06_image_copy_location_data();
    }

    function test_WebViewContextMenu08_image_copy(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(2 + data.offset);
      verify(entry.visible);
      mouseClick(entry);

      verifyClipboardHasImage();
    }

    function test_WebViewContextMenu09_image_open_in_new_view_data() {
      return test_WebViewContextMenu06_image_copy_location_data();
    }

    function test_WebViewContextMenu09_image_open_in_new_view(data) {
      newViewRequestedConnection.target = webView;

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(0 + data.canCreateWindowsOffset);
      verify(entry.visible);
      mouseClick(entry);

      verify(waitForCreatedView());
      verify(webView.newViewCreated.waitForLoadSucceeded());
      compare(navigationRequestSpy.count, 0);

      compare(webView.newViewCreated.getTestApi().documentURI,
              data.url);
      compare(webView.newViewDisposition, NewViewRequest.DispositionNewBackgroundTab);
    }

    function test_WebViewContextMenu10_canvas_save() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#canvas");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(0);
      verify(entry.visible);
      mouseClick(entry);

      downloadRequestSpy.wait();
      compare(downloadRequestSpy.signalArguments[0][0].url, "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAASwAAACWCAYAAABkW7XSAAAEYklEQVR4Xu3UAQkAAAwCwdm/9HI83BLIOdw5AgQIRAQWySkmAQIEzmB5AgIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlAABg+UHCBDICBisTFWCEiBgsPwAAQIZAYOVqUpQAgQMlh8gQCAjYLAyVQlKgIDB8gMECGQEDFamKkEJEDBYfoAAgYyAwcpUJSgBAgbLDxAgkBEwWJmqBCVAwGD5AQIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlAABg+UHCBDICBisTFWCEiBgsPwAAQIZAYOVqUpQAgQMlh8gQCAjYLAyVQlKgIDB8gMECGQEDFamKkEJEDBYfoAAgYyAwcpUJSgBAgbLDxAgkBEwWJmqBCVAwGD5AQIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlAABg+UHCBDICBisTFWCEiBgsPwAAQIZAYOVqUpQAgQMlh8gQCAjYLAyVQlKgIDB8gMECGQEDFamKkEJEDBYfoAAgYyAwcpUJSgBAgbLDxAgkBEwWJmqBCVAwGD5AQIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlAABg+UHCBDICBisTFWCEiBgsPwAAQIZAYOVqUpQAgQMlh8gQCAjYLAyVQlKgIDB8gMECGQEDFamKkEJEDBYfoAAgYyAwcpUJSgBAgbLDxAgkBEwWJmqBCVAwGD5AQIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlAABg+UHCBDICBisTFWCEiBgsPwAAQIZAYOVqUpQAgQMlh8gQCAjYLAyVQlKgIDB8gMECGQEDFamKkEJEDBYfoAAgYyAwcpUJSgBAgbLDxAgkBEwWJmqBCVAwGD5AQIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlAABg+UHCBDICBisTFWCEiBgsPwAAQIZAYOVqUpQAgQMlh8gQCAjYLAyVQlKgIDB8gMECGQEDFamKkEJEDBYfoAAgYyAwcpUJSgBAgbLDxAgkBEwWJmqBCVAwGD5AQIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlAABg+UHCBDICBisTFWCEiBgsPwAAQIZAYOVqUpQAgQMlh8gQCAjYLAyVQlKgIDB8gMECGQEDFamKkEJEDBYfoAAgYyAwcpUJSgBAgbLDxAgkBEwWJmqBCVAwGD5AQIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlAABg+UHCBDICBisTFWCEiBgsPwAAQIZAYOVqUpQAgQMlh8gQCAjYLAyVQlKgIDB8gMECGQEDFamKkEJEDBYfoAAgYyAwcpUJSgBAgbLDxAgkBEwWJmqBCVAwGD5AQIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlAABg+UHCBDICBisTFWCEiBgsPwAAQIZAYOVqUpQAgQMlh8gQCAjYLAyVQlKgIDB8gMECGQEDFamKkEJEDBYfoAAgYyAwcpUJSgBAgbLDxAgkBEwWJmqBCVAwGD5AQIEMgIGK1OVoAQIGCw/QIBARsBgZaoSlACBB1YxAJfjJb2jAAAAAElFTkSuQmCC");
      compare(downloadRequestSpy.signalArguments[0][0].referrer, "");
    }

    function test_WebViewContextMenu11_canvas_copy() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#canvas");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(1);
      verify(entry.visible);
      mouseClick(entry);

      verifyClipboardHasImage();
    }

    function test_WebViewContextMenu12_editable_undo() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#editable");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(0);
      verify(!entry.visible);

      dismissContextMenu();
      compare(getTextAreaContents("editable"), "Test area");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      keyPress("A");

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(0);
      verify(entry.visible);
      verify(getTextAreaContents("editable") != "Test area");

      mouseClick(entry);
      verify(waitForContextMenuToClose());

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(0);
      verify(!entry.visible);

      compare(getTextAreaContents("editable"), "Test area");
    }

    function test_WebViewContextMenu13_editable_redo() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#editable");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(1);
      verify(!entry.visible);

      dismissContextMenu();
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      keyPress("A");

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(1);
      verify(!entry.visible);

      entry = getContextMenuEntryAtIndex(0);
      mouseClick(entry);
      verify(waitForContextMenuToClose());

      compare(getTextAreaContents("editable"), "Test area");

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(1);
      verify(entry.visible);
      mouseClick(entry);
      verify(waitForContextMenuToClose());

      verify(getTextAreaContents("editable") != "Test area");

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(1);
      verify(!entry.visible);
    }

    function test_WebViewContextMenu14_editable_cut() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#editable");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(2);
      verify(!entry.visible);

      dismissContextMenu();
      compare(getTextAreaContents("editable"), "Test area");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(2);
      verify(entry.visible);
      mouseClick(entry);

      verifyClipboardContents("text/plain", "Test area");
      compare(getTextAreaContents("editable"), "");

      verify(waitForContextMenuToClose());

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(2);
      verify(!entry.visible);
    }

    function test_WebViewContextMenu15_editable_copy() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#editable");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(3);
      verify(!entry.visible);

      dismissContextMenu();
      compare(getTextAreaContents("editable"), "Test area");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(3);
      verify(entry.visible);
      mouseClick(entry);

      verifyClipboardContents("text/plain", "Test area");
      compare(getTextAreaContents("editable"), "Test area");

      verify(waitForContextMenuToClose());

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(3);
      verify(!entry.visible);
    }

    function test_WebViewContextMenu16_editable_paste() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#editable");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(4);
      verify(entry.visible); // Can always paste

      dismissContextMenu();
      ClipboardTestUtils.copyToClipboard("text/plain", "moo");

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(4);
      verify(entry.visible);
      mouseClick(entry);

      verify(getTextAreaContents("editable").indexOf("moo") != -1);
    }

    function test_WebViewContextMenu17_editable_erase() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#editable");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(5);
      verify(!entry.visible);

      dismissContextMenu();
      compare(getTextAreaContents("editable"), "Test area");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      entry = getContextMenuEntryAtIndex(5);
      verify(entry.visible);
      mouseClick(entry);
      compare(getTextAreaContents("editable"), "");
    }

    function test_WebViewContextMenu18_editable_select_all() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#editable");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      compare(getTextAreaContents("editable"), "Test area");

      var entry = getContextMenuEntryAtIndex(6);
      verify(entry.visible);
      mouseClick(entry);
      verify(waitForContextMenuToClose());

      keyPress("1");
      TestSupport.wait(100); // Yuck
      compare(getTextAreaContents("editable"), "1");
    }

    function test_WebViewContextMenu19_media_copy_location_data() {
      return [
        { selector: "#audio", url: "http://testsuite/fire.oga" },
        { selector: "#video", url: "http://testsuite/buddha.mp4" },
      ];
    }

    function test_WebViewContextMenu19_media_copy_location(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(0);
      verify(entry.visible);
      mouseClick(entry);

      verifyClipboardContents("text/plain", data.url);
    }

    function test_WebViewContextMenu20_media_save_data() {
      return test_WebViewContextMenu19_media_copy_location_data();
    }

    function test_WebViewContextMenu20_media_save(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(1);
      verify(entry.visible);
      mouseClick(entry);

      downloadRequestSpy.wait();
      compare(downloadRequestSpy.signalArguments[0][0].url, data.url);
      compare(downloadRequestSpy.signalArguments[0][0].referrer, "http://testsuite/tst_WebViewContextMenu.html");
    }

    function test_WebViewContextMenu21_media_open_in_new_view_data() {
      return test_WebViewContextMenu19_media_copy_location_data();
    }

    function test_WebViewContextMenu21_media_open_in_new_view(data) {
      newViewRequestedConnection.target = webView;

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var entry = getContextMenuEntryAtIndex(0);
      verify(entry.visible);
      mouseClick(entry);

      verify(waitForCreatedView());
      // XXX(chrisccoulson): Wat https://launchpad.net/bugs/1645909
      verify(webView.newViewCreated.waitForLoadStopped());

      compare(webView.newViewCreated.getTestApi().documentURI,
              data.url);
      compare(webView.newViewDisposition, NewViewRequest.DispositionNewBackgroundTab);
    }

    function test_WebViewContextMenu22_title_data() {
      return [
        { selector: "#link", title: "http://testsuite/tst_WebViewContextMenu2.html" },
        { selector: "#image1", title: "http://testsuite/cof.svg" },
        { selector: "#image2", title: "data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTgiPz4NCjwhLS0gR2VuZXJhdG9yOiBBZG9iZSBJbGx1c3RyYXRvciAxNC4wLjAsIFNWRyBFeHBvcnQgUGx1Zy1JbiAgLS0+DQo8IURPQ1RZUEUgc3ZnIFBVQkxJQyAiLS8vVzNDLy9EVEQgU1ZHIDEuMS8vRU4iICJodHRwOi8vd3d3LnczLm9yZy9HcmFwaGljcy9TVkcvMS4xL0RURC9zdmcxMS5kdGQiIFsNCgk8IUVOVElUWSBuc19mbG93cyAiaHR0cDovL25zLmFkb2JlLmNvbS9GbG93cy8xLjAvIj4NCl0+DQo8c3ZnIHZlcnNpb249IjEuMSINCgkgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB4bWxuczp4bGluaz0iaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayIgeG1sbnM6YT0iaHR0cDovL25zLmFkb2JlLmNvbS9BZG9iZVNWR1ZpZXdlckV4dGVuc2lvbnMvMy4wLyINCgkgeD0iMHB4IiB5PSIwcHgiIHdpZHRoPSIyODVweCIgaGVpZ2h0PSIyODVweCIgdmlld0JveD0iLTAuODY2IC0wLjg2NiAyODUgMjg1IiBlbmFibGUtYmFja2dyb3VuZD0ibmV3IC0wLjg2NiAtMC44NjYgMjg1IDI4NSINCgkgeG1sOnNwYWNlPSJwcmVzZXJ2ZSI+DQo8ZGVmcz4NCjwvZGVmcz4NCjxwYXRoIGZpbGw9IiNERDQ4MTQiIGQ9Ik0yODMuNDY1LDE0MS43MzRjMCw3OC4yNzMtNjMuNDU3LDE0MS43My0xNDEuNzM0LDE0MS43M1MwLDIyMC4wMDgsMCwxNDEuNzM0QzAsNjMuNDU1LDYzLjQ1MywwLDE0MS43MywwDQoJUzI4My40NjUsNjMuNDU1LDI4My40NjUsMTQxLjczNHoiLz4NCjxwYXRoIGZpbGw9IiNGRkZGRkYiIGQ9Ik00NS4zNTYsMTIyLjgxMmMtMTAuNDUzLDAtMTguOTIzLDguNDctMTguOTIzLDE4LjkyM2MwLDEwLjQ0OSw4LjQ3LDE4LjkyLDE4LjkyMywxOC45Mg0KCWMxMC40NDksMCwxOC45Mi04LjQ3MSwxOC45Mi0xOC45MkM2NC4yNzYsMTMxLjI4MSw1NS44MDYsMTIyLjgxMiw0NS4zNTYsMTIyLjgxMnogTTE4MC40NjMsMjA4LjgxNA0KCWMtOS4wNTEsNS4yMjUtMTIuMTQ5LDE2Ljc5My02LjkyNiwyNS44NGM1LjIyNiw5LjA1MSwxNi43OTMsMTIuMTUxLDI1Ljg0NCw2LjkyNmM5LjA0OC01LjIyNCwxMi4xNDgtMTYuNzkyLDYuOTIzLTI1Ljg0Mg0KCUMyMDEuMDgsMjA2LjY5MSwxODkuNTExLDIwMy41OSwxODAuNDYzLDIwOC44MTR6IE04Ni40NTgsMTQxLjczMmMwLTE4LjcwMSw5LjI5My0zNS4yMTksMjMuNTA0LTQ1LjIyMUw5Ni4xMjgsNzMuMzM4DQoJYy0xNi41NiwxMS4wNjQtMjguODc4LDI3Ljk3OC0zMy45OTUsNDcuNzg4YzUuOTc3LDQuODcyLDkuNzk2LDEyLjI5MSw5Ljc5NiwyMC42MDhjMCw4LjMxNS0zLjgxOSwxNS43MzQtOS43OTcsMjAuNjA1DQoJYzUuMTE2LDE5LjgxMiwxNy40MzUsMzYuNzI2LDMzLjk5NSw0Ny43ODlsMTMuODM1LTIzLjE3NUM5NS43NTEsMTc2Ljk1Myw4Ni40NTgsMTYwLjQzNiw4Ni40NTgsMTQxLjczMnogTTE0MS43MzMsODYuNDU3DQoJYzI4Ljg3NywwLDUyLjU2NCwyMi4xNDEsNTUuMDQ3LDUwLjM3M2wyNi45NjgtMC4zOTRjLTEuMzI3LTIwLjg0NC0xMC40MzItMzkuNTYyLTI0LjQyNS01My4zMTkNCgljLTcuMTk0LDIuNzE4LTE1LjUwNSwyLjMwNi0yMi42ODgtMS44NDJjLTcuMTkyLTQuMTUyLTExLjcwNS0xMS4xNTYtMTIuOTQxLTE4Ljc1N2MtNi45OTItMS45MzUtMTQuMzUxLTIuOTktMjEuOTYtMi45OQ0KCWMtMTMuMDg2LDAtMjUuNDQ5LDMuMDcyLTM2LjQzMSw4LjUxMmwxMy4xNDYsMjMuNTZDMTI1LjUyNiw4OC4zMDcsMTMzLjQxMiw4Ni40NTcsMTQxLjczMyw4Ni40NTd6IE0xNDEuNzMzLDE5Ny4wMDgNCgljLTguMzIyLDAtMTYuMjA3LTEuODUtMjMuMjg1LTUuMTQzTDEwNS4zLDIxNS40MjdjMTAuOTgzLDUuNDM4LDIzLjM0Nyw4LjUxMSwzNi40MzMsOC41MTFjNy42MDksMCwxNC45NjgtMS4wNTUsMjEuOTYxLTIuOTkNCgljMS4yMzYtNy42MDEsNS43NS0xNC42MDUsMTIuOTQzLTE4Ljc2YzcuMTgzLTQuMTQ2LDE1LjQ5NC00LjU1OCwyMi42ODgtMS44MzljMTMuOTkyLTEzLjc1OCwyMy4wOTctMzIuNDc2LDI0LjQyMi01My4zMg0KCWwtMjYuOTY4LTAuMzk0QzE5NC4yOTgsMTc0Ljg3MSwxNzAuNjEsMTk3LjAwOCwxNDEuNzMzLDE5Ny4wMDh6IE0xODAuNDYsNzQuNjQ5YzkuMDUsNS4yMjcsMjAuNjE5LDIuMTI2LDI1Ljg0Mi02LjkyMQ0KCWM1LjIyNi05LjA1MSwyLjEyOC0yMC42MTktNi45MjMtMjUuODQ1Yy05LjA0OS01LjIyNC0yMC42MTctMi4xMjQtMjUuODQzLDYuOTI3QzE2OC4zMTIsNTcuODU3LDE3MS40MTIsNjkuNDI2LDE4MC40Niw3NC42NDl6Ii8+DQo8L3N2Zz4NCg==" },
        { selector: "#canvas", title: "" },
        { selector: "#editable", title: "" },
        { selector: "#imagelink", title: "http://testsuite/cof.svg" },
        { selector: "#video", title: "http://testsuite/buddha.mp4" },
        { selector: "#audio", title: "http://testsuite/fire.oga" }
      ];
    }

    function test_WebViewContextMenu22_title(data) {
      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      var title = TestSupport.findItemInScene(getContextMenu(), "webView_WebContextMenu_title");
      verify(data.title == "" ? !title.visible : title.visible);
      compare(title.text, data.title);
    }

    function test_WebViewContextMenu23_empty() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#text");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      TestSupport.wait(1000);

      verify(!getContextMenu());
    }

    function test_WebViewContextMenu24_text() {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#text");
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2);

      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);
      verify(waitForContextMenu());

      compare(TestSupport.findItemsInScene(getContextMenu(), "webView_WebContextMenu_item_").length, 1);

      var entry = getContextMenuEntryAtIndex(0);
      verify(entry.visible);
      mouseClick(entry);

      verifyClipboardContents("text/plain", "Some text\n");
    }

    function test_WebViewContextMenu25_destroy_on_webview_close() {
      var webView2 = webViewFactory.createObject(webView, { "anchors.fill": parent, objectName: "webView2" });
      webView2.url = "http://testsuite/tst_WebViewContextMenu.html";
      verify(webView2.waitForLoadSucceeded());

      var r = webView2.getTestApi().getBoundingClientRectForSelector("#link");
      mouseClick(webView2, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);

      function getContextMenu() {
        return TestSupport.findItemInScene(TestWindow.rootItem, "webView2_WebContextMenu");
      }
      verify(TestUtils.waitFor(function() { return !!getContextMenu() && getContextMenu().visible; }));

      var menu = getContextMenu();
      var helper = TestSupport.createQObjectTestHelper(menu);

      webView2.destroy();
      verify(TestUtils.waitFor(function() { return helper.destroyed; }));
    }
  }
}
