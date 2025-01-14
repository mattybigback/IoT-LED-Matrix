const char mainPage[] PROGMEM = R"(<!DOCTYPE html>
<html>
<head>
<title>
LED Matrix Configuration Page
</title>
<body>
<h2>Matt's Matrix Display</h2>
<h3>Control Panel</h3>
<form action="/update">
<p>Message to display:
<input type="text" name="messageToScroll" maxlength="%d" value="%s">
</p>
<p>LED Brightness:
<input type="number" name="intensity" min="0" max="15" value="%d">
</p>
<p>Scroll Speed (lower is faster):
<input type="number" name="speed" min="10" max="200" value="%d">
</p>
<input type="submit" value="Submit">
</form> 
</body>
</html>
)";
