import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property var currentContextMenu: null
  property url downloadUrl
  property string downloadReferrer

  contextMenu: Item {
    property var contextModel: model
    Component.onCompleted: WebView.view.currentContextMenu = this;
    Component.onDestruction: WebView.view.currentContextMenu = null;
  }

  function waitForContextMenu() {
    return waitFor(function() { return currentContextMenu != null; });
  }

  onDownloadRequested: {
    downloadUrl = request.url;
    downloadReferrer = request.referrer;
  }

  SignalSpy {
    id: spy
    target: webView
    signalName: "downloadRequested"
  }

  TestCase {
    name: "WebView_contextMenu"
    when: windowShown

    function init() {
      webView.currentContextMenu = null;

      webView.url = "http://testsuite/tst_WebView_contextMenu.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      spy.clear();
    }

    function cleanup() {
      webView.currentContextMenu.contextModel.close()
      tryCompare(webView, "currentContextMenu", null);
    }

    function invokeContextMenu(id) {
      var r = webView.getTestApi().getBoundingClientRectForSelector("#" + id);
      var x = r.x + r.width / 2;
      var y = r.y + r.height / 2;
      mouseClick(webView, x, y, Qt.RightButton);
      verify(webView.waitForContextMenu(),
             "Timed out waiting for context menu to show");
      var model = webView.currentContextMenu.contextModel;
      compare(model.position, Qt.point(x, y));
      compare(model.pageUrl, "http://testsuite/tst_WebView_contextMenu.html");
    }

    function test_WebView_contextMenu_properties_data() {
      return [
        { id: "text", mediaType: WebView.MediaTypeNone, linkUrl: "", linkText: "", srcUrl: "", frameUrl: "", isEditable: false },
        { id: "hyperlink", mediaType: WebView.MediaTypeNone, linkUrl: "http://testsuite/empty.html", linkText: "super-link", srcUrl: "", frameUrl: "", isEditable: false },
        { id: "image", mediaType: WebView.MediaTypeImage, linkUrl: "", linkText: "", srcUrl: "http://testsuite/cof.svg", frameUrl: "", isEditable: false },
        { id: "canvas", mediaType: WebView.MediaTypeCanvas, linkUrl: "", linkText: "", srcUrl: "", frameUrl: "", isEditable: false },
        { id: "editable", mediaType: WebView.MediaTypeNone, linkUrl: "", linkText: "", srcUrl: "", frameUrl: "", isEditable: true },
        { id: "imagelink", mediaType: WebView.MediaTypeImage, linkUrl: "http://testsuite/empty.html", linkText: "", srcUrl: "http://testsuite/cof.svg", frameUrl: "", isEditable: false },
        { id: "iframe", mediaType: WebView.MediaTypeNone, linkUrl: "", linkText: "", srcUrl: "", frameUrl: "http://testsuite/empty.html", isEditable: false },
        { id: "video", mediaType: WebView.MediaTypeVideo, linkUrl: "", linkText: "", srcUrl: "http://testsuite/buddha.mp4", frameUrl: "", isEditable: false },
        { id: "audio", mediaType: WebView.MediaTypeAudio, linkUrl: "", linkText: "", srcUrl: "http://testsuite/fire.oga", frameUrl: "", isEditable: false },
      ];
    }

    function test_WebView_contextMenu_properties(data) {
      invokeContextMenu(data.id);
      var model = webView.currentContextMenu.contextModel;
      compare(model.mediaType, data.mediaType);
      compare(model.linkUrl, data.linkUrl);
      compare(model.linkText, data.linkText);
      compare(model.srcUrl, data.srcUrl);
      if ((model.mediaType == WebView.MediaTypeImage) ||
          (model.mediaType == WebView.MediaTypeCanvas)) {
        verify(model.hasImageContents);
      }
      compare(model.frameUrl, data.frameUrl);
      compare(model.isEditable, data.isEditable);
    }

    function test_WebView_contextMenu_saveLink() {
      invokeContextMenu("hyperlink");
      var model = webView.currentContextMenu.contextModel;
      model.saveLink();
      spy.wait();
      compare(webView.downloadUrl, "http://testsuite/empty.html");
      compare(webView.downloadReferrer,
              "http://testsuite/tst_WebView_contextMenu.html");
    }

    function test_WebView_contextMenu_saveMedia_data() {
      return [
        { id: "image", url: "http://testsuite/cof.svg", referrer: "http://testsuite/tst_WebView_contextMenu.html" },
        { id: "canvas", url: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAAKElEQVQ4T2NkoDJgpLJ5DKMGUh6io2E4GoZkhMBosiEj0NC0jMAwBABIxgAVO+SUsAAAAABJRU5ErkJggg==", referrer: "" },
        { id: "video", url: "http://testsuite/buddha.mp4", referrer: "http://testsuite/tst_WebView_contextMenu.html" },
        { id: "audio", url: "http://testsuite/fire.oga", referrer: "http://testsuite/tst_WebView_contextMenu.html" },
      ];
    }

    function test_WebView_contextMenu_saveMedia(data) {
      invokeContextMenu(data.id);
      var model = webView.currentContextMenu.contextModel;
      model.saveMedia();
      spy.wait();
      compare(webView.downloadUrl, data.url);
      compare(webView.downloadReferrer, data.referrer);
    }

    function test_WebView_contextMenu_editable() {
      invokeContextMenu("editable");
      var model = webView.currentContextMenu.contextModel;
      compare(model.selectionText, "");
      verify(!(model.editFlags & WebView.CanUndo));
      verify(!(model.editFlags & WebView.CanRedo));
      verify(!(model.editFlags & WebView.CanCut));
      verify(!(model.editFlags & WebView.CanCopy));
      verify(!(model.editFlags & WebView.CanErase));
      verify(model.editFlags & WebView.CanSelectAll);
      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      cleanup();

      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      compare(model.selectionText, "text area");
      verify(!(model.editFlags & WebView.CanUndo));
      verify(!(model.editFlags & WebView.CanRedo));
      verify(model.editFlags & WebView.CanCut);
      verify(model.editFlags & WebView.CanCopy);
      verify(model.editFlags & WebView.CanErase);
      verify(model.editFlags & WebView.CanSelectAll);
      webView.executeEditingCommand(WebView.EditingCommandErase);
      cleanup();
      var r = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#editable\").value");
      compare(r, "");

      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      compare(model.selectionText, "");
      verify(model.editFlags & WebView.CanUndo);
      verify(!(model.editFlags & WebView.CanRedo));
      webView.executeEditingCommand(WebView.EditingCommandUndo);
      cleanup();
      var r = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#editable\").value");
      compare(r, "text area");

      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      compare(model.selectionText, "text area");
      verify(!(model.editFlags & WebView.CanUndo));
      verify(model.editFlags & WebView.CanRedo);
      webView.executeEditingCommand(WebView.EditingCommandRedo);
      var r = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#editable\").value");
      compare(r, "");

      // TODO: once clipboard support is implemented
      // (https://launchpad.net/bugs/1301419), test cut/copy/paste
    }

    function test_WebView_contextMenu_saveLink_saveMedia() {
      invokeContextMenu("imagelink");
      var model = webView.currentContextMenu.contextModel;

      model.saveLink();
      spy.wait();
      compare(webView.downloadUrl, "http://testsuite/empty.html");
      compare(webView.downloadReferrer,
              "http://testsuite/tst_WebView_contextMenu.html");

      model.saveMedia();
      spy.wait();
      compare(webView.downloadUrl, "http://testsuite/cof.svg");
      compare(webView.downloadReferrer,
              "http://testsuite/tst_WebView_contextMenu.html");
    }

    function test_WebView_contextMenu_mediaFlags_data() {
      return [
        { id: "video", loop: true, canSave: true, hasAudio: false, controls: false },
        { id: "audio", loop: false, canSave: true, hasAudio: true, controls: true },
      ];
    }

    function test_WebView_contextMenu_mediaFlags(data) {
      invokeContextMenu(data.id);
      var model = webView.currentContextMenu.contextModel;
      compare(model.mediaFlags & WebView.MediaLoop,
              data.loop ? WebView.MediaLoop : WebView.MediaNone);
      compare(model.mediaFlags & WebView.MediaCanSave,
              data.canSave ? WebView.MediaCanSave : WebView.MediaNone);
      compare(model.mediaFlags & WebView.MediaHasAudio,
              data.hasAudio ? WebView.MediaHasAudio : WebView.MediaNone);
      compare(model.mediaFlags & WebView.MediaControls,
              data.controls ? WebView.MediaControls : WebView.MediaNone);
    }
  }
}
