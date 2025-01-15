#include "webPortal.hpp"

// Server response to a request for root page
void handleRoot() {
  // Turn the LED or NeoPixel orange to indicate action
  neopixelWrite(RGB_BUILTIN, 255, 128, 0); // Orange
  debugln("Web portal index loaded");
  // Open file in read mode
  File webpage = LittleFS.open("/confpage/index.html", "r");
  if (!webpage) {
    debugln("Failed to open /confpage/index.html");
    server.send(404, "text/plain", "File not found");
    return;
  } else {
    debugln("File found succesfully");
  }
  // Read file contents into a String
  String pageContent;
  while (webpage.available()) {
    pageContent += (char)webpage.read();
  }
  webpage.close();
  debugln("File loaded succesfully");


  // We need a buffer large enough to hold the entire page with replacements.
  // Make sure it's large enough for your HTML plus variable expansions.
  char mainPageBuffer[1024];

  // Insert your variables in the exact order the placeholders appear
  // e.g. `sprintf(buffer, pageContent.c_str(), messageToScroll.c_str(), intensity, speed);`
  sprintf(
    mainPageBuffer, 
    pageContent.c_str(),
    BUF_SIZE,
    curMessage,
    intensity,
    scrollSpeed
  );

  // Serve the modified page
  server.send(200, "text/html", mainPageBuffer);
}


// Server response to incoming data from form
void handleForm() {
    neopixelWrite(RGB_BRIGHTNESS, 10, 20, 0); //Orange
    String incomingMessage = server.arg("messageToScroll"); // Must use strings as that is what the library returns
    String incomingIntensity = server.arg("intensity");     
    String incomingscrollSpeed = server.arg("speed");       

    incomingMessage.toCharArray(newMessage, BUF_SIZE); // Convert incoming message to a char array;
    intensity = incomingIntensity.toInt();             // Convert incoming intensity value to int
    scrollSpeed = incomingscrollSpeed.toInt();         // Comvert incoming scroll value to int

    // Write message, speed and intensity files to LittleFS
    messageFile = LittleFS.open(messagePath, "w");
    messageFile.print(newMessage);
    messageFile.close();
    scrollSpeedConfFile = LittleFS.open(speedConfPath, "w");
    scrollSpeedConfFile.print(scrollSpeed);
    scrollSpeedConfFile.close();
    intensityConfFile = LittleFS.open(intensityConfPath, "w");
    intensityConfFile.print(intensity);
    intensityConfFile.close();

    // Set the newMessageAvailable flag, clear the display, reset the display and set the resetDisplay flag
    newMessageAvailable = true;
    matrix.displayClear();
    matrix.displayReset();
    resetDisplay = true;

    // Debug output
    debugln(newMessage);
    debugln(intensity);
    debugln(scrollSpeed);
    debugln();

    // Send mainPage array as HTML
    server.send(200, "text/html", updatePage);
}