import QtQuick 2.0
import QtTest 1.0
import com.canonical.Oxide 0.1
import com.canonical.Oxide.Testing 0.1 as Testing

Item {
  SignalSpy {
    id: spy
    target: OxideSettings
  }

  Component {
    id: webView
    WebView {}
  }

  TestCase {
    id: test
    name: "OxideSettings"
    when: windowShown

    function waitFor(predicate, timeout) {
      timeout = timeout || 5000;
      var i = 0;
      while (i < timeout && !predicate()) {
        wait(50);
        i += 50;
      }
      return predicate();
    }

    function init() {
      spy.clear();
      spy.signalName = "";
    }

    function test_OxideSettings_data() {
      var r = [
        { attr: "product", signal: "productChanged", value1: "Test1", value2: "Test2", constructOnly: false },
        { attr: "userAgent", signal: "userAgentChanged", value1: "Test1", value2: "Test2", contructOnly: false },
        { attr: "acceptLangs", signal: "acceptLangsChanged", value1: "foo", value2: "bar", constructOnly: false }
      ];

      var dataPath = Testing.Utils.DATA_PATH;
      if (dataPath != "") {
        r.push(
            { attr: "dataPath", signal: "dataPathChanged", value1: dataPath + "/test1", value2: dataPath + "/test2", constructOnly: true },
            { attr: "cachePath", signal: "cachePathChanged", value1: dataPath + "/test1", value2: dataPath + "/test2", constructOnly: true }
        );
      }

      return r;
    }

    function test_OxideSettings(data) {
      spy.signalName = data.signal;

      OxideSettings[data.attr] = data.value1;
      compare(spy.count, 1,
              "Setting attribute before context exists should have generated a signal");
      compare(OxideSettings[data.attr], data.value1,
              "Got the wrong value back");

      var view = webView.createObject(null, {});
      compare(view.context[data.attr], data.value1,
              "Got the wrong value back from the default context");

      var expectedCount = 1;
      var expectedVal;

      OxideSettings[data.attr] = data.value2;
      expectedVal = data.constructOnly ? data.value1 : data.value2;
      if (!data.constructOnly) {
        expectedCount++;
      }
      compare(spy.count, expectedCount, "Unexpected number of signals");
      compare(OxideSettings[data.attr], expectedVal,
              "Got the wrong value back");
      compare(view.context[data.attr], expectedVal,
              "Got the wrong value back from the default context");

      if (!data.constructOnly) {
        view.context[data.attr] = data.value1;
        compare(spy.count, expectedCount + 1,
                "Setting attribute on the default context should generate a signal");
        compare(OxideSettings[data.attr], data.value1,
                "Got the wrong value back");
      }

      var destructionObs = Testing.Utils.createDestructionObserver(view.context);
      view.destroy();
      gc();
      verify(waitFor(function() { return destructionObs.destroyed; }),
             "Timed out waiting for WebView to be destroyed");

      compare(OxideSettings[data.attr], data.value1,
              "Got the wrong value back");
    }
  }
}
