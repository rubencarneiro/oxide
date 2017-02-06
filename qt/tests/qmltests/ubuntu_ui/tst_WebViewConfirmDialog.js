// ==UserScript==
// @run-at document-start
// ==/UserScript==

document.addEventListener("confirm-response", (event) => {
  oxide.sendMessage("confirm-response", event.detail);
});
