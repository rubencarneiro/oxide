import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 1.0
import com.canonical.Oxide.Testing 1.0

TestWebView {
    id: webView

    focus: true

    width: 200
    height: 200

    function expect_content(expected, rangemax) {
        var result = webView.getTestApi().evaluateCode(
                    "return document.querySelector('#content').value", true);
        if (!rangemax) {
            rangemax = -1
        }
        return String(result).substr(0, rangemax) === expected.substr(0, rangemax);
    }

    function expect_has_file(expected) {
        var result = webView.getTestApi().evaluateCode(
                    "return document.querySelector('#has_file').innerHTML", true);
        result = result == 'true'
        return (result === expected);
    }

    function expect_mime_type(expected) {
        var result = webView.getTestApi().evaluateCode(
                    "return document.querySelector('#mime_type').innerHTML", true);
        return (result === expected);
    }

    function select_textarea_content() {
        webView.getTestApi().evaluateCode(
                    "document.querySelector('#content').select()", true);
    }

    function set_content(content) {
        webView.getTestApi().evaluateCode(
                    "document.querySelector('#content').value = '" + content + "'", true);
    }

    function get_image_data() {
        return "iVBORw0KGgoAAAANSUhEUgAAABAAAA\
AQCAYAAAAf8/9hAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL\
2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB9gHHRY6HX\
zuCtIAAAAZdEVYdENvbW1lbnQAQ3JlYXRlZCB3aXRoIEdJTVBXgQ\
4XAAAB6ElEQVQ4y33TT2jPcRzH8cfn9/1uMxsbWmsi1jS05cJuyF\
r7g4t22QgnWg6Sw5aLGwdJymFFTZKk0JTFKIkLByItsvIvSi2HsZ\
/N2n77OuxbfrF51/vw+fR5vnt/3q/XO/hPjLUKIShHlMwY+3RfCy\
5hBNvr+RjPB2fbbEUHahHhJjZjaZotuPBPganT1qNq8oEq7MMiCB\
mhqMytye824SceQfgLLsFdrJge1pz77Bw2YgwVyYyBHx88jorFpc\
u9TxKDcR5chBPYAnGtXbkvOpIZ2UX3yLapChldZTXOYiHehaAzk9\
fAmvRf8A0DhY2zMJQO+oqT6MMMqrEjZNu0YGco8bywQZPIfhwv6H\
ZinuGux0Wsw4sYvahJfnqT++JMtEonXv9H3fc4jCWYiDGOBJNiEW\
Isno/+OKgMe1J5P8c4iMZMueGoSicy2JkaZq7YgL2owO2Qp0IlLu\
cNcneSuFbYM3sYmjVTLa6jDlkc+GOkxIigF80IEhPDPdqHKE190J\
B2VpcSj/AwzOHEo1j7tlsfrqbywkSI9C9r8nB6TDT6VH99YmSuXT\
iPIhzC6rz7Zyu7PC2udgy/Kts90T1HgYJu4xgf4hRupFZeECIvi6\
sdQU36dBtezbuN9eQwnCY5priCVoziDvwGNw6PMb/zL+4AAAAASU\
VORK5CYII=";
        }

    TestCase {
        id: testcase
        name: "clipboard"

        when: windowShown

        function setup() {
            OxideTestingUtils.clearClipboard();
        }

        function test_paste_data() {
            return [
                        { content: "content", mimeType: "text/plain" },
                        { content: get_image_data(), mimeType: "image/png"},
                    ];
        }

        function test_paste(data) {
            var isImage = (data.mimeType.indexOf("image/") === 0)

            OxideTestingUtils.copyToClipboard(data.mimeType, data.content);

            webView.url = "http://testsuite/tst_Clipboard.html";

            verify(webView.waitForLoadSucceeded(), "Timed out waiting for successful load");

            if ( ! isImage) {
                select_textarea_content();
            }
            keyPress("v", Qt.ControlModifier)

            verify(webView.waitFor(function() { return expect_mime_type(data.mimeType); }));
            verify(webView.waitFor(function() { return expect_has_file(isImage); }));

            /**
              The image we get from QImage and the one we pasted are slightly different
              but overall is the same image. QImage does some "processing" on the raw image content
              that slightly alters it and make it hard to have an exact match.
             */
            verify(webView.waitFor(function() { return expect_content(data.content, 41); }));
        }

        function test_copy(data) {
            webView.url = "http://testsuite/tst_Clipboard.html";

            verify(webView.waitForLoadSucceeded(), "Timed out waiting for successful load");

            var data_content = "content"
            set_content(data_content)

            webView.waitFor(function() { return expect_content(data_content); });

            select_textarea_content();

            keyPress("c", Qt.ControlModifier)

            verify(webView.waitFor(function() {
                var current_content = OxideTestingUtils.getFromClipboard(
                            "text/plain");
                return current_content === data_content
            }));
        }
    }
}
