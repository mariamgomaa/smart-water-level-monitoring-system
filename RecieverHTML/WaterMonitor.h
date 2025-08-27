/*
  WaterMonitor.h
  HTML page for Water Level Monitoring System
  This file contains the web page code that will be served by the ESP32
*/

// HTML page stored in PROGMEM to save RAM
const char PAGE_MAIN[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en" class="js-focus-visible">

<title>Water Level Monitor</title>

  <style>
    table {
      position: relative;
      width:100%;
      border-spacing: 0px;
    }
    tr {
      border: 1px solid white;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 20px;
    }
    th {
      height: 20px;
      padding: 3px 15px;
      background-color: #343a40;
      color: #FFFFFF !important;
      }
    td {
      height: 20px;
       padding: 3px 15px;
    }
    .tabledata {
      font-size: 24px;
      position: relative;
      padding-left: 5px;
      padding-top: 5px;
      height: 25px;
      border-radius: 5px;
      color: #FFFFFF;
      line-height: 20px;
      transition: all 200ms ease-in-out;
      background-color: #00AA00;
    }
    .danger {
      background-color: #AA0000 !important;
      animation: blink 1s infinite;
    }
    .warning {
      background-color: #FF6600 !important;
    }
    .normal {
      background-color: #00AA00 !important;
    }
    @keyframes blink {
      0% { opacity: 1; }
      50% { opacity: 0.5; }
      100% { opacity: 1; }
    }
    .bodytext {
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 24px;
      text-align: left;
      font-weight: light;
      border-radius: 5px;
      display:inline;
    }
    .navbar {
      width: 100%;
      height: 50px;
      margin: 0;
      padding: 10px 0px;
      background-color: #FFF;
      color: #000000;
      border-bottom: 5px solid #293578;
    }
    .fixed-top {
      position: fixed;
      top: 0;
      right: 0;
      left: 0;
      z-index: 1030;
    }
    .navtitle {
      float: left;
      height: 50px;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 50px;
      font-weight: bold;
      line-height: 50px;
      padding-left: 20px;
    }
   .navheading {
     position: fixed;
     left: 60%;
     height: 50px;
     font-family: "Verdana", "Arial", sans-serif;
     font-size: 20px;
     font-weight: bold;
     line-height: 20px;
     padding-right: 20px;
   }
   .navdata {
      justify-content: flex-end;
      position: fixed;
      left: 70%;
      height: 50px;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 20px;
      font-weight: bold;
      line-height: 20px;
      padding-right: 20px;
   }
    .category {
      font-family: "Verdana", "Arial", sans-serif;
      font-weight: bold;
      font-size: 32px;
      line-height: 50px;
      padding: 20px 10px 0px 10px;
      color: #000000;
    }
    .heading {
      font-family: "Verdana", "Arial", sans-serif;
      font-weight: normal;
      font-size: 28px;
      text-align: left;
    }
    .btn {
      background-color: #444444;
      border: none;
      color: white;
      padding: 10px 20px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      margin: 4px 2px;
      cursor: pointer;
    }
    .foot {
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 20px;
      position: relative;
      height: 30px;
      text-align: center;   
      color: #AAAAAA;
      line-height: 20px;
    }
    .container {
      max-width: 1800px;
      margin: 0 auto;
    }
    table tr:first-child th:first-child {
      border-top-left-radius: 5px;
    }
    table tr:first-child th:last-child {
      border-top-right-radius: 5px;
    }
    table tr:last-child td:first-child {
      border-bottom-left-radius: 5px;
    }
    table tr:last-child td:last-child {
      border-bottom-right-radius: 5px;
    }
    .status-box {
      padding: 20px;
      margin: 20px 0;
      border-radius: 10px;
      font-size: 24px;
      font-weight: bold;
      text-align: center;
    }
    .status-normal {
      background-color: #d4edda;
      color: #155724;
      border: 2px solid #c3e6cb;
    }
    .status-warning {
      background-color: #fff3cd;
      color: #856404;
      border: 2px solid #ffeaa7;
    }
    .status-danger {
      background-color: #f8d7da;
      color: #721c24;
      border: 2px solid #f5c6cb;
      animation: blink 1s infinite;
    }
  </style>

  <body style="background-color: #efefef" onload="process()">
  
    <header>
      <div class="navbar fixed-top">
          <div class="container">
            <div class="navtitle">Water Level Monitor</div>
            <div class="navdata" id = "date">mm/dd/yyyy</div>
            <div class="navheading">DATE</div><br>
            <div class="navdata" id = "time">00:00:00</div>
            <div class="navheading">TIME</div>
          </div>
      </div>
    </header>
  
    <main class="container" style="margin-top:70px">
      
      <!-- System Status Box -->
      <div id="systemStatus" class="status-box status-normal">
        SYSTEM NORMAL
      </div>

      <div class="category">Water Level Readings</div>
      <div style="border-radius: 10px !important;">
      <table style="width:80%">
      <colgroup>
        <col span="1" style="background-color:rgb(230,230,230); width: 30%; color:#000000;">
        <col span="1" style="background-color:rgb(200,200,200); width: 25%; color:#000000;">
        <col span="1" style="background-color:rgb(180,180,180); width: 25%; color:#000000;">
        <col span="1" style="background-color:rgb(160,160,160); width: 20%; color:#000000;">
      </colgroup>
      <tr>
        <th colspan="1"><div class="heading">Source</div></th>
        <th colspan="1"><div class="heading">Water Level (cm)</div></th>
        <th colspan="1"><div class="heading">Status</div></th>
        <th colspan="1"><div class="heading">Last Update</div></th>
      </tr>
      <tr>
        <td><div class="bodytext">Measured (Local Sensor)</div></td>
        <td><div class="tabledata" id="measuredLevel"></div></td>
        <td><div class="tabledata" id="measuredStatus"></div></td>
        <td><div class="tabledata" id="measuredTime"></div></td>
      </tr>
      <tr>
        <td><div class="bodytext">Received (ESP-NOW)</div></td>
        <td><div class="tabledata" id="receivedLevel"></div></td>
        <td><div class="tabledata" id="receivedStatus"></div></td>
        <td><div class="tabledata" id="receivedTime"></div></td>
      </tr>
      </table>
    </div>
    <br>
    
    <div class="category">LED Status Indicators</div>
    <br>
    <div class="bodytext">Danger LED: </div>
    <div class="tabledata" id="dangerLed" style="display:inline-block; width:100px; text-align:center;">OFF</div>
    <br><br>
    <div class="bodytext">Normal LED: </div>
    <div class="tabledata" id="normalLed" style="display:inline-block; width:100px; text-align:center;">OFF</div>
    <br><br>
    
    <div class="category">System Controls</div>
    <br>
    <div class="bodytext">Test SMS Alert: </div>
    <button type="button" class="btn" id="smsBtn" onclick="SendTestSMS()">Send Test SMS</button>
    <br><br>
    <div class="bodytext">Reset System: </div>
    <button type="button" class="btn" id="resetBtn" onclick="ResetSystem()">Reset</button>
    <br><br>
  </main>

  <footer div class="foot" id="footer">ESP32 Water Level Monitoring System</div></footer>
  
  </body>

  <script type="text/javascript">
  
    // Global variable for XML HTTP object
    var xmlHttp = createXmlHttpObject();

    // Function to create XML object
    function createXmlHttpObject(){
      if(window.XMLHttpRequest){
        xmlHttp = new XMLHttpRequest();
      }
      else{
        xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
      }
      return xmlHttp;
    }

    // Function to send test SMS
    function SendTestSMS() {
      var xhttp = new XMLHttpRequest(); 
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          alert("Test SMS sent: " + this.responseText);
        }
      }
      xhttp.open("PUT", "SEND_SMS", true);
      xhttp.send();
    }

    // Function to reset system
    function ResetSystem() {
      var xhttp = new XMLHttpRequest(); 
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          alert("System reset: " + this.responseText);
        }
      }
      xhttp.open("PUT", "RESET_SYSTEM", true);
      xhttp.send();
    }

    // Function to get status class based on water level
    function getStatusClass(level) {
      if (level > 15) return "danger";
      else if (level > 10) return "warning";
      else return "normal";
    }

    // Function to get status text based on water level
    function getStatusText(level) {
      if (level > 15) return "DANGER";
      else if (level > 10) return "WARNING";
      else return "NORMAL";
    }

    // Function to handle XML response from ESP32
    function response(){
      var message;
      var xmlResponse;
      var xmldoc;
      var dt = new Date();
     
      // Get the XML stream
      xmlResponse = xmlHttp.responseXML;
  
      // Update date and time
      document.getElementById("time").innerHTML = dt.toLocaleTimeString();
      document.getElementById("date").innerHTML = dt.toLocaleDateString();
  
      // Process measured water level
      xmldoc = xmlResponse.getElementsByTagName("MEASURED_LEVEL");
      if (xmldoc.length > 0) {
        message = xmldoc[0].firstChild.nodeValue;
        var measuredLevel = parseFloat(message);
        
        document.getElementById("measuredLevel").innerHTML = message + " cm";
        document.getElementById("measuredLevel").className = "tabledata " + getStatusClass(measuredLevel);
        document.getElementById("measuredStatus").innerHTML = getStatusText(measuredLevel);
        document.getElementById("measuredStatus").className = "tabledata " + getStatusClass(measuredLevel);
        document.getElementById("measuredTime").innerHTML = dt.toLocaleTimeString();
      }
      
      // Process received water level
      xmldoc = xmlResponse.getElementsByTagName("RECEIVED_LEVEL");
      if (xmldoc.length > 0) {
        message = xmldoc[0].firstChild.nodeValue;
        var receivedLevel = parseFloat(message);
        
        document.getElementById("receivedLevel").innerHTML = message + " cm";
        document.getElementById("receivedLevel").className = "tabledata " + getStatusClass(receivedLevel);
        document.getElementById("receivedStatus").innerHTML = getStatusText(receivedLevel);
        document.getElementById("receivedStatus").className = "tabledata " + getStatusClass(receivedLevel);
      }
      
      // Process received time
      xmldoc = xmlResponse.getElementsByTagName("RECEIVED_TIME");
      if (xmldoc.length > 0) {
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("receivedTime").innerHTML = message;
      }

      // Process LED status
      xmldoc = xmlResponse.getElementsByTagName("DANGER_LED");
      if (xmldoc.length > 0) {
        message = xmldoc[0].firstChild.nodeValue;
        if (message == "1") {
          document.getElementById("dangerLed").innerHTML = "ON";
          document.getElementById("dangerLed").className = "tabledata danger";
        } else {
          document.getElementById("dangerLed").innerHTML = "OFF";
          document.getElementById("dangerLed").className = "tabledata";
        }
      }
      
      xmldoc = xmlResponse.getElementsByTagName("NORMAL_LED");
      if (xmldoc.length > 0) {
        message = xmldoc[0].firstChild.nodeValue;
        if (message == "1") {
          document.getElementById("normalLed").innerHTML = "ON";
          document.getElementById("normalLed").className = "tabledata normal";
        } else {
          document.getElementById("normalLed").innerHTML = "OFF";
          document.getElementById("normalLed").className = "tabledata";
        }
      }

      // Update system status box
      xmldoc = xmlResponse.getElementsByTagName("SYSTEM_STATUS");
      if (xmldoc.length > 0) {
        message = xmldoc[0].firstChild.nodeValue;
        var statusBox = document.getElementById("systemStatus");
        if (message == "DANGER") {
          statusBox.innerHTML = "⚠️ WATER LEVEL CRITICAL!";
          statusBox.className = "status-box status-danger";
        } else if (message == "WARNING") {
          statusBox.innerHTML = "⚠️ WATER LEVEL HIGH";
          statusBox.className = "status-box status-warning";
        } else {
          statusBox.innerHTML = "✅ SYSTEM NORMAL";
          statusBox.className = "status-box status-normal";
        }
      }
    }
  
    // Main processing function to request XML data from ESP32
    function process(){
      if(xmlHttp.readyState == 0 || xmlHttp.readyState == 4) {
        xmlHttp.open("PUT", "xml", true);
        xmlHttp.onreadystatechange = response;
        xmlHttp.send(null);
      }       
      // Update every 2 seconds
      setTimeout("process()", 2000);
    }
  
  </script>

</html>

)=====";