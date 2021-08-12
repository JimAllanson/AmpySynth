
#ifdef LCD_240_240
    #define ST7789_DRIVER 1
    #define TFT_WIDTH  240
    #define TFT_HEIGHT 240

    #define TFT_MISO 12
    #define TFT_MOSI 13
    #define TFT_SCLK 14
    #define TFT_RST 27
    #define TFT_DC 32
    #define TFT_CS 33

    #define LOAD_GLCD 1
    #define LOAD_FONT2 1

    #define SPI_FREQUENCY 80000000

    #define USER_SETUP_LOADED 1
#endif