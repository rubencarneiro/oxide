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

.pragma library

function TestApiHost(webview) {
  this._webview = webview;
}

TestApiHost.prototype = {
  get documentURI() {
    var result;
    var got_error = false;
    var got_result = false;
    var r = this._webview.rootFrame.sendMessage("TestUtils",
                                                "GET-DOCUMENT-URI",
                                                {});
    r.onreply = function(response) {
      result = response.location;
      got_result = true;
    };
    r.onerror = function(msg) {
      result = msg;
      got_error = true;
    }

    this._webview.waitFor(function() { return got_result || got_error; });

    if (got_error) {
      throw Error(result);
    } else if (got_result) {
      return result;
    } else {
      throw Error("Message call timed out");
    }
  },

  set documentTitle(title) {
    this._webview.rootFrame.sendMessageNoReply("TestUtils",
                                               "SET-DOCUMENT-TITLE",
                                               { title: title });
  }
};
