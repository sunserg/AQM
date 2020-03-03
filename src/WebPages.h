//#define PROGMEM ICACHE_RODATA_ATTR

const char CONFIG_PAGE[] PROGMEM = R"=====(
<HTML>
 <HEAD>
   <TITLE>Air quality monitor (BME680)</TITLE>
 </HEAD>
 <BODY>
  <form action="/settings" method="get">
   %s<br><br>
   %s<br><br>
   %s<br><br>
   <br><br>
   <fieldset style='display: inline-block; width: 300px'>
    <legend>LED settings</legend>
    Blink interval:<br>
    <input type="text" name="blink_interval" value="%d"><br>
    <small><em>in seconds, 0 to disable blinking, max value - 3600</em></small><br>
   </fieldset>
   <br><br>
   <input type="submit" value="Save" style='width: 150px;'>
   &nbsp;&nbsp;&nbsp;
   <a href="/reboot">
    <button type="button" style='width: 150px;'>Restart</button>
   </a>
  </form>
 </BODY>
</HTML>
)=====";

const char GET_JSON[] PROGMEM = R"=====({
 "temp":%.2f,
 "humidity":%.2f,
 "pressure":%.2f,
 "gas_resistance":%.0f,
 "iaq_accuracy":%d,
 "iaq":%.2f,
 "static_iaq":%.2f,
 "free_heap": %d
})=====";