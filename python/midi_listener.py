import mido
from terminal_logs import print_python_log
from constants import MIDI_INPUT_NAME
from arduino_serial import write_to_arduino, read_and_log_arduino,read_from_arduino

"""Sends one MIDI event and waits until Arduino is ready for another."""
def send_midi_event(message: str) -> None:
    write_to_arduino(message)

    response = read_from_arduino()

    if response != "OK":
        print_python_log(
            f"Unexpected Arduino response: {response}",
            True,
        )
        
def handle_key_release(message: mido.Message) -> None:
    send_midi_event(
        f"RELEASE,{message.note}"
    )


def handle_key_press(message: mido.Message) -> None:
    send_midi_event(
        f"PRESS,{message.note},{message.velocity}"
    )

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
