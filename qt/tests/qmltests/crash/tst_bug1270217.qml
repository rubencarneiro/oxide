import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1

Item {
  WebView {}
  WebView {}

  TestCase {
    id: test
    name: "bug1270217"
    when: windowShown
  }
}
