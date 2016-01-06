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
      "<input class=\"button large large-12\" type=\"submit\" value=\"Save Configuration\" />" +
      "</form>"
      );

  $("form.configuration").submit(save_form);
}

save_form = function() {
  console.log(JSON.stringify($("form.configuration").serializeJSON()));
  return false;
}

build_config_list = function(json) {
  $("div.configurations").html(
      json.map(function(txt) { return "<a href=\"#\" class=\"medium load_config button large-12 medium-12\">" + txt + "</a><br/>"; }).join("")
  );

  $("a.load_config").click(function() {
    var conf = $(this).text();
    $.getJSON("/api/config/" + conf, build_form);
    return false;
  });
}

teste = function(name) {
  $.getJSON(name, build_form);
}

$(document).ready(function() {
  $.getJSON("/api/config/sirius", build_form);
  $.getJSON("/api/config", build_config_list);
});
