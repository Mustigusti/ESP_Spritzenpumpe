from time import sleep
from threeinch5.main_3inch5 import LCD_3inch5

class MyScreen(LCD_3inch5):
    
    def __init__(self):
        super().__init__() #initialises like main_3inch5
        self.FHAACHEN = 0x7605
        
        self.bitmap_smiley = [
            0b00111100,
            0b01000010,
            0b10100101,
            0b10000001,
            0b10100101,
            0b10011001,
            0b01000010,
            0b00111100,
        ]
        self.bitmap_mu = [
            0b00000000,  #         
            0b01000100,  # *     * 
            0b01000100,  # *     * 
            0b01000100,  # *     * 
            0b01000100,  # *     * 
            0b01111100,  # *     * 
            0b11000100,  # ***  *  
            0b00000110,  # *       
        ]

    def draw_bitmap(self, bitmap, x_start, y_start, pixel_width, color):
        """
        Draws a picture based on an 8-byte bitmap on the screen.

        Parameters:
        - bitmap: List of 8 bytes, each representing one row of the picture.
        - x_start: X-coordinate of the top-left corner.
        - y_start: Y-coordinate of the top-left corner.
        - pixel_width: Width (and height) of each pixel block.
        - color: Color to fill the pixels.
        """
        for row_index, byte in enumerate(bitmap):
            for bit_index in range(8):
                # Check if the bit at position `bit_index` in `byte` is set (1)
                if (byte >> (7 - bit_index)) & 1:
                    # Calculate the position of the block to draw
                    x_pos = x_start + bit_index * pixel_width
                    y_pos = y_start + row_index * pixel_width

                    # Draw the filled block
                    self.fill_rect(x_pos, y_pos, pixel_width, pixel_width, color)

if __name__ == '__main__':
    lcd = MyScreen()
    lcd.bl_ctrl(100)
    lcd.fill(lcd.WHITE)
    lcd.show_up()
    lcd.fill(lcd.WHITE)
    lcd.show_down()
    
    lcd.fill(lcd.WHITE)
    lcd.text("Test: Mustafa Tugan, Regelungstechnik", 80,50)
    lcd.show_up()
    
    lcd.fill(lcd.WHITE)
    lcd.draw_bitmap(lcd.bitmap_mu, x_start=41+50, y_start=0, pixel_width=17, color=lcd.FHAACHEN)
    for i in range(0, 130, 18):
        for j in range(0, 130, 18):
            lcd.draw_bitmap(lcd.bitmap_mu, x_start=250+i, y_start=j, pixel_width=1, color=lcd.FHAACHEN)
        lcd.draw_bitmap(lcd.bitmap_mu, x_start=250+j, y_start=i, pixel_width=1, color=lcd.FHAACHEN)
    
    lcd.show_down()

#    while True:
#        get = lcd.touch_get()
#        print(get)
#        sleep(0.1)


