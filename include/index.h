const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang=en>
<head>
    <meta charset=utf-8>
    <title>ENV monitor</title>
    <meta charset=utf-8>
    <meta name="author" content="Tauno Erik">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    * {box-sizing: border-box;}
    body {font-family: Arial, Helvetica, sans-serif;}
    table {border-collapse: collapse;}
    td, th{
      border: 1px solid #dddddd;
      text-align: left;
      padding: 8px;
    }
    tr:nth-child(even) {background-color: #dddddd;}
  </style>
</head>
<body>
  <header>
    <h1>Tauno ENV monitor</h1>
  </header>
  <section>
    <table>
        <caption>The sensors data</caption>
        <tr>
            <td>PM2.5</td>
            <td><span id="sds011_pm25">0</span></td>
            <td>μg/m3</td>
        </tr>
        <tr>
            <td>PM10</td>
            <td><span id="sds011_pm10">0</span></td>
            <td>μg/m3</td>
        </tr>
        <tr>
            <td>eCO2</td>
            <td><span id="ccs811_eco2">0</span></td>
            <td>ppm</td>
        </tr>
        <tr>
            <td>eTVOC</td>
            <td><span id="ccs811_etvoc">0</span></td>
            <td>ppb</td>
        </tr>
        <tr>
            <td>BMP Temp</td>
            <td><span id="bmp280_temp">0</span></td>
            <td>*C</td>
        </tr>
        <tr>
            <td>Pressure</td>
            <td><span id="bmp280_pressure">0</span></td>
            <td>hPa</td>
        </tr>
        <tr>
            <td>AM2 temp</td>
            <td><span id="am2320_temp">0</span></td>
            <td>*C</td>
        </tr>
        <tr>
            <td>Hum</td>
            <td><span id="am2320_hum">0</span></td>
            <td></td>
        </tr>
        <tr>
            <td>Proximity</td>
            <td><span id="proximity">0</span></td>
            <td></td>
        </tr>
        <tr>
            <td>RGB</td>
            <td><span id="r">0</span>,<span id="g">0</span>,<span id="b">0</span></td>
            <td></td>
        </tr>
        <tr>
            <td>Gesture</td>
            <td><span id="gesture">0</span></td>
            <td></td>
        </tr>
    </table>
  </section>
  <footer>
    <p>Tauno Erik, 2020, <a href="https://github.com/taunoe/tauno-env-monitor">Code on Github</a></p>
  </footer>

<script>
setInterval(function() {
  get_sds011_pm25();
  get_sds011_pm10();
  get_ccs811_eco2();
  get_ccs811_etvoc();
  get_bmp280_temp();
  get_bmp280_pressure();
  get_am2320_temp();
  get_am2320_hum();
  get_proximity();
  get_r();
  get_g();
  get_b();
}, 2000); // 2 Second update interval

function get_sds011_pm25() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("sds011_pm25").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_sds011_pm25", true);
  xhttp.send();
}

function get_sds011_pm10() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("sds011_pm10").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_sds011_pm10", true);
  xhttp.send();
}

function get_ccs811_eco2() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ccs811_eco2").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_ccs811_eco2", true);
  xhttp.send();
}

function get_ccs811_etvoc() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ccs811_etvoc").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_ccs811_etvoc", true);
  xhttp.send();
}

function get_bmp280_temp() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("bmp280_temp").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_bmp280_temp", true);
  xhttp.send();
}

function get_bmp280_pressure() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("bmp280_pressure").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_bmp280_pressure", true);
  xhttp.send();
}

function get_am2320_temp() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("am2320_temp").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_am2320_temp", true);
  xhttp.send();
}

function get_am2320_hum() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("am2320_hum").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_am2320_hum", true);
  xhttp.send();
}

function get_proximity() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("proximity").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_proximity", true);
  xhttp.send();
}

function get_r() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("r").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_r", true);
  xhttp.send();
}

function get_g() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("g").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_g", true);
  xhttp.send();
}

function get_b() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("b").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "read_b", true);
  xhttp.send();
}

</script>
</body>
</html>
)=====";