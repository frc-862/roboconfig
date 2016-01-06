$(document).foundation();

var first = true;
var data = [[0,0,0]];
var dg;

add_point = function(lines) {
  var rec = lines.shift();

  // do stuff
  data.push([rec.at, rec.left_power, rec.right_power]);
  dg.updateOptions({ 'file': data });
  
  if (lines.length > 0)
    setTimeout(add_point, 1, lines);
}

$(document).ready(function() {
  var viz = $("#viz");
  var width = viz.width();
  viz.height(width / 1.5);

  console.log("Ready => ");
  console.log(data);
  dg = new Dygraph(document.getElementById("viz"), data);

  var ws = new WebSocket('ws://' + location.host + '/tail/d2.log');
  ws.onmessage = function(ev) {
    if (first) { data = []; first = false; }

    var lines = JSON.parse(ev.data);
    lines.forEach(function(rec) {
      data.push([rec.at, rec.left_power, rec.right_power]);
    });
    dg.updateOptions({ 'file': data });
  };
});
