$(document).foundation();

$(document).ready(function() {
  var viz = $("#viz");
  var width = viz.width();
  viz.height(width / 1.5);
  $.plot(viz, [ [[0, 0], [1, 1]] ], { yaxis: { max: 1 } });
});
