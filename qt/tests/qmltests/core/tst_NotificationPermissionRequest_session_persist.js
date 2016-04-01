// ==UserScript==
// @run-at document-start
// ==/UserScript==
document.addEventListener("result", function(event) {
  oxide.sendMessage("TEST-RESPONSE", event.detail.status);
});
