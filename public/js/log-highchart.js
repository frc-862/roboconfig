$(document).foundation();

add_point = function(lines) {
  var rec = lines.shift();
  console.log("add_point");
  console.log(rec);
  window.series.addPoint([rec.at, rec.left_power], true, true);

  if (lines.length > 0)
    setTimeout(add_point, 40, lines);
}

$(document).ready(function() {
  var viz = $("#viz");
  var width = viz.width();
  viz.height(width / 1.5);

         viz.highcharts({
            chart: {
                zoomType: 'x',
                  events: {
                    load: function () {

                        // set up the updating of the chart each second
                        window.series = this.series[0];
                        //setInterval(function () {
                            //var x = (new Date()).getTime(), // current time
                                //y = Math.random();
                            //series.addPoint([x, y], true, true);
                        //}, 1000);
                    }
                }
            },
            title: {
                text: 'Robot data'
            },
            subtitle: {
                text: document.ontouchstart === undefined ?
                        'Click and drag in the plot area to zoom in' : 'Pinch the chart to zoom in'
            },
            xAxis: {
                not_type: 'datetime',
                min: 4749636,
                max: 4782700
            },
            yAxis: {
                title: {
                    text: 'Power'
                }
            },
            legend: {
                enabled: false
            },
            plotOptions: {
                area: {
                    fillColor: {
                        linearGradient: {
                            x1: 0,
                            y1: 0,
                            x2: 0,
                            y2: 1
                        },
                        stops: [
                            [0, Highcharts.getOptions().colors[0]],
                            [1, Highcharts.Color(Highcharts.getOptions().colors[0]).setOpacity(0).get('rgba')]
                        ]
                    },
                    marker: {
                        radius: 2
                    },
                    lineWidth: 1,
                    states: {
                        hover: {
                            lineWidth: 1
                        }
                    },
                    threshold: null
                }
            },

            series: [{
                type: 'line',
                name: '862 Plot',
                data: [[4749636,0],[4749637,1]]
            }]
        });

  var ws = new WebSocket('ws://' + location.host + '/tail/d2.log');

  if (!window.console) { window.console = { log: function() {} } };

  ws.onopen = function(ev)  { console.log("Open" + ev); };
  ws.onerror = function(ev) { console.log("Error" + ev); };
  ws.onclose = function(ev) { console.log("Close" + ev); };
  ws.onmessage = function(ev) {
    var lines = JSON.parse(ev.data);
    setTimeout(add_point, 0, lines);
    //lines.forEach(function(datum) {
      //console.log(datum.at);
      //console.log(datum.left_power);
      //window.series.addPoint([datum.at, datum.left_power], true, true);
    //});
  };

         
});
