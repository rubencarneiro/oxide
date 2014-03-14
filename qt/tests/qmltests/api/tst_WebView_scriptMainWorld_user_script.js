try {
  oxide.sendMessage("FOO", {});
  document.querySelector("#result").innerHTML =
	"Main world content script found oxide.postMessage";
} catch(e) {
  document.querySelector("#result").innerHTML =
	"Main world content script DID NOT found oxide.postMessage";
}

