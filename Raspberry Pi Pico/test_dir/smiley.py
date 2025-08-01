from threeinch5.main_3inch5 import LCD_3inch5
from threeinch5.screen import MyScreen
from array import array

lcd = MyScreen()
lcd.init_display()
lcd.clear_screen()

coords = array('h', [0, 0, 480, 80])  # Three vertices (x0, y0, x1, y1, x2, y2)

# Draw an outlined polygon
lcd.poly(0, 0, coords, 1)

lcd.show_up()