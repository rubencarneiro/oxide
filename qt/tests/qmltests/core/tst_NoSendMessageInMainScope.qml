import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
  id: webView
  focus: true
  width: 200
  height: 200

  TestCase {
    id: test
    name: "NoSendMessageInMainScope"
    when: windowShown

    function test_NoSendMessageInMainScope() {
      webView.url = "http://testsuite/tst_NoSendMessageInMainScope.html";
      verify(webView.waitForLoadSucceeded(),
             "Timed out waiting for successful load");

      var res;
      try {
        res = webView.getTestApi().evaluateCode("\
var el = document.querySelector(\".exception\");\
if (!el) throw Exception();\
return el.innerHTML;", true);
      } catch(e) {
        fail("Accessing oxide.sendMessage in the main JS scope should throw");
      }

      compare(res, "ReferenceError: oxide is not defined",
              "Unexpected error message");
    }
  }
}
