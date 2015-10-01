
const char MAIN_page[] PROGMEM = R"=====(
<html>
<head>
 <meta name="viewport" content="initial-scale=1">
 <style>
 body {font-family: helvetica,arial,sans-serif;}
 table {border-collapse: collapse; border: 1px solid black;}
 td {padding: 0.25em;}
 .heading {font-weight: bold; background: #c0c0c0; padding: 0.5em;}
 </style>
</head>
<h1>NTP Clock</h1>
<form method="post" action="/form">
<table>
<tr><td colspan=2 class=heading>Status</td></tr>
<tr><td>Current time:</td><td><b>@@HOUR@@:@@MIN@@</b></td></tr>
<tr><td>Sync Status:</td><td>@@SYNCSTATUS@@</td></tr>

<tr><td colspan=2 class=heading>WiFi Setup</td></tr>
<tr><td>SSID:</td><td><input type=text name="ssid" value="@@SSID@@"></td></tr>
<tr><td>PSK:</td><td><input type=password name="psk" value="@@PSK@@"></td></tr>
<tr><td colspan=2>Update Wifi config:<input type=checkbox name=update_wifi value=1></td></tr>

<tr><td colspan=2 class=heading>Time Setup</td></tr>
<tr><td>NTP Server:</td><td><input type=text name="ntpsrv" value="@@NTPSRV@@"></td></tr>
<tr><td>Sync Interval:</td><td><input type=text name="ntpint" value="@@NTPINT@@">s</td></tr>
<tr><td>Timezone:</td><td><input type=text name="timezone" value="@@TZ@@">h</td></tr>
</table>
<p/>
<input type="submit" value="Update">
</form>

</html>
)=====";

