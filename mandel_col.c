/* This color map generator appreciatively stolen, with slight alterations,
 * from Dennis' xmand implementation at https://git.sr.ht/~blastwave/bw
 */

/*
 * mandel_col.c A simple colour map for the mandlebrot
 * Copyright (C) Dennis Clarke 2019
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * https://www.gnu.org/licenses/gpl-3.0.txt
 */

/*********************************************************************
 * The Open Group Base Specifications Issue 6
 * IEEE Std 1003.1, 2004 Edition
 *
 *    An XSI-conforming application should ensure that the feature
 *    test macro _XOPEN_SOURCE is defined with the value 600 before
 *    inclusion of any header. This is needed to enable the
 *    functionality described in The _POSIX_C_SOURCE Feature Test
 *    Macro and in addition to enable the XSI extension.
 *
 *********************************************************************/
#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

unsigned long linear_inter( uint8_t  in_val,
                            uint32_t low_col, uint32_t high_col,
                            uint8_t  low_val, uint8_t upper_val);

unsigned long mandel_col ( uint8_t height )
{
    unsigned long cpixel = 0;
    /* the idea on the table is to compute a reasonable
     * 24 bit value for RGB data based on a range of
     * possible mandlebrot evaluations :
     *
     *  range val
     *   0 - 31  : dark blue     -> light blue
     *             0x0b104b         0x6973ee
     *
     *  32 - 63  : light blue    -> light red
     *             0x6973ee         0xf73f3f
     *
     *  64 - 127 : light red     -> dark cyan
     *             0xf73f3f         0xb7307b
     *
     * 128 - 159 : dark cyan     -> bright yellow
     *             0xb7307b         0xecff3a
     *
     * 160 - 191 : bright yellow -> dark red
     *             0xecff3a         0x721a1a
     *
     * 192 - 223 : dark red      -> green
     *             0x721a1a         0x00ff00
     *
     * 224 - 239 : green         -> magenta
     *             0x00ff00         0xff00ff
     *
     * 11 May 2021 lets try to smooth this out and avoid
     * a transition to pure white 0xffffff where really
     * we need to wrap around the 8-bit colour index to
     * the start value dark blue 0x0b104b
     * 240 - 255 : magenta       -> white
     *             0xff00ff         0xffffff
     *                              0x0b104b  <-- try this
     */

    if ( height < 32 ) {
        cpixel = linear_inter( height, (uint32_t)0x0b104b,
                                       (uint32_t)0x6973ee,
                                       (uint8_t)0, (uint8_t)31);
    } else if ( ( height > 31 ) && ( height < 64 ) ) {
        cpixel = linear_inter( height, (uint32_t)0x6973ee,
                                       (uint32_t)0xf73f3f,
                                       (uint8_t)32, (uint8_t)63);
    } else if ( ( height > 63 ) && ( height < 128 ) ) {
        cpixel = linear_inter( height, (uint32_t)0xf73f3f,
                                       (uint32_t)0xb7307b,
                                       (uint8_t)64, (uint8_t)127);
    } else if ( ( height > 127 ) && ( height < 160 ) ) {
        cpixel = linear_inter( height, (uint32_t)0xb7307b,
                                       (uint32_t)0xecff3a,
                                       (uint8_t)128, (uint8_t)159);
    } else if ( ( height > 159 ) && ( height < 192 ) ) {
        cpixel = linear_inter( height, (uint32_t)0xecff3a,
                                       (uint32_t)0x721a1a,
                                       (uint8_t)160, (uint8_t)191);
    } else if ( ( height > 191 ) && ( height < 224 ) ) {
        cpixel = linear_inter( height, (uint32_t)0x721a1a,
                                       (uint32_t)0x00ff00,
                                       (uint8_t)192, (uint8_t)223);
    } else if ( ( height > 223 ) && ( height < 240 ) ) {
        cpixel = linear_inter( height, (uint32_t)0x00ff00,
                                       (uint32_t)0xff00ff,
                                       (uint8_t)224, (uint8_t)239);
    } else if ( height > 239 ) {
        /* see comment above from 11 May 2021 */
        cpixel = linear_inter( height, (uint32_t)0xff00ff,
                                       (uint32_t)0x0b104b,
                                       (uint8_t)240, (uint8_t)255);
    }

    return ( cpixel );

}

unsigned long linear_inter( uint8_t  in_val,
                            uint32_t low_col, uint32_t high_col,
                            uint8_t  low_val, uint8_t upper_val)
{
    /* in_val is some number that should fall between
     *        the low_val and upper_val. If not then
     *        just assume low_val or upper_val as
     *        needed.
     *
     * low_col is the actual RGB value at the low end
     *         of the scale
     *
     * high_col is the RGB colour at the high end of the
     *           scale
     *
     * low_val and upper_val are the possible range values
     *          for the in_val
     *
     * How to do a linear interpolation between two 32-bit colour
     * values?  We need a smooth function :
     *
     *    uint32_t cpixel = ( uint8_t   red_val << 16 )
     *                       + ( uint8_t green_val << 8 )
     *                       +   uint8_t  blue_val
     **/

    uint16_t red, green, blue;
    int red_direction = 1;
    int green_direction = 1;
    int blue_direction = 1;
    int lower_red, upper_red;
    int lower_green, upper_green;
    int lower_blue, upper_blue;
    unsigned long cpixel;

    if (    ( high_col & (uint32_t)0xff0000 )
         <= (  low_col & (uint32_t)0xff0000 ) ) {

        lower_red = (uint16_t)( ( high_col & (uint32_t)0xff0000 ) >> 16 );
        upper_red = (uint16_t)( (  low_col & (uint32_t)0xff0000 ) >> 16 );
        /* the red color decreases as the input index increases */
        red_direction = 0;

    } else {

        upper_red = (uint16_t)( ( high_col & (uint32_t)0xff0000 ) >> 16 );
        lower_red = (uint16_t)( (  low_col & (uint32_t)0xff0000 ) >> 16 );

    }

    if (    ( high_col & (uint32_t)0x00ff00 )
         <= (  low_col & (uint32_t)0x00ff00 ) ) {

        lower_green = (uint16_t)( ( high_col & (uint32_t)0x00ff00 ) >> 8 );
        upper_green = (uint16_t)( (  low_col & (uint32_t)0x00ff00 ) >> 8 );
        /* the green color decreases as the input index increases */
        green_direction = 0;

    } else {

        upper_green = (uint16_t)( ( high_col & (uint32_t)0x00ff00 ) >> 8 );
        lower_green = (uint16_t)( (  low_col & (uint32_t)0x00ff00 ) >> 8 );

    }

    if (    ( high_col & (uint32_t)0x0000ff )
         <= (  low_col & (uint32_t)0x0000ff ) ) {

        lower_blue = (uint16_t)( high_col & (uint32_t)0x0000ff );
        upper_blue = (uint16_t)(  low_col & (uint32_t)0x0000ff );
        /* the blue color decreases as the input index increases */
        blue_direction = 0;

    } else {

        upper_blue = (uint16_t)( high_col & (uint32_t)0x0000ff );
        lower_blue = (uint16_t)(  low_col & (uint32_t)0x0000ff );

    }

    if ( red_direction ) {
        red = (uint16_t)( lower_red
                 + ( upper_red - lower_red )
                   * ( in_val - low_val ) / ( upper_val - low_val ) );
    } else {
        red = (uint16_t)( upper_red
                - ( upper_red - lower_red )
                   * ( in_val - low_val ) / ( upper_val - low_val ) );
    }

    if ( green_direction ) {
        green = (uint16_t)( lower_green
                 + ( upper_green - lower_green )
                    * ( in_val - low_val ) / ( upper_val - low_val ) );
    } else {
        green = (uint16_t)( upper_green
                - ( upper_green - lower_green )
                   * ( in_val - low_val ) / ( upper_val - low_val ) );
    }

    if ( blue_direction ) {
        blue = (uint16_t)( lower_blue
                 + ( upper_blue - lower_blue )
                    * ( in_val - low_val ) / ( upper_val - low_val ) );
    } else {
        blue = (uint16_t)( upper_blue
                - ( upper_blue - lower_blue )
                   * ( in_val - low_val ) / ( upper_val - low_val ) );
    }

    cpixel = (unsigned long)(   (uint32_t)red << 16 ) 
                            | ( (uint32_t)green << 8 ) 
                              | (uint32_t)blue;

    return ( cpixel );

}