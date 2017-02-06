// ==UserScript==
// @run-at document-start
// ==/UserScript==

document.addEventListener("alert-response", (event) => {
  oxide.sendMessage("alert-response", null);
});
