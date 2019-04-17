const char STATE_page[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html lang="fr">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <!--jquery/3.1.1')-->
    <script src="js/jquery.min.js"></script>
    <!--bootstrap/3.4.1')-->
    <script src="js/bootstrap.min.js"></script>
    <link rel="stylesheet" href="css/bootstrap.min.css">
    <title>Etat GPIO</title>
<style>

      table {
        width: 100%;
        margin: 1em 0;
        border-collapse: separate;
        border-spacing: .2em;
        word-wrap: break-word;
        text-align: left;
        background-color: #fefefe;
        -webkit-box-shadow: .5rem .5rem 2rem rgba(0,0,0,0.25);
        box-shadow: .5rem .5rem 2rem rgba(0,0,0,0.25);
      } 
      td, th { 
        border: 1px solid black;
        text-align: center;
        vertical-align: middle;
      }
      .ok {
        color: black;
        background-color: #7eff7e;
      }
      .error {
        color: white;
        background-color: #ff001c;
      }
      .colorblue {
        color: black;
        background-color: #00afff;
      }
      .colororange {
        color: black;
        background-color: orange;
      }
    
</style>
</head>
<body>
<div class="container">
  <ol class="breadcrumb">
    <li><a href="/sys"><span class="glyphicon glyphicon-home"></span></a></li>
    <li class="active">Etat GPIO</li>
  </ol>
  <div class="text-center">
    <h1><span class="glyphicon glyphicon-dashboard"></span> Etat GPIO</h1>
    <p>v 3.0.0</p> 
  </div>
  <div id="statusPort">
  %%DATA%%
  </div>
</div>
<script>
setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
}, 2000); //2000mSeconds update rate

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("statusPort").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readPort?mode=1", true);
  xhttp.send();
}
</script>

</body>
</html>
)=====";