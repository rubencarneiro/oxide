import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0

TestCase {
  name: "bug1338639"
  when: windowShown

  Component {
    id: webviewComponent
    WebView {
      property string html
      onHtmlChanged: loadHtml(html, "file:///");
    }
  }

  function test_bug1338639() {
    webviewComponent.createObject(this, {html: "<html></html>"});
  }
}
