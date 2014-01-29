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

function MessageError(error, desc) {
  this.error = error;
  this.message = desc;
}

function TestApiHost(webview, frame) {
  this._webview = webview;
  this._frame = frame;
}

TestApiHost.prototype = {
  waitForResult: function(req, timeout) {
    var result;
    var error;

    req.onreply = function(response) {
      result = response;
      error = 0;
    };
    req.onerror = function(error_code, msg) {
      result = msg;
      error = error_code;
    };

    this._webview.waitFor(function() { return error !== undefined; },
                          timeout);

    if (error > 0) {
      throw new MessageError(error, result);
    } else if (error === 0) {
      return result;
    } else {
      throw new Error("Message call timed out");
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
    var r = this.waitForResult(
        this._frame.sendMessage("TestUtils",
                                "SEND-MESSAGE-TO-SELF",
                                { id: id, args: args } ));
    if (r.error > 0) {
      throw new MessageError(r.error, r.response);
    } else {
      return r.response;
    }
  }
};
