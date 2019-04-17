const char I2C_page[] PROGMEM = R"=====( 
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
    <title>Scan I2C</title>
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
  </HEAD>
  <BODY>
    <div class="container">
      <ol class="breadcrumb">
        <li><a href="/sys"><span class="glyphicon glyphicon-home"></span></a></li>
        <li class="active">Scan I2C</li>
      </ol>
      <div class="text-center">
        <h1><span class="glyphicon glyphicon-search"></span> Scan I2C</h1>
        <p>v 3.0.0</p> 
      </div>
      <div>
        <img alt="" src="data:image/gif;base64,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" />
      </div>
      <table>
      %%DATA%%
      </table>
    </div>
  </BODY>
</HTML>
)=====";