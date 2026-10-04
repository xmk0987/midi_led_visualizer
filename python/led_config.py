
from tkinter import Tk, colorchooser
from constants import RESET_COLOR, PYTHON_COLOR
from arduino_serial import read_and_log_arduino, write_to_arduino
from terminal_logs import print_python_log

def get_color_from_picker():
    root = Tk()
    root.withdraw()

    rgb, _ = colorchooser.askcolor(
        parent=root,
        title="Choose LED color",
    )

    root.destroy()

    if rgb is None:
        return None

    return list(rgb)

def get_color_input():
    while True:
        color_input = input(
            f"{PYTHON_COLOR}Python: Enter RGB r,g,b or 'q' to quit: {RESET_COLOR}"
        )

        if color_input.lower() == "q":
            return None

        parts = color_input.split(",")

        if len(parts) != 3:
            print_python_log("RGB must contain exactly three values.", True)
            continue

        try:
            rgb = [int(value.strip()) for value in parts]
        except ValueError:
            print_python_log("RGB values must be numbers.", True)
            continue

        is_all_in_rgb_range = all(0 <= value <= 255 for value in rgb)

        if not is_all_in_rgb_range:
            print_python_log("RGB values must be between 0 and 255.", True)
            continue

        return rgb
    
def get_brigthness_input():
    while True:   
        brightness_input = input(
            f"{PYTHON_COLOR}Python: Enter brightness 0-255 or 'q' to quit: {RESET_COLOR}"
        )

        if brightness_input.lower() == "q":
            return None

        try:
            stripped_brightness = int(brightness_input.strip())
        except ValueError:
            print_python_log("Brightness has to be a number", True)
            continue
        
        is_in_brightness_range = 0 <= stripped_brightness <= 255
        if not is_in_brightness_range:
            print_python_log("Brightness must be between 0 and 255.", True)
            continue

        return stripped_brightness
    
def get_and_send_config_to_arduino():
    "Gets config to arduino"
    rgb = get_color_from_picker()
    
    if rgb is None:
        return
    
    brightness = get_brigthness_input()
    
    write_to_arduino(
        f"CONFIG,{rgb[0]},{rgb[1]},{rgb[2]},{brightness}"
    )
    read_and_log_arduino()