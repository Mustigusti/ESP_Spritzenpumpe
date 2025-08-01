from time import sleep
from threeinch5.main_3inch5 import LCD_3inch5
from utime import ticks_us
import os


class MyScreen(LCD_3inch5):
    
    def __init__(self):
        super().__init__() #initialises like main_3inch5
        self.FHAACHEN = 0x7605
        self.OCEANBLUE= 0x4A19
        self.current_text = ("Geben Sie die Flussrate in mikroliter/min an!")
        self.is_touched = False
        self. last_time_is_touched = False
        self.first_touch = True

        # Initialize flow-related variables
        self.current_flow_rate = 0
        self.max_flow_rate = 0
        self.last_flow_rate = 0
        self.average_flow_rate = 0
        self.flow_counter = 0
        self.flow_sum = 0
                        
        self.bitmap_smiley = [
            0b00111100,  #  ****  
            0b01000010,  # *    * 
            0b10100101,  #* *  * *
            0b10000001,  #*      *
            0b10100101,  #* *  * *
            0b10011001,  #*  **  *
            0b01000010,  #  ****  
            0b00111100,  #
        ]

        self.digits = {
            "0": [
                0b00111100,  #   ****  
                0b01000010,  #  *    * 
                0b01000010,  #  *    * 
                0b01000010,  #  *    * 
                0b01000010,  #  *    * 
                0b01000010,  #  *    * 
                0b00111100,  #   ****  
                0b00000000,  #         
            ],
            "1": [
                0b00001000,  #     *   
                0b00011000,  #    **   
                0b00001000,  #     *   
                0b00001000,  #     *   
                0b00001000,  #     *   
                0b00001000,  #     *   
                0b00111110,  #   ***** 
                0b00000000,  #         
            ],
            "2": [
                0b00111100,  #   ****  
                0b01000010,  #  *    * 
                0b00000010,  #       * 
                0b00001100,  #     **  
                0b00010000,  #    *    
                0b00100000,  #   *     
                0b01111110,  #  ****** 
                0b00000000,  #         
            ],
            "3": [
                0b00111100,  #   ****  
                0b01000010,  #  *    * 
                0b00000010,  #       * 
                0b00111100,  #   ****  
                0b00000010,  #       * 
                0b01000010,  #  *    * 
                0b00111100,  #   ****  
                0b00000000,  #         
            ],
            "4": [
                0b00000100,  #      *  
                0b00001100,  #     **  
                0b00010100,  #    * *  
                0b00100100,  #   *  *  
                0b01111110,  #  ****** 
                0b00000100,  #      *  
                0b00000100,  #      *  
                0b00000000,  #         
            ],
            "5": [
                0b01111110,  #  ****** 
                0b01000000,  #  *      
                0b01111100,  #  *****  
                0b00000010,  #       * 
                0b00000010,  #       * 
                0b01000010,  #  *    * 
                0b00111100,  #   ****  
                0b00000000,  #         
            ],
            "6": [
                0b00111100,  #   ****  
                0b01000000,  #  *      
                0b01111100,  #  *****  
                0b01000010,  #  *    * 
                0b01000010,  #  *    * 
                0b01000010,  #  *    * 
                0b00111100,  #   ****  
                0b00000000,  #         
            ],
            "7": [
                0b01111110,  #  ****** 
                0b00000010,  #       * 
                0b00000100,  #      *  
                0b00001000,  #     *   
                0b00010000,  #    *    
                0b00100000,  #   *     
                0b00100000,  #   *     
                0b00000000,  #         
            ],
            "8": [
                0b00111100,  #   ****  
                0b01000010,  #  *    * 
                0b01000010,  #  *    * 
                0b00111100,  #   ****  
                0b01000010,  #  *    * 
                0b01000010,  #  *    * 
                0b00111100,  #   ****  
                0b00000000,  #         
            ],
            "9": [
                0b00111100,  #   ****  
                0b01000010,  #  *    * 
                0b01000010,  #  *    * 
                0b00111110,  #   ***** 
                0b00000010,  #       * 
                0b00000100,  #      *  
                0b00111000,  #   ***   
                0b00000000,  #         
            ]
        }
        self.symbols = {
            "comma": [
                0b00000000,  #         
                0b00000000,  #         
                0b00000000,  #         
                0b00000000,  #         
                0b00000000,  #         
                0b00000000,  #         
                0b00001000,  #     *   
                0b00011000,  #    **   
            ],
            "enter": [
                0b11111111,  #********
                0b10000001,  #*      *
                0b10000100,  #*    *  
                0b10111110,  #* ***** 
                0b10111110,  #* *****
                0b10000100,  #*    *
                0b10000001,  #*      *
                0b11111111,  #******** 
            ],
            "backspace": [
                0b00000000,  #        
                0b00011111,  #   *****
                0b00100001,  #  *    *
                0b01010101,  # * * * *
                0b11001001,  #**  *  *
                0b01010101,  # * * * *
                0b00100001,  #  *    *
                0b00011111,  #   *****
            ],
            "mu": [
                0b00000000,  #   
                0b01000100,  # *   *
                0b01000100,  # *   *
                0b01000100,  # *   *
                0b01000100,  # *   *
                0b01111100,  # *****
                0b11000100,  #*    *
                0b00000110,  #     **
            ],
            "L": [
                0b00000000,  #          
                0b10000000,  # *       
                0b10000000,  # *
                0b10000000,  # *
                0b10000000,  # *
                0b10000000,  # *
                0b10000000,  # *
                0b11110000,  # ****
            ],
            "m": [
                0b00000000,  #   
                0b00000000,  # 
                0b11110000,  # ****
                0b10101000,  # * * *
                0b10101000,  # * * *
                0b10101000,  # * * *
                0b10101000,  # * * *
                0b10101000,  # * * *
            ],
            "in": [
                0b00000000,  #   
                0b00000000,  #
                0b10011100,  # *   *** 
                0b00010010,  #     *  *
                0b10010010,  # *   *  *
                0b10010010,  # *   *  *
                0b10010010,  # *   *  *
                0b10010010,  # *   *  *
            ],
            "right_arrow_head": [
                0b00001000,  # ****   
                0b00001100,  #  ****  
                0b11111110,  #*******  
                0b11111111,  #********
                0b11111110,  #*******
                0b00001100,  #  ****
                0b00001000,  # ****
                0b00000000,  #
            ],
            "left_arrow_head": [
                0b0010000,  #   **** 
                0b00110000,  #  ***  
                0b01111111,  # ******* 
                0b11111111,  #********
                0b01111111,  # *******
                0b00110000,  #  ***
                0b00010000,  #   ****
                0b00000000,  #
            ],
            "stop": [
                0b00111100,  #  ****  
                0b01100010,  # **   * 
                0b11110001,  #****   * 
                0b10111001,  #* ***  *
                0b10011101,  #*  *** *
                0b10001111,  #*   ****
                0b01000110,  # *   ** 
                0b00111100,  #  ****  
            ],
        }
    
        
    def clear_screen(self):
        self.bl_ctrl(100)
        self.fill(self.FHAACHEN)
        self.show_up()
        self.fill(self.FHAACHEN)
        self.show_down()
        
    def draw_bitmap(self, bitmap, x_start, y_start, pixel_width, color):
        """
        Parameters:
        - bitmap: List of 8 bytes, every byte represents one row.
        - x_start: x-coordinate of the top-left corner.
        - y_start: x-coordinate of the top-left corner.
        - pixel_width: Width and height of each pixel block
        - color: Color to fill the pixels.
        """
        for row_index, byte in enumerate(bitmap):
            for bit_index in range(8):
                if (byte >> (7 - bit_index)) & 1:
                    x_pos = x_start + bit_index * pixel_width
                    y_pos = y_start + row_index * pixel_width

                    self.fill_rect(x_pos, y_pos, pixel_width, pixel_width, color)
                    
    def set_text(self, new_text):
        self.current_text = new_text
        self.draw_menu()
    
    def draw_menu(self):
        
        """
        Draws a numpad-like menu with the colors of FH Aachen
        """
        
        #first row upper half
        self.fill_rect(0,             0, self.width, self.height//2, self.OCEANBLUE)
        self.rect(0,             0, self.width, self.height//2, self.BLACK) # double black rect to rect like this makes the border 2pixels thick
        self.fill_rect(0, self.height//2, self.width, self.height//2, self.FHAACHEN) # borders this ^ line
        self.fill_rect(50, self.height//4 -12, self.width-100, 24, self.WHITE)
        self.rect(50, self.height//4 -12, self.width-100, 24, self.BLACK)
        self.text(self.current_text,60, self.height//4-3)

        #second row upper half
        self.rect(0, self.height//2, self.width//4, self.height//2, self.BLACK)
        self.rect(self.width//4, self.height//2, self.width//4, self.height//2, self.BLACK)
        self.rect(self.width//2, self.height//2, self.width//4, self.height//2, self.BLACK)
        self.rect(self.width*3//4, self.height//2, self.width//4, self.height//2, self.BLACK)

        #symbols
        self.draw_bitmap(bitmap = self.digits['1'], x_start = 15, y_start= 85, pixel_width = 10, color = self.BLACK)
        self.draw_bitmap(bitmap = self.digits['2'], x_start = 15+ self.width//4, y_start= 85, pixel_width = 10, color = self.BLACK)
        self.draw_bitmap(bitmap = self.digits['3'], x_start = 15+ self.width//2, y_start= 85, pixel_width = 10, color = self.BLACK)
        self.draw_bitmap(bitmap = self.symbols['backspace'], x_start = 15+ self.width*3//4, y_start= 85 - 10, pixel_width = 10, color = self.BLACK)

        self.show_up()
        self.fill(self.WHITE)

        #first row lower half (technically third row)
        self.fill_rect(0,             0, self.width, self.height//2, self.WHITE)
        self.rect(0, 0, self.width//4, self.height//2, self.BLACK)
        self.rect(self.width//4, 0, self.width//4, self.height//2, self.BLACK)
        self.rect(self.width//2, 0, self.width//4, self.height//2, self.BLACK)
        self.rect(self.width*3//4, 0, self.width//4, self.height//2, self.BLACK)

        #second row lower half (technically fourth row)
        self.fill_rect(0, self.height//2, self.width, self.height//2, self.BLACK)
        self.rect(0, self.height//2-1, self.width//4, self.height//2, self.WHITE)
        self.rect(self.width//4, self.height//2-1, self.width//4, self.height//2, self.WHITE)
        self.rect(self.width//2, self.height//2-1, self.width//4, self.height//2, self.WHITE)
        self.rect(self.width*3//4, self.height//2-1, self.width//4, self.height//2, self.WHITE)

        # symbols

        self.draw_bitmap(bitmap = self.digits['4'], x_start = 15, y_start= 5, pixel_width = 10, color = self.BLACK)
        self.draw_bitmap(bitmap = self.digits['5'], x_start = 15+ self.width//4, y_start= 5, pixel_width = 10, color = self.BLACK)
        self.draw_bitmap(bitmap = self.digits['6'], x_start = 15+ self.width//2, y_start= 5, pixel_width = 10, color = self.BLACK)
        self.draw_bitmap(bitmap = self.digits['0'], x_start = 15+ self.width*3//4, y_start= 5, pixel_width = 10, color = self.BLACK)

        self.draw_bitmap(bitmap = self.digits['7'], x_start = 15, y_start= 85, pixel_width = 10, color = self.WHITE)
        self.draw_bitmap(bitmap = self.digits['8'], x_start = 15+ self.width//4, y_start= 85-1, pixel_width = 10, color = self.WHITE)
        self.draw_bitmap(bitmap = self.digits['9'], x_start = 15+ self.width//2, y_start= 85-1, pixel_width = 10, color = self.WHITE)
        self.draw_bitmap(bitmap = self.symbols['enter'], x_start = 15+ self.width*3//4, y_start= 80, pixel_width = 10, color = self.WHITE)

        #enter key correction
        self.fill_rect(0,self.height-1, self.width, 1, self.BLACK)
        self.fill_rect(self.width*3//4+5, self.height//2, self.width//4-8, 2, self.BLACK)
        self.fill_rect(self.width*3//4+5, self.height-4, self.width//4-8, 2, self.BLACK)
        self.show_down()
    
    def screen_touched(self, X_Point, Y_Point):
        if X_Point > 0 and Y_Point > 0:
            # Only register touch if it hasn't been registered before
            if not self.is_touched:
                self.is_touched = True
                return True
            else:
                return False
        else:
            self.is_touched = False
            return False
    
    def flow_is_valid(self, current_text):
        if 300 <= int(self.current_text) <= 3000 and len(self.current_text)>0:
            return True
        self.set_text('Bitte nur Werte zwischen 300 und 3000')
        sleep(2)
        self.set_text('')
        return False
    
    def handle_touch(self, get):
        X_Point= 0
        Y_Point= 0
        
        if get != None: 
            X_Point = 480-int((get[1]-430)*480/3270) # added 480-... to make the Ursprung start left
            if(X_Point>480):
                X_Point = 480
            elif X_Point<0:
                X_Point = 0
                
            Y_Point = int((get[0]-430)*320/3270) # removed 320-... to make the Ursprung start at the top (and left^)--> so its the same as the pixel coordinates
            if(Y_Point>320):
                Y_Point = 320
            elif Y_Point<0:
                Y_Point = 0
            
            #print(X_Point, Y_Point)
        return X_Point, Y_Point
    
    def get_set_point(self, get):
        X_Point, Y_Point = self.handle_touch(get)
                
        if(self.screen_touched(X_Point, Y_Point) and self.first_touch):
            self.set_text('')
            self.first_touch = False
            
        # ACHTUNG: EXTREM SCHLECHTER CODE. FORTFAHREN AUF EIGENE GEWÄHR
        if (0 < X_Point < self.width//4) and (self.height//2 < Y_Point < self.height): # # 0 < X_Point < 120 # 80 < Y_Point < 160
            self.fill_rect(X_Point, Y_Point, self.width//4 -0, self.height-self.height//2, self.RED)
            self.current_text = self.current_text + '1'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (self.width//4 < X_Point < self.width//2) and (self.height//2 < Y_Point < self.height): # # 12 < X_Point < 240 # 80 < Y_Point < 160 
            self.current_text = self.current_text + '2'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (self.width//2 < X_Point < self.width*3//4) and (self.height//2 < Y_Point < self.height): # # 240 < X_Point < 360 # 80 < Y_Point < 160 
            self.current_text = self.current_text + '3'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (0 < X_Point < self.width//4) and (self.height < Y_Point < self.height*3//2): # # 0 < X_Point < 120 # 160 < Y_Point < 240 
            self.current_text = self.current_text + '4'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (self.width//4 < X_Point < self.width//2) and (self.height < Y_Point < self.height*3//2): # # 0 < X_Point < 120 # 160 < Y_Point < 240 
            self.current_text = self.current_text + '5'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (self.width//2 < X_Point < self.width*3//4) and (self.height < Y_Point < self.height*3//2): # # 0 < X_Point < 120 # 160 < Y_Point < 240 
            self.current_text = self.current_text + '6'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (self.width*3//4 < X_Point < self.width) and (self.height < Y_Point < self.height*3//2): # # 0 < X_Point < 120 # 160 < Y_Point < 240 
            self.current_text = self.current_text + '0'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (0 < X_Point < self.width//4) and (self.height*3//2< Y_Point < 2*self.height): # # 0 < X_Point < 120 # 160 < Y_Point < 240 
            self.current_text = self.current_text + '7'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (self.width//4 < X_Point < self.width//2) and (self.height*3//2< Y_Point < 2*self.height): # # 0 < X_Point < 120 # 160 < Y_Point < 240 
            self.current_text = self.current_text + '8'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (self.width//2 < X_Point < self.width*3//4) and (self.height*3//2< Y_Point < 2*self.height): # # 0 < X_Point < 120 # 160 < Y_Point < 240 
            self.current_text = self.current_text + '9'
            print(self.current_text)
            self.set_text(self.current_text)
            
        if (self.width*3//4 < X_Point < self.width) and (self.height//2 < Y_Point < self.height): # # 0 < X_Point < 120 # 160 < Y_Point < 240
            if len(self.current_text) > 0:
                self.current_text = self.current_text[:-1]
                print(self.current_text)
                self.set_text(self.current_text)
        try:
            if (self.width*3//4 < X_Point < self.width) and (self.height*3//2 < Y_Point < 2*self.height): # # 0 < X_Point < 120 # 160 < Y_Point < 240
                if self.flow_is_valid(self.current_text):                    
                    set_point = int(self.current_text)
                    return set_point
        except ValueError:
            self.current_text = "Bitte geben Sie einen Wert ein"
            print(self.current_text)            
            self.set_text(self.current_text)
            sleep(2)
            self.set_text('')
        return None
    
    
    
    
    def draw_flow_display(self):
        self.clear_screen()
        self.fill(self.FHAACHEN)
        self.draw_current_flow()
        self.draw_flow_unit()
        self.draw_max_flow_rate()
        self.draw_average_flow()
        
        self.show_up()
        
        self.draw_save_button_upper_half()
        self.show_up()
        
        self.fill(self.FHAACHEN)
        self.draw_save_button_lower_half()
        self.show_down()
        
    def draw_current_flow(self):
        self.text("Momentane Flussrate:", 5,10)
        current_flow_string = str(self.current_flow_rate)
        
        for i in range(len(current_flow_string)):
            self.draw_bitmap(bitmap = self.digits[current_flow_string[i]], x_start = 10+(8*7)*i, y_start= 30, pixel_width = 8, color = self.BLACK)

    def draw_flow_unit(self):
    
        self.draw_bitmap(bitmap = self.symbols["mu"], x_start = 20-40+(8*7*5), y_start= 5, pixel_width = 6, color = self.BLACK) #These arbitrary coordinates correspond with the places of the symbols of the unit 
        self.draw_bitmap(bitmap = self.symbols["L"], x_start = 70-40+(8*7*5), y_start= 5, pixel_width = 6, color = self.BLACK)      # ^
        self.fill_rect(287-40, 56, 95,7, self.BLACK)                                                                                # ^
        self.draw_bitmap(bitmap = self.symbols["m"], x_start = 15-40+(8*7*5), y_start= 55, pixel_width = 6, color = self.BLACK)     # ^
        self.draw_bitmap(bitmap = self.symbols["in"], x_start = 55-40+(8*7*5), y_start= 55, pixel_width = 6, color = self.BLACK)    # ^
            
    def draw_max_flow_rate(self):
        self.text("Max. Fluss:", 375,10)
        self.text(str(self.max_flow_rate), 395,25)
        self.draw_bitmap(bitmap = self.symbols["mu"], x_start = 430, y_start= 24, pixel_width = 1, color = self.BLACK)
        self.text("L/min",437,25)

    def draw_average_flow(self):
        #print("Counter:", self.flow_counter)
        if self.flow_counter < 1:
            self.average_flow_rate = self.current_flow_rate
        else:
            self.average_flow_rate = int((self.flow_counter * self.average_flow_rate + self.current_flow_rate) / (1+self.flow_counter))
        self.flow_counter += 1
        self.text("Mittl. Fluss:", 375, 60)
        self.text(str(self.average_flow_rate), 395, 75)
        self.draw_bitmap(bitmap = self.symbols["mu"], x_start = 430, y_start= 74, pixel_width = 1, color = self.BLACK)
        self.text("L/min", 437,75)

    def draw_save_button_upper_half(self):
        self.text("Knopf gedrueckt halten -->", 130, 150)
        self.ellipse(410,160,60,60, self.WHITE, True)
        self.text("Speichern und", 357,150)
        
    def draw_save_button_lower_half(self):
        self.ellipse(410, 0, 60, 60, self.WHITE, True)
        self.text("beenden", 381,0)
    
    def user_pressed_quit(self, get):
        x, y = self.handle_touch(get)
        if 410-60 <= x <= 410+60 and 160-60 <= y <= 160+60:
            return True
        
    def draw_syringe_screen(self):
                
        self.current_text = ""
        self.fill(self.FHAACHEN)
        self.text("Bitte entnehmen Sie die Spritze aus ihrer Halterung", 40, 20)
        self.text("und ziehen Sie sie auf ihre Anfangsposition.", 60, 35)
        
        for i in range(3):
            self.fill_rect(30+160*i, 60, 100, 100, self.BLACK)
        self.draw_syringe_screen_arrows()
        self.draw_syringe_screen_stop()
        
        self.show_up()
        self.fill(self.FHAACHEN)
        
        self.text("Der Fluss-Graph ist als .txt Datei nun abrufbar  ", 45,30)
        self.draw_continue_button()
        
        self.show_down()

    def draw_syringe_screen_arrows(self):
        self.draw_bitmap(bitmap = self.symbols["left_arrow_head"], x_start = 45, y_start= 85, pixel_width = 7, color = self.WHITE) #These arbitrary coordinates correspond with the places of the symbols of the unit 
        self.draw_bitmap(bitmap = self.symbols["right_arrow_head"], x_start = 60+160*2, y_start= 85, pixel_width = 7, color = self.WHITE) #These arbitrary coordinates correspond with the places of the symbols of the unit
        
    def draw_syringe_screen_stop(self):
        self.draw_bitmap(bitmap = self.symbols["stop"], x_start = 45+166, y_start= 80, pixel_width = 7, color = self.WHITE) #These arbitrary coordinates correspond with the places of the symbols of the unit 

    def right_arrow_pressed(self, touch_data):
        x, y = self.handle_touch(touch_data)
        if 350 <= x <= 450 and 60 <= y <= 160:
            return True
        
    def left_arrow_pressed(self, touch_data):
        x, y = self.handle_touch(touch_data)
        if 30 <= x <= 130 and 60 <= y <= 160:
            return True
        
    def stop_pressed(self, touch_data):
        x, y = self.handle_touch(touch_data)
        if 30+160 <= x <= 130+160 and 60 <= y <= 160:
            return True
   
    def draw_continue_button(self):
        self.fill_rect(20 , 60, 440, 80, 0)
        self.text("Weiter", 215, 95, self.WHITE)
        
    def continue_pressed(self, touch_data):
        x, y = self.handle_touch(touch_data)
        if 20 <= x <= 20+440 and 220 <= y <= 220+80:
            return True
        
    def save_txt_file(self, flow_array, Soll_wert):

        file_path = "/flow_data.txt"
        header = "Zeit(s)\tFlussrate(Mikroliter pro Minute)\tSollwert(Mikroliter pro Minute)\n"
        
        
        try:
            with open(file_path, 'w') as file:
                file.write(header)
                
                for i in range(0, len(flow_array), 2):
                    if i+1 < len(flow_array):
                        time = flow_array[i] / 1_000_000
                        flow = flow_array[i+1]
                        line = f"{time}\t{flow}\t{Soll_wert}\n"
                        file.write(line)
                    #file.flush()
                print("Data was written to {file_path}")
                return True 
        except OSError as e:
            print(f'failed to write file: {e}')
            return False
    

            
if __name__ == '__main__':
    lcd = MyScreen()
    lcd.draw_syringe_screen()
