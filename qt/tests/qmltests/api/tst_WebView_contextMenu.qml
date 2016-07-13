import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.8
import Oxide.testsupport 1.0

TestWebView {
  id: webView
  focus: true

  property var currentContextMenu: null
  property url downloadUrl
  property string downloadMimeType
  property string downloadReferrer

  contextMenu: Item {
    property var contextModel: model
    Component.onCompleted: WebView.view.currentContextMenu = this;
    Component.onDestruction: WebView.view.currentContextMenu = null;
  }

  function waitForContextMenu() {
    return TestUtils.waitFor(function() { return currentContextMenu != null; });
  }

  onDownloadRequested: {
    downloadUrl = request.url;
    downloadMimeType = request.mimeType;
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
      webView.currentContextMenu.contextModel.close();
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

    function test_WebView_contextMenu_copyImage() {
      ClipboardTestUtils.clearClipboard();
      verify(!ClipboardTestUtils.hasImage());
      invokeContextMenu("image");
      webView.currentContextMenu.contextModel.copyImage();
      verify(TestUtils.waitFor(ClipboardTestUtils.hasImage));
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
        { id: "image", url: "http://testsuite/cof.svg", referrer: "http://testsuite/tst_WebView_contextMenu.html", mimeType: "image/svg+xml" },
        { id: "canvas", url: "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAAKElEQVQ4T2NkoDJgpLJ5DKMGUh6io2E4GoZkhMBosiEj0NC0jMAwBABIxgAVO+SUsAAAAABJRU5ErkJggg==", referrer: "", mimeType: "" },
        { id: "video", url: "http://testsuite/buddha.mp4", referrer: "http://testsuite/tst_WebView_contextMenu.html", mimeType: "" },
        { id: "audio", url: "http://testsuite/fire.oga", referrer: "http://testsuite/tst_WebView_contextMenu.html", mimeType: "" },
      ];
    }

    function test_WebView_contextMenu_saveMedia(data) {
      invokeContextMenu(data.id);
      var model = webView.currentContextMenu.contextModel;
      model.saveMedia();
      spy.wait();
      compare(webView.downloadUrl, data.url);
      compare(webView.downloadMimeType, data.mimeType);
      compare(webView.downloadReferrer, data.referrer);
    }

    function test_WebView_contextMenu_editable() {
      invokeContextMenu("editable");
      var model = webView.currentContextMenu.contextModel;
      compare(model.selectionText, "");
      verify(!(model.editFlags & WebView.UndoCapability));
      verify(!(model.editFlags & WebView.RedoCapability));
      verify(!(model.editFlags & WebView.CutCapability));
      verify(!(model.editFlags & WebView.CopyCapability));
      verify(model.editFlags & WebView.PasteCapability);
      verify(!(model.editFlags & WebView.EraseCapability));
      verify(model.editFlags & WebView.SelectAllCapability);
      cleanup();

      webView.executeEditingCommand(WebView.EditingCommandSelectAll);
      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      compare(model.selectionText, "text area");
      verify(!(model.editFlags & WebView.UndoCapability));
      verify(!(model.editFlags & WebView.RedoCapability));
      verify(model.editFlags & WebView.CutCapability);
      verify(model.editFlags & WebView.CopyCapability);
      verify(model.editFlags & WebView.PasteCapability);
      verify(model.editFlags & WebView.EraseCapability);
      verify(model.editFlags & WebView.SelectAllCapability);
      cleanup();

      webView.executeEditingCommand(WebView.EditingCommandErase);
      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      compare(model.selectionText, "");
      verify(model.editFlags & WebView.UndoCapability);
      verify(!(model.editFlags & WebView.RedoCapability));
      cleanup();

      webView.executeEditingCommand(WebView.EditingCommandUndo);
      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      compare(model.selectionText, "text area");
      verify(!(model.editFlags & WebView.UndoCapability));
      verify(model.editFlags & WebView.RedoCapability);
      cleanup();

      webView.executeEditingCommand(WebView.EditingCommandRedo);
      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      compare(model.selectionText, "");
      verify(model.editFlags & WebView.UndoCapability);
      verify(!(model.editFlags & WebView.RedoCapability));
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
      compare(model.mediaFlags & WebView.MediaStatusLoop,
              data.loop ? WebView.MediaStatusLoop : WebView.MediaStatusNone);
      compare(model.mediaFlags & WebView.MediaStatusCanSave,
              data.canSave ? WebView.MediaStatusCanSave : WebView.MediaStatusNone);
      compare(model.mediaFlags & WebView.MediaStatusHasAudio,
              data.hasAudio ? WebView.MediaStatusHasAudio : WebView.MediaStatusNone);
      compare(model.mediaFlags & WebView.MediaStatusControls,
              data.controls ? WebView.MediaStatusControls : WebView.MediaStatusNone);
    }
  }
}
