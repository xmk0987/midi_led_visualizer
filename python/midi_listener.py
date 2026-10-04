import mido
from python.terminal_logs import print_python_log
from python.constants import MIDI_INPUT_NAME
from python.arduino_serial import write_to_arduino, read_and_log_arduino

def handle_key_release(message: mido.Message):
    event = "RELEASE"
    note_name = message.note
    
    arduino_message = f"{event},{note_name}" 
    write_to_arduino(arduino_message)

def handle_key_press(message: mido.Message):
    event = "PRESS"
    note_name = message.note
    velocity = message.velocity

    arduino_message = f"{event},{note_name},{velocity}" 
    write_to_arduino(arduino_message)

"""Prints whether piano keys are pressed or released."""
def listen_to_midi():
    with mido.open_input(MIDI_INPUT_NAME) as midi_input:
        print_python_log(f"Listening to {MIDI_INPUT_NAME}")
        for message in midi_input:
            if message.type != "note_on":
                continue
            
            if message.velocity > 0:
                handle_key_press(message)
            else:
                handle_key_release(message)
