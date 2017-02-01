// ==UserScript==
// @run-at document-start
// ==/UserScript==

document.addEventListener("prompt-response", (event) => {
  oxide.sendMessage("prompt-response", event.detail);
});
