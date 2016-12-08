import QtQuick 2.0
import QtTest 1.0
import Oxide.testsupport 1.0
import Oxide.Ubuntu 1.0
import Ubuntu.Components 1.3

UbuntuTestWebView {
  id: webView

  objectName: "webView"

  onNewViewRequested: {}

  TestCase {
    id: test
    name: "UbuntuWebView_contextMenuOpening"
    when: windowShown

    function wrapCallbackTestSequence(callback) {
      var testCompleted = false;
      function runTest() {
        try {
          callback.apply(null, arguments);
        } catch(e) {
          fail("Callback test sequence threw exception: " + e);
        } finally {
          testCompleted = true;
        }
      }

      runTest.wait = function(timeout) {
        timeout = timeout || 5000;
        return TestUtils.waitFor(function() { return testCompleted; }, timeout);
      }

      return runTest;
    }

    function init() {
      webView.clearLoadEventCounters();
      webView.url = "http://testsuite/tst_UbuntuWebView_contextMenuOpening.html";
      verify(webView.waitForLoadSucceeded());
    }

    function test_UbuntuWebView_contextMenuOpening1_params_data() {
      return [
        { selector: "#text", frameUrl: "", isLink: false, isEditable: false,
          mediaType: WebContextMenuParams.MediaTypeNone, linkUrl: "", linkText: "",
          titleText: "", srcUrl: "" },
        { selector: "#link", frameUrl: "", isLink: true, isEditable: false,
          mediaType: WebContextMenuParams.MediaTypeNone, linkUrl: "http://testsuite/empty.html", linkText: "Linky",
          titleText: "", srcUrl: "" },
        { selector: "#image1", frameUrl: "", isLink: false, isEditable: false,
          mediaType: WebContextMenuParams.MediaTypeImage, linkUrl: "", linkText: "",
          titleText: "", srcUrl: "http://testsuite/cof.svg" },
        { selector: "#image2", frameUrl: "", isLink: false, isEditable: false,
          mediaType: WebContextMenuParams.MediaTypeImage, linkUrl: "", linkText: "",
          titleText: "", srcUrl: "data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTgiPz4NCjwhLS0gR2VuZXJhdG9yOiBBZG9iZSBJbGx1c3RyYXRvciAxNC4wLjAsIFNWRyBFeHBvcnQgUGx1Zy1JbiAgLS0+DQo8IURPQ1RZUEUgc3ZnIFBVQkxJQyAiLS8vVzNDLy9EVEQgU1ZHIDEuMS8vRU4iICJodHRwOi8vd3d3LnczLm9yZy9HcmFwaGljcy9TVkcvMS4xL0RURC9zdmcxMS5kdGQiIFsNCgk8IUVOVElUWSBuc19mbG93cyAiaHR0cDovL25zLmFkb2JlLmNvbS9GbG93cy8xLjAvIj4NCl0+DQo8c3ZnIHZlcnNpb249IjEuMSINCgkgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB4bWxuczp4bGluaz0iaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayIgeG1sbnM6YT0iaHR0cDovL25zLmFkb2JlLmNvbS9BZG9iZVNWR1ZpZXdlckV4dGVuc2lvbnMvMy4wLyINCgkgeD0iMHB4IiB5PSIwcHgiIHdpZHRoPSIyODVweCIgaGVpZ2h0PSIyODVweCIgdmlld0JveD0iLTAuODY2IC0wLjg2NiAyODUgMjg1IiBlbmFibGUtYmFja2dyb3VuZD0ibmV3IC0wLjg2NiAtMC44NjYgMjg1IDI4NSINCgkgeG1sOnNwYWNlPSJwcmVzZXJ2ZSI+DQo8ZGVmcz4NCjwvZGVmcz4NCjxwYXRoIGZpbGw9IiNERDQ4MTQiIGQ9Ik0yODMuNDY1LDE0MS43MzRjMCw3OC4yNzMtNjMuNDU3LDE0MS43My0xNDEuNzM0LDE0MS43M1MwLDIyMC4wMDgsMCwxNDEuNzM0QzAsNjMuNDU1LDYzLjQ1MywwLDE0MS43MywwDQoJUzI4My40NjUsNjMuNDU1LDI4My40NjUsMTQxLjczNHoiLz4NCjxwYXRoIGZpbGw9IiNGRkZGRkYiIGQ9Ik00NS4zNTYsMTIyLjgxMmMtMTAuNDUzLDAtMTguOTIzLDguNDctMTguOTIzLDE4LjkyM2MwLDEwLjQ0OSw4LjQ3LDE4LjkyLDE4LjkyMywxOC45Mg0KCWMxMC40NDksMCwxOC45Mi04LjQ3MSwxOC45Mi0xOC45MkM2NC4yNzYsMTMxLjI4MSw1NS44MDYsMTIyLjgxMiw0NS4zNTYsMTIyLjgxMnogTTE4MC40NjMsMjA4LjgxNA0KCWMtOS4wNTEsNS4yMjUtMTIuMTQ5LDE2Ljc5My02LjkyNiwyNS44NGM1LjIyNiw5LjA1MSwxNi43OTMsMTIuMTUxLDI1Ljg0NCw2LjkyNmM5LjA0OC01LjIyNCwxMi4xNDgtMTYuNzkyLDYuOTIzLTI1Ljg0Mg0KCUMyMDEuMDgsMjA2LjY5MSwxODkuNTExLDIwMy41OSwxODAuNDYzLDIwOC44MTR6IE04Ni40NTgsMTQxLjczMmMwLTE4LjcwMSw5LjI5My0zNS4yMTksMjMuNTA0LTQ1LjIyMUw5Ni4xMjgsNzMuMzM4DQoJYy0xNi41NiwxMS4wNjQtMjguODc4LDI3Ljk3OC0zMy45OTUsNDcuNzg4YzUuOTc3LDQuODcyLDkuNzk2LDEyLjI5MSw5Ljc5NiwyMC42MDhjMCw4LjMxNS0zLjgxOSwxNS43MzQtOS43OTcsMjAuNjA1DQoJYzUuMTE2LDE5LjgxMiwxNy40MzUsMzYuNzI2LDMzLjk5NSw0Ny43ODlsMTMuODM1LTIzLjE3NUM5NS43NTEsMTc2Ljk1Myw4Ni40NTgsMTYwLjQzNiw4Ni40NTgsMTQxLjczMnogTTE0MS43MzMsODYuNDU3DQoJYzI4Ljg3NywwLDUyLjU2NCwyMi4xNDEsNTUuMDQ3LDUwLjM3M2wyNi45NjgtMC4zOTRjLTEuMzI3LTIwLjg0NC0xMC40MzItMzkuNTYyLTI0LjQyNS01My4zMTkNCgljLTcuMTk0LDIuNzE4LTE1LjUwNSwyLjMwNi0yMi42ODgtMS44NDJjLTcuMTkyLTQuMTUyLTExLjcwNS0xMS4xNTYtMTIuOTQxLTE4Ljc1N2MtNi45OTItMS45MzUtMTQuMzUxLTIuOTktMjEuOTYtMi45OQ0KCWMtMTMuMDg2LDAtMjUuNDQ5LDMuMDcyLTM2LjQzMSw4LjUxMmwxMy4xNDYsMjMuNTZDMTI1LjUyNiw4OC4zMDcsMTMzLjQxMiw4Ni40NTcsMTQxLjczMyw4Ni40NTd6IE0xNDEuNzMzLDE5Ny4wMDgNCgljLTguMzIyLDAtMTYuMjA3LTEuODUtMjMuMjg1LTUuMTQzTDEwNS4zLDIxNS40MjdjMTAuOTgzLDUuNDM4LDIzLjM0Nyw4LjUxMSwzNi40MzMsOC41MTFjNy42MDksMCwxNC45NjgtMS4wNTUsMjEuOTYxLTIuOTkNCgljMS4yMzYtNy42MDEsNS43NS0xNC42MDUsMTIuOTQzLTE4Ljc2YzcuMTgzLTQuMTQ2LDE1LjQ5NC00LjU1OCwyMi42ODgtMS44MzljMTMuOTkyLTEzLjc1OCwyMy4wOTctMzIuNDc2LDI0LjQyMi01My4zMg0KCWwtMjYuOTY4LTAuMzk0QzE5NC4yOTgsMTc0Ljg3MSwxNzAuNjEsMTk3LjAwOCwxNDEuNzMzLDE5Ny4wMDh6IE0xODAuNDYsNzQuNjQ5YzkuMDUsNS4yMjcsMjAuNjE5LDIuMTI2LDI1Ljg0Mi02LjkyMQ0KCWM1LjIyNi05LjA1MSwyLjEyOC0yMC42MTktNi45MjMtMjUuODQ1Yy05LjA0OS01LjIyNC0yMC42MTctMi4xMjQtMjUuODQzLDYuOTI3QzE2OC4zMTIsNTcuODU3LDE3MS40MTIsNjkuNDI2LDE4MC40Niw3NC42NDl6Ii8+DQo8L3N2Zz4NCg==" },
        { selector: "#canvas", frameUrl: "", isLink: false, isEditable: false,
          mediaType: WebContextMenuParams.MediaTypeCanvas, linkUrl: "", linkText: "",
          titleText: "Test title", srcUrl: "" },
        { selector: "#editable", frameUrl: "", isLink: false, isEditable: true,
          mediaType: WebContextMenuParams.MediaTypeNone, linkUrl: "", linkText: "",
          titleText: "", srcUrl: "" },
        { selector: "#imagelink", frameUrl: "", isLink: true, isEditable: false,
          mediaType: WebContextMenuParams.MediaTypeImage, linkUrl: "http://testsuite/empty.html", linkText: "",
          titleText: "", srcUrl: "http://testsuite/cof.svg" },
        { selector: "#video", frameUrl: "", isLink: false, isEditable: false,
          mediaType: WebContextMenuParams.MediaTypeVideo, linkUrl: "", linkText: "",
          titleText: "", srcUrl: "http://testsuite/buddha.mp4" },
        { selector: "#audio", frameUrl: "", isLink: false, isEditable: false,
          mediaType: WebContextMenuParams.MediaTypeAudio, linkUrl: "", linkText: "",
          titleText: "", srcUrl: "http://testsuite/fire.oga" },
      ];
    }

    function test_UbuntuWebView_contextMenuOpening1_params(data) {
      function contextMenuOpeningHandler(params, menu) {
        menu.removeAll();

        compare(params.pageUrl, "http://testsuite/tst_UbuntuWebView_contextMenuOpening.html");
        compare(params.frameUrl, data.frameUrl);
        compare(params.isLink, data.isLink);
        compare(params.isEditable, data.isEditable);
        verify(!params.isSelection);
        compare(params.mediaType, data.mediaType);
        compare(params.linkUrl, data.linkUrl);
        compare(params.linkText, data.linkText);
        compare(params.titleText, data.titleText);
        compare(params.srcUrl, data.srcUrl);
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);

      verify(callback.wait());
      webView.contextMenuOpening.disconnect(callback);
    }

    function test_UbuntuWebView_contextMenuOpening2_default_actions_data() {
      return [
        { selector: "#text",
          actions: [
          ]
        },
        { selector: "#link",
          actions: [
            { action: UbuntuWebContextMenuItem.ActionOpenLinkInNewTab, section: UbuntuWebContextMenuItem.SectionOpenLink, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionOpenLinkInNewBackgroundTab, section: UbuntuWebContextMenuItem.SectionOpenLink, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionOpenLinkInNewWindow, section: UbuntuWebContextMenuItem.SectionOpenLink, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyLinkLocation, section: UbuntuWebContextMenuItem.SectionLink, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionSaveLink, section: UbuntuWebContextMenuItem.SectionLink, enabled: true },
          ]
        },
        { selector: "#image1",
          actions: [
            { action: UbuntuWebContextMenuItem.ActionOpenMediaInNewTab, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyMediaLocation, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionSaveMedia, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyImage, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
          ]
        },
        { selector: "#image2",
          actions: [
            { action: UbuntuWebContextMenuItem.ActionOpenMediaInNewTab, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyMediaLocation, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionSaveMedia, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyImage, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
          ]
        },
        { selector: "#canvas",
          actions: [
            { action: UbuntuWebContextMenuItem.ActionSaveMedia, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyImage, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
          ]
        },
        { selector: "#editable",
          actions: [
            { action: UbuntuWebContextMenuItem.ActionUndo, section: UbuntuWebContextMenuItem.SectionEditing, enabled: false },
            { action: UbuntuWebContextMenuItem.ActionRedo, section: UbuntuWebContextMenuItem.SectionEditing, enabled: false },
            { action: UbuntuWebContextMenuItem.ActionCut, section: UbuntuWebContextMenuItem.SectionEditing, enabled: false },
            { action: UbuntuWebContextMenuItem.ActionCopy, section: UbuntuWebContextMenuItem.SectionEditing, enabled: false },
            { action: UbuntuWebContextMenuItem.ActionPaste, section: UbuntuWebContextMenuItem.SectionEditing, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionErase, section: UbuntuWebContextMenuItem.SectionEditing, enabled: false },
            { action: UbuntuWebContextMenuItem.ActionSelectAll, section: UbuntuWebContextMenuItem.SectionEditing, enabled: true },
          ]
        },
        { selector: "#imagelink",
          actions: [
            { action: UbuntuWebContextMenuItem.ActionOpenLinkInNewTab, section: UbuntuWebContextMenuItem.SectionOpenLink, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionOpenLinkInNewBackgroundTab, section: UbuntuWebContextMenuItem.SectionOpenLink, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionOpenLinkInNewWindow, section: UbuntuWebContextMenuItem.SectionOpenLink, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyLinkLocation, section: UbuntuWebContextMenuItem.SectionLink, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionSaveLink, section: UbuntuWebContextMenuItem.SectionLink, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionOpenMediaInNewTab, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyMediaLocation, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionSaveMedia, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyImage, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
          ]
        },
        { selector: "#video",
          actions: [
            { action: UbuntuWebContextMenuItem.ActionOpenMediaInNewTab, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyMediaLocation, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionSaveMedia, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
          ]
        },
        { selector: "#audio",
          actions: [
            { action: UbuntuWebContextMenuItem.ActionOpenMediaInNewTab, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionCopyMediaLocation, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
            { action: UbuntuWebContextMenuItem.ActionSaveMedia, section: UbuntuWebContextMenuItem.SectionMedia, enabled: true },
          ]
        }
      ];
    }

    function test_UbuntuWebView_contextMenuOpening2_default_actions(data) {
      function contextMenuOpeningHandler(params, menu) {
        try {
          compare(menu.items.length, data.actions.length);
          for (var i = 0; i < data.actions.length; ++i) {
            compare(menu.items[i].stockAction, data.actions[i].action);
            compare(menu.items[i].section, data.actions[i].section, "action: " + menu.items[i].stockAction);
            verify(menu.items[i].action, "action: " + menu.items[i].stockAction);
            compare(menu.items[i].action.enabled, data.actions[i].enabled, "action: " + menu.items[i].stockAction);
            compare(menu.items[i].action.visible, true, "action: " + menu.items[i].stockAction);
          }
        } finally {
          menu.removeAll();
        }
      }

      var callback = wrapCallbackTestSequence(contextMenuOpeningHandler);
      webView.contextMenuOpening.connect(callback);

      var r = webView.getTestApi().getBoundingClientRectForSelector(data.selector);
      mouseClick(webView, r.x + r.width / 2, r.y + r.height / 2, Qt.RightButton);

      verify(callback.wait());
      webView.contextMenuOpening.disconnect(callback);
    }
  }
}
