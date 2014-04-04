import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0

WebView {
  id: webview
  width: 800
  height: 600

  TestCase {
    id: test
    name: "bug1253964"
    when: windowShown
  }
}
