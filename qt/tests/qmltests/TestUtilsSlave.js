// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

// ==UserScript==
// @run-at document-start
// ==/UserScript==

oxide.addMessageHandler("GET-DOCUMENT-URI", function(msg) {
  msg.reply({location: window.document.documentURI});
});

oxide.addMessageHandler("EVALUATE-CODE", function(msg) {
  var code = msg.payload.code;
  if (msg.payload.wrap) {
    code = "(function() {" + code + "})()";
  }
  console.log("EVALUATE-CODE: " + code);
  try {
    msg.reply({result: eval(code)});
  } catch(e) {
    msg.error("Code threw exception: \"" + e + "\"");
  }
});

oxide.addMessageHandler("GET-BOUNDING-CLIENT-RECT", function(msg) {
  var e = document.querySelector(msg.payload);
  if (!e) {
    msg.error("No element found");
    return;
  }

  var r = e.getBoundingClientRect();
  msg.reply({x: r.left, y: r.top, width: r.width, height: r.height});
});
