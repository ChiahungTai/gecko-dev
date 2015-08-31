# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

var Ci = Components.interfaces;

var gShowRecognitizedText = {
  _text           : null,

  load: function ()
  {
#ifdef XP_MACOSX
    document.documentElement.getButton("accept").hidden = true;
#endif
    _text = document.getElementById("textResult");
    // make sure that the correct dimensions will be used
    setTimeout(function(self) {
      self.init(window.arguments[0]);
    }, 0, this);
  },

  init: function (aText)
  {
    _text.label = aText;
  },

  goSearch: function ()
  {
    var search = "https://www.google.com/search?q=" + _text.label;
    openUILinkIn(search, "tab", {});
  }
};
