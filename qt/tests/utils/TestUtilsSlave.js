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

oxide.addMessageHandler("GET-DOCUMENT-URI", function(msg) {
  msg.reply({location: window.document.documentURI});
});

oxide.addMessageHandler("SET-DOCUMENT-TITLE", function(msg) {
  window.document.title = msg.args.title;
});

oxide.addMessageHandler("EVALUATE-CODE", function(msg) {
  var code = msg.args.code;
  if (msg.args.wrap) {
    code = "(function() {" + code + "})()";
  }
  try {
    msg.reply({result: eval(code)});
  } catch(e) {
    msg.error("Caught exception: \"" + e + "\"");
  }
});

oxide.addMessageHandler("GET-BOUNDING-CLIENT-RECT", function(msg) {
  try {
    var r = document.querySelector(msg.args.selector).getBoundingClientRect();
    msg.reply({x: r.left, y: r.top, width: r.width, height: r.height});
  } catch(e) {
    msg.error("Caught exception: \"" + e + "\"");
  }
});
