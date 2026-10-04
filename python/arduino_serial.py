import time
import serial
from constants import ARDUINO_PORT
from terminal_logs import print_arduino_log

ARDUINO = serial.Serial(
    ARDUINO_PORT,
    115200,
)

def write_read(x): 
    ARDUINO.write(bytes(x, 'utf-8')) 
    time.sleep(0.05) 
    data = ARDUINO.readline().decode("utf-8").strip()
    return data

def write_to_arduino(message: str) -> None:
    ARDUINO.write(f"{message}\n".encode("utf-8"))


def read_from_arduino() -> str:
    return ARDUINO.readline().decode("utf-8").strip()

def read_and_log_arduino(): 
    response = read_from_arduino()
    is_error = response.startswith('ERROR,')
    print_arduino_log(response, is_error)