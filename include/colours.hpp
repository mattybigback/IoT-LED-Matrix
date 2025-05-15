#ifndef COLOUR_NAMES_H
#define COLOUR_NAMES_H

// Allow override of colour order
#ifndef COLOUR_ORDER
#define COLOUR_ORDER RGB
#endif

// Macro helpers to extract channels in the desired order
#define _GET_R(r, g, b) r
#define _GET_G(r, g, b) g
#define _GET_B(r, g, b) b

#define _ORDER_RGB(r, g, b) _GET_R(r, g, b), _GET_G(r, g, b), _GET_B(r, g, b)
#define _ORDER_RBG(r, g, b) _GET_R(r, g, b), _GET_B(r, g, b), _GET_G(r, g, b)
#define _ORDER_GRB(r, g, b) _GET_G(r, g, b), _GET_R(r, g, b), _GET_B(r, g, b)
#define _ORDER_GBR(r, g, b) _GET_G(r, g, b), _GET_B(r, g, b), _GET_R(r, g, b)
#define _ORDER_BRG(r, g, b) _GET_B(r, g, b), _GET_R(r, g, b), _GET_G(r, g, b)
#define _ORDER_BGR(r, g, b) _GET_B(r, g, b), _GET_G(r, g, b), _GET_R(r, g, b)

// Macro to select the correct order
#define _COLOUR_ORDER_EXPAND(order, r, g, b) _ORDER_##order(r, g, b)
#define _COLOUR_ORDER_HELPER(order, r, g, b) _COLOUR_ORDER_EXPAND(order, r, g, b)
#define COLOUR(r, g, b) _COLOUR_ORDER_HELPER(COLOUR_ORDER, r, g, b)

// Colours
#define RED      COLOUR(50,  0,  0)
#define ORANGE   COLOUR(45,  5,  0)
#define YELLOW   COLOUR(35, 15,  0)
#define GREEN    COLOUR( 0, 50,  0)
#define CYAN     COLOUR( 0, 35, 15)
#define BLUE     COLOUR( 0,  0, 50)
#define MAGENTA  COLOUR(45,  0,  5)
#define VIOLET   COLOUR(10,  0, 40)
#define WHITE    COLOUR(16, 16, 16)

#endif