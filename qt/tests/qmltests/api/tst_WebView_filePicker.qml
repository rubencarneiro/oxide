import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  property var currentFilePicker: null
  property var filenames: []

  Component {
    id: filePickerComponent
    Item {
      id: filePicker
      readonly property bool allowMultipleFiles: model.allowMultipleFiles
      readonly property string defaultFileName: model.defaultFileName
      readonly property var acceptTypes: model.acceptTypes
      anchors.fill: parent
      MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
          if (mouse.button == Qt.LeftButton) {
            model.accept(webView.filenames);
          } else if (mouse.button == Qt.RightButton) {
            model.reject();
          }
        }
      }
      Component.onCompleted: {
        WebView.view.currentFilePicker = filePicker;
      }
      Component.onDestruction: {
        WebView.view.currentFilePicker = null;
      }
    }
  }

  function filePickerShown() {
    return (currentFilePicker != null);
  }

  function filePickerDismissed() {
    return (currentFilePicker == null);
  }

  function resolvedFilepath(filename) {
    var filepath = Qt.resolvedUrl(filename).toString();
    var scheme = filepath.indexOf("://");
    if (scheme >= 0) {
      filepath = filepath.substr(scheme + 3);
    }
    return filepath;
  }

  TestCase {
    id: test
    name: "WebView_filePicker"
    when: windowShown

    function test_noFilePickerComponent() {
      webView.filePicker = null;
      webView.url = "http://localhost:8080/tst_WebView_filePicker_single.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      webView.filenames = [];
      mouseClick(webView, webView.width / 2, webView.height / 2);
      wait(1000);
      compare(webView.currentFilePicker, null);
    }

    function test_customFilePickerComponent_data() {
      return [
        { page: "tst_WebView_filePicker_single.html", allowMultipleFiles: false, acceptTypes: ["text/plain", "text/html"], filenames: ["tst_WebView_filePicker_data.txt"], button: Qt.LeftButton, selected: 1 },
        { page: "tst_WebView_filePicker_multiple.html", allowMultipleFiles: true, acceptTypes: ["text/plain", "text/html"], filenames: ["tst_WebView_filePicker_data.html", "tst_WebView_filePicker_data.txt"], button: Qt.LeftButton, selected: 2 },
        { page: "tst_WebView_filePicker_single.html", allowMultipleFiles: false, acceptTypes: ["text/plain", "text/html"], filenames: ["tst_WebView_filePicker_data.html", "tst_WebView_filePicker_data.txt"], button: Qt.LeftButton, selected: 1 },
        { page: "tst_WebView_filePicker_single.html", allowMultipleFiles: false, acceptTypes: ["text/plain", "text/html"], filenames: [], button: Qt.RightButton, selected: 0 }
      ];
    }

    function test_customFilePickerComponent(data) {
      webView.filePicker = filePickerComponent;
      webView.url = "http://localhost:8080/" + data.page;
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");
      mouseClick(webView, webView.width / 2, webView.height / 2);
      verify(webView.waitFor(webView.filePickerShown), "File picker not shown");
      var filePicker = webView.currentFilePicker;
      compare(filePicker.width, webView.width);
      compare(filePicker.height, webView.height);
      compare(filePicker.allowMultipleFiles, data.allowMultipleFiles);
      compare(filePicker.acceptTypes, data.acceptTypes);
      webView.filenames = data.filenames.map(resolvedFilepath);
      mouseClick(filePicker, filePicker.width / 2, filePicker.height / 2,
                 data.button);
      verify(webView.waitFor(webView.filePickerDismissed),
             "File picker not dismissed");
      compare(webView.getTestApi().evaluateCode(
              "document.querySelector(\"#filePicker\").files.length"),
              data.selected);
      for (var i = 0; i < data.selected; ++i) {
        compare(webView.getTestApi().evaluateCode(
            "document.querySelector(\"#filePicker\").files[%1].name".arg(i)),
            data.filenames[i]);
      }
    }
  }
}
