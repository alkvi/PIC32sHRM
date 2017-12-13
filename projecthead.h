/* projecthead.h
   Header file for project
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson
   Modified 2017 by akv

   Latest update 2017-11-19 by akv

   For copyright and licensing, see file COPYING */

/* Declare display-related functions from mipslabfunc.c */
void display_image(int x, const uint8_t *data);
void display_init(void);
void display_string(int line, char *s);
void display_update(void);
uint8_t spi_send_recv(uint8_t data);

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];
/* Declare bitmap array containing icon */
extern const uint8_t const icon[128];
/* Declare text buffer for display output */
extern char textbuffer[4][16];

char * itoaconv( int num );

