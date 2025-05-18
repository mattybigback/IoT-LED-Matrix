
#include "maxMatrix.hpp"

void messageScroll() {
    // If the display is still animating OR the resetDisplay flag has been set
    if (matrix.displayAnimate() || resetDisplay) {
        matrix.displayClear();
        if (newMessageAvailable) {            // If a new message has been set
            debugln("New message available"); // Debug message
            resetDisplay = false;             // Clear the resetDisplay flag
            snprintf(curMessage, MSG_BUF_SIZE, "%s", newMessage); // Copy the newMessage buffer to curMessage
            newMessageAvailable = false;      // Clear the newMessageAvailabe flag

        }
        if (displaySettingsChanged) { // If the display settings have changed
            debugln("Display settings changed"); // Debug message
            resetDisplay = false;             // Clear the resetDisplay flag
            matrix.setSpeed(scrollSpeed);        // Set the scroll speed
            debug("Scroll speed: ");
            debugln(scrollSpeed);               // Debug message
            matrix.setIntensity(intensity);      // Set the intensity
            debug("Intensity: ");
            debugln(intensity);                 // Debug message
            setMatrixOrientation(displayFlipped); // Set the display orientation
            debug("Display flipped: ");
            debugln(displayFlipped);           // Debug message
            displaySettingsChanged = false;      // Clear the displaySettingsChanged flag
        }
        matrix.displayReset(); // If display has finished animating reset the animation
        #if defined(HAS_NEOPIXEL)
            neopixelWrite(NEOPIXEL_PIN, GREEN);
        #endif
    }
}

bool setMatrixOrientation(bool flipDisplay) {
    matrix.displayClear();
    //matrix.displayReset();
    matrix.setZoneEffect(0, flipDisplay, PA_FLIP_LR);
    matrix.setZoneEffect(0, flipDisplay, PA_FLIP_UD);

    // Set the matrix orientation
    if (flipDisplay) {
        debugln("Flipping display");
        scrollEffect = PA_SCROLL_RIGHT;
        scrollAlign = PA_RIGHT;

    } else {
        // Set the matrix to the default orientation
        debugln("Setting display to default orientation");
        scrollEffect = PA_SCROLL_LEFT;
        scrollAlign = PA_LEFT;

    }

    matrix.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
    return flipDisplay;
}