const char STATE_page[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
<head>
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
<div id="statusPort">
%%DATA%%
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
  xhttp.open("GET", "readPort", true);
  xhttp.send();
}
</script>

</body>
</html>
)=====";