/*
  Geiger counter web interface by Marco Santi (github.com/marcoSanti)
*/


var endpoint = window.location.protocol + '//' + window.location.hostname + "/data";
var CpmChart=null;
var itemCounts=0;
var maxItemInGraph = 30;

const CPMGraphConfig = {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
          label: 'Counts',
          backgroundColor: 'rgb(255, 99, 132)',
          borderColor: 'rgb(255, 99, 132)',
          data: [],
        }]
      },
    options: {
        responsive: true,
        plugins: {
          legend: {
            position: 'top',
          },
          title: {
            display: true,
            text: 'Counts graph'
          }
        }
      },
  };


function addData(chart, label, data) {
    chart.data.labels.push(label);
    chart.data.datasets.forEach((dataset) => {
        dataset.data.push(data);
    });
    chart.update();
    if(itemCounts<maxItemInGraph) itemCounts++;
    else{
      while(itemCounts >= maxItemInGraph){
          chart.data.labels.shift();
          chart.data.datasets.forEach((dataset) => {
            dataset.data.shift();
          });
          itemCounts--;
      }
      
      chart.update();
    }
}




function fetchData(){
    $.get(endpoint).done(function(data){
        console.log(data);
        var today = new Date();
        var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
        addData(CpmChart, time, data["CPM"]);
        $("#cpm").html(data["CPM"]);
        $("#usvh").html(data["uSvH"]);
    });

}

function updateNumberItemsLabel(){
  $("#lblMaxItemInGraph").html(maxItemInGraph);
}


$(document).ready(function(){
    setInterval(fetchData, 1000);
    CpmChart = new Chart(document.getElementById('cpmCountsGraph'),CPMGraphConfig);
    updateNumberItemsLabel();

    $("#increaseGraphItems").click(function(){
      maxItemInGraph++;
      updateNumberItemsLabel();
    });
    
    $("#decreaseGraphItems").click(function(){
      maxItemInGraph--;
      updateNumberItemsLabel();
    });
})
