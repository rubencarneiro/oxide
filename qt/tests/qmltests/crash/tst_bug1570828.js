// ==UserScript==
// @run-at document-start
// ==/UserScript==

document.addEventListener("oxideunloadevent", function(event) {
  oxide.sendMessage("TEST", null);
});
