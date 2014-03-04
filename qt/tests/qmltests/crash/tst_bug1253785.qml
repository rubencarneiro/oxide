import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1

WebView {
  id: webview
  width: 800
  height: 600
  url: "about:blank"

  TestCase {
    id: test
    name: "bug1253785"
    when: windowShown
  }

  readonly property bool test: loading
}
