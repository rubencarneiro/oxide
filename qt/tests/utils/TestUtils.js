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

function TestApiHost(webview, frame) {
  this._webview = webview;
  this._frame = frame;
}

TestApiHost.prototype = {
  waitForResult: function(req, timeout) {
    var result;
    var got_error = false;
    var got_result = false;

    req.onreply = function(response) {
      got_result = true;
      result = response;
    };
    req.onerror = function(error, msg) {
      got_error = true;
      result = msg;
    };

    this._webview.waitFor(function() { return got_result || got_error; },
                          timeout);

    if (got_error) {
      throw Error(result);
    } else if (got_result) {
      return result;
    } else {
      throw Error("Message call timed out");
    }
  },

  get documentURI() {
    return this.waitForResult(
        this._frame.sendMessage("TestUtils",
                                "GET-DOCUMENT-URI",
                                {})).location;
  },

  evaluateCode: function(code, wrap) {
    return this.waitForResult(
        this._frame.sendMessage(
          "TestUtils",
          "EVALUATE-CODE",
          { code: code,
            wrap: wrap === undefined ? false : wrap })).result;
  },

  getBoundingClientRectForSelector: function(selector) {
    return this.waitForResult(
        this._frame.sendMessage("TestUtils",
                                "GET-BOUNDING-CLIENT-RECT",
                                { selector: selector }));
  },

  sendMessageToSelf: function(id, args) {
    return this.waitForResult(
        this._frame.sendMessage("TestUtils",
                                "SEND-MESSAGE-TO-SELF",
                                { id: id, args: args } ));
  }
};
