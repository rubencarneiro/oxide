// ==UserScript==
// @run-at document-start
// ==/UserScript==

document.addEventListener("oxidegumresult", function(event) {
  oxide.sendMessage("GUM-RESPONSE", { error: event.detail.error });
});
