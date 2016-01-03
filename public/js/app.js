$(document).foundation();

build_field = function(fld, val, base) {
  var result = "";

  if (typeof(val) == "object") {
    result += "<fieldset class=\"callout light-gray\">";
    result += "<h5>" + fld + "</h5>";
    result += Object.keys(val).map(function(key) {
      return build_field(key, val[key], base.concat([fld]));
    }).join("")
    result += "</fieldset>";
  } else {
    na = base.concat([fld]);
    name = na[0] + na.slice(1).map(function(v) { return "[" + v + "]"; }).join("");
    result += "<div class=\"row\"><div class=\"large-12 columns\">";
    result += "<label>" + fld + "</label>";
    result += "<input type=\"text\" name=\"" + name + ":auto\" value=\"" + val + "\" />";
    result += "</div></div>";
  }

  return result;
}

build_form = function(json) {
  $("div.formy").html(
      "<form class=\"configuration\">" +
      Object.keys(json).map(function(key) { return build_field(key, json[key], []); }).join("") + 
      "<input class=\"button small large-12\" type=\"submit\" value=\"Send Message\" />" +
      "</form>"
      );

  $("form.configuration").submit(function() { 
  });
}

teste = function(name) {
  $.getJSON(name, build_form);
}

$(document).ready(function() {
  $.getJSON("/api/config/sirius", build_form);
});
