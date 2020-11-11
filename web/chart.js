async function getDevices() {
    try {
        const response = await axios.get("./get-iotdata");
        return response.data;
    } catch (error) {
        console.error(error);
    }
}

async function showUptime() {
    try {
        const response = await axios.get("./get-uptime");
        var sec_num = response.data["uptime"]; // don't forget the second param
        var hours   = Math.floor(sec_num / 3600);
        var minutes = Math.floor((sec_num - (hours * 3600)) / 60);
        var seconds = sec_num - (hours * 3600) - (minutes * 60);
    
        if (hours   < 10) {hours   = "0"+hours;}
        if (minutes < 10) {minutes = "0"+minutes;}
        if (seconds < 10) {seconds = "0"+seconds;}
        document.getElementById("uptime").innerHTML = "Uptime is: " + hours + ':' + minutes + ':' + seconds;
    } catch (error) {
        console.error(error);
    }
}
window.setInterval(function(){
    showUptime();
  }, 1000);


function RemoveDevice(handle, name, param) {
    document.getElementById('handle').innerHTML = handle;
    document.getElementById('name').innerHTML = name;
    document.getElementById('parametr').innerHTML = param;
}

function RemoveDeviceURL() {
    let handle = document.getElementById('handle').innerHTML;
    let result = document.querySelector('.result');
    let alert = document.getElementById("alert")
    let xhr = new XMLHttpRequest();
    xhr.open("GET", "/remove-observable/" + handle, true); 
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
          //result.innerHTML = this.responseText;
          //alert.hidden = false;
          //alert.classList.add("bg-success");
          console.log(this.responseText);
        } else {
          //result.innerHTML = this.responseText;
          //alert.hidden = false;
          //alert.classList.add("bg-danger");
          console.log(this.responseText);
        }
    }; 
    var data = JSON.stringify({
    }); 
    console.log(data);
    xhr.send(data); 
}
update();
window.setInterval(function(){
    update();
}, 10000);


function update() {
am4core.ready(async function () {
    // Themes begin
    am4core.useTheme(am4themes_material);
    am4core.useTheme(am4themes_animated);
    var count_val = 2;
    const chart = am4core.create("chartdiv", am4charts.XYChart);
    const devices = await getDevices();
    let dateAxis = chart.xAxes.push(new am4charts.DateAxis());
    dateAxis.renderer.minGridDistance = 60;
    let valueAxis = chart.yAxes.push(new am4charts.ValueAxis());
    chart.legend = new am4charts.Legend();
    chart.legend.scrollable = true;
    chart.legend.valueLabels.template.align = "left";
    chart.legend.valueLabels.template.textAlign = "end"; 
    addSeries(count_val);
    chart.cursor = new am4charts.XYCursor();
    chart.cursor.xAxis = dateAxis;
    chart.focusable = true;
    chart.scrollbarX = new am4core.Scrollbar();

    function addSeries() {
        //window.addSeries = function() {
        for ( var prop in devices["observables"] ){
            chart.data = devices["observables"][prop]["values"];
            chart.dateFormatter.inputDateFormat = "yyyy-MM-DDTHH:mm:ssZ";
            chart.dataDateFormat = "yyyy-MM-DDTHH:mm:ssZ";
            chart.numberFormatter.numberFormat = "#,###.##";

            var series = new am4charts.LineSeries();
            series.data = chart.data;
            series.dataFields.valueY = "value";
            series.dataFields.dateX = "date";
            series.name =  devices["observables"][prop]["name"];
            series.strokeWidth = 3;
            series.tensionX = 0.7;
            series.fillOpacity = 0.2;
            series.stacked = false;  
            series.smoothing = "monotoneX";
            series.tooltip.pointerOrientation = "vertical";
            series.tooltipText = devices["observables"][prop]["param"] + " : {valueY.formatNumber('#.##')}";
            series.legendSettings.labelText = devices["observables"][prop]["name"] + "\n" + devices["observables"][prop]["param"] + " : [bold {color}]{value.formatNumber('#.##')}[/]";
            let bullet = series.bullets.push(new am4charts.CircleBullet());
            bullet.circle.stroke = am4core.color("#000");
            bullet.circle.strokeWidth = 1;
            series = chart.series.push(series);
        }
    };
    var chart2 = am4core.create("chartdivtest", am4charts.XYChart);
    chart.hiddenState.properties.opacity = 0;
    chart.padding(0, 0, 0, 0);
    chart.zoomOutButton.disabled = true;

    var data = [];
    var visits = 10;
    var i = 0;

    for (i = 0; i <= 30; i++) {
        visits -= Math.round((Math.random() < 0.5 ? 1 : -1) * Math.random() * 10);
        data.push({ date: new Date().setSeconds(i - 30), value: visits });
    }

    chart2.data = data;

    var dateAxis2 = chart2.xAxes.push(new am4charts.DateAxis());
    dateAxis2.renderer.grid.template.location = 0;
    dateAxis2.renderer.minGridDistance = 30;
    dateAxis2.dateFormats.setKey("second", "ss");
    dateAxis2.periodChangeDateFormats.setKey("second", "[bold]h:mm a");
    dateAxis2.periodChangeDateFormats.setKey("minute", "[bold]h:mm a");
    dateAxis2.periodChangeDateFormats.setKey("hour", "[bold]h:mm a");
    dateAxis2.renderer.inside = true;
    dateAxis2.renderer.axisFills.template.disabled = true;
    dateAxis2.renderer.ticks.template.disabled = true;

    var valueAxis2 = chart2.yAxes.push(new am4charts.ValueAxis());
    valueAxis2.tooltip.disabled = true;
    valueAxis2.interpolationDuration = 500;
    valueAxis2.rangeChangeDuration = 500;
    valueAxis2.renderer.inside = true;
    valueAxis2.renderer.minLabelPosition = 0.05;
    valueAxis2.renderer.maxLabelPosition = 0.95;
    valueAxis2.renderer.axisFills.template.disabled = true;
    valueAxis2.renderer.ticks.template.disabled = true;

    var series = chart2.series.push(new am4charts.LineSeries());
    series.dataFields.dateX = "date";
    series.dataFields.valueY = "value";
    series.interpolationDuration = 500;
    series.defaultState.transitionDuration = 0;
    series.tensionX = 0.8;
    series.strokeWidth = 3;

    chart2.events.on("datavalidated", function () {
        dateAxis2.zoom({ start: 1 / 15, end: 1.2 }, false, true);
    });

    dateAxis2.interpolationDuration = 500;
    dateAxis2.rangeChangeDuration = 500;

    document.addEventListener("visibilitychange", function() {
        if (document.hidden) {
            if (interval) {
                clearInterval(interval);
            }
        }
        else {
            startInterval();
        }
    }, false);

    // add data
    var interval;
function startInterval() {
    interval = setInterval(function() {
        visits =
            visits + Math.round((Math.random() < 0.5 ? 1 : -1) * Math.random() * 5);
        var lastdataItem = series.dataItems.getIndex(series.dataItems.length - 1);
        chart2.addData(
            { date: new Date(lastdataItem.dateX.getTime() + 1000), value: visits },
            1
        );
    }, 1000);
}

startInterval();

// all the below is optional, makes some fancy effects
// gradient fill of the series
series.fillOpacity = 1;
var gradient = new am4core.LinearGradient();
gradient.addColor(chart2.colors.getIndex(0), 0.2);
gradient.addColor(chart2.colors.getIndex(0), 0);
series.fill = gradient;

// this makes date axis labels to fade out
dateAxis2.renderer.labels.template.adapter.add("fillOpacity", function (fillOpacity, target) {
    var dataItem = target.dataItem;
    return dataItem.position;
})

// need to set this, otherwise fillOpacity is not changed and not set
dateAxis2.events.on("validated", function () {
    am4core.iter.each(dateAxis2.renderer.labels.iterator(), function (label) {
        label.fillOpacity = label.fillOpacity;
    })
})

// this makes date axis labels which are at equal minutes to be rotated
dateAxis2.renderer.labels.template.adapter.add("rotation", function (rotation, target) {
    var dataItem = target.dataItem;
    if (dataItem.date.getTime() == am4core.time.round(new Date(dataItem.date.getTime()), "minute").getTime()) {
        
      target.verticalCenter = "middle";  
      target.horizontalCenter = "left";
        return -90;
    }
    else {
        target.verticalCenter = "bottom";  
        target.horizontalCenter = "middle";
        return 0;
    }
})

// bullet at the front of the line
var bullet = series.createChild(am4charts.CircleBullet);
bullet.circle.radius = 5;
bullet.fillOpacity = 1;
bullet.fill = chart2.colors.getIndex(0);
bullet.isMeasured = false;

series.events.on("validated", function() {
    bullet.moveTo(series.dataItems.last.point);
    bullet.validatePosition();
});
});
}