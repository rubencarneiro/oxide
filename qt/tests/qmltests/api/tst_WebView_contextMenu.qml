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

    function test_WebView_contextMenu_text() {
      invokeContextMenu("text");
      var model = webView.currentContextMenu.contextModel;
      compare(model.linkUrl, "");
      compare(model.srcUrl, "");
      verify(!model.isEditable);
    }

    function test_WebView_contextMenu_hyperlink() {
      invokeContextMenu("hyperlink");
      var model = webView.currentContextMenu.contextModel;
      verify(!model.isEditable);
      compare(model.linkUrl, "http://testsuite/empty.html");
      compare(model.linkText, "super-link");
      model.saveLink();
      spy.wait();
      compare(webView.downloadUrl, "http://testsuite/empty.html");
      compare(webView.downloadReferrer,
              "http://testsuite/tst_WebView_contextMenu.html");
    }

    function test_WebView_contextMenu_image() {
      invokeContextMenu("image");
      var model = webView.currentContextMenu.contextModel;
      verify(!model.isEditable);
      verify(model.hasImageContents);
      compare(model.srcUrl, "http://testsuite/cof.svg");
      model.saveImage();
      spy.wait();
      compare(webView.downloadUrl, "http://testsuite/cof.svg");
      compare(webView.downloadReferrer,
              "http://testsuite/tst_WebView_contextMenu.html");
    }

    function test_WebView_contextMenu_canvas() {
      invokeContextMenu("canvas");
      var model = webView.currentContextMenu.contextModel;
      verify(!model.isEditable);
      verify(model.hasImageContents);
      compare(model.srcUrl, "");
      model.saveImage();
      spy.wait();
      compare(webView.downloadUrl.toString().slice(0, 11), "data:image/");
      compare(webView.downloadReferrer, "");
    }

    function test_WebView_contextMenu_editable() {
      invokeContextMenu("editable");
      var model = webView.currentContextMenu.contextModel;
      verify(model.isEditable);
      compare(model.selectionText, "");
      verify(!model.canUndo);
      verify(!model.canRedo);
      verify(!model.canCut);
      verify(!model.canCopy);
      verify(!model.canErase);
      verify(model.canSelectAll);
      model.selectAll();
      cleanup();

      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      verify(model.isEditable);
      compare(model.selectionText, "text area");
      verify(!model.canUndo);
      verify(!model.canRedo);
      verify(model.canCut);
      verify(model.canCopy);
      verify(model.canErase);
      verify(model.canSelectAll);
      model.erase();
      cleanup();
      var r = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#editable\").value");
      compare(r, "");

      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      verify(model.isEditable);
      compare(model.selectionText, "");
      verify(model.canUndo);
      verify(!model.canRedo);
      model.undo();
      cleanup();
      var r = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#editable\").value");
      compare(r, "text area");

      invokeContextMenu("editable");
      model = webView.currentContextMenu.contextModel;
      verify(model.isEditable);
      compare(model.selectionText, "text area");
      verify(!model.canUndo);
      verify(model.canRedo);
      model.redo();
      var r = webView.getTestApi().evaluateCode(
          "document.querySelector(\"#editable\").value");
      compare(r, "");

      // TODO: once clipboard support is implemented
      // (https://launchpad.net/bugs/1301419), test cut/copy/paste
    }

    function test_WebView_contextMenu_iframe() {
      invokeContextMenu("iframe");
      var model = webView.currentContextMenu.contextModel;
      verify(!model.isEditable);
      compare(model.srcUrl, "");
      compare(model.frameUrl, "http://testsuite/empty.html");
      compare(model.frameCharset.toLowerCase(), "utf-8");
    }
  }
}
