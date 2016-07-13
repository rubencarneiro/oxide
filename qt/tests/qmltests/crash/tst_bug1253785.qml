import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0

WebView {
  id: webview
  url: "about:blank"

  TestCase {
    id: test
    name: "bug1253785"
    when: windowShown
  }

  readonly property bool test: loading
}
