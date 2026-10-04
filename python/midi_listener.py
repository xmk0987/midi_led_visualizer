import mido
from terminal_logs import print_python_log
from constants import MIDI_INPUT_NAME
from arduino_serial import write_to_arduino, read_and_log_arduino,read_from_arduino

is_pedal_on = False

PEDAL_CONTROL_VALUE=64
PEDAL_ON_THRESHOLD=64

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
    
def handle_note(message: mido.Message) -> None:
    if message.velocity > 0:
        handle_key_press(message)
    else:
        handle_key_release(message)
        
def handle_pedal_on() -> None:
    global is_pedal_on

    is_pedal_on = True
    print_python_log("PEDAL ON")
    send_midi_event("PEDAL_ON")


def handle_pedal_off() -> None:
    global is_pedal_on

    is_pedal_on = False
    print_python_log("PEDAL OFF")
    send_midi_event("PEDAL_OFF")
    
def handle_pedal(message: mido.Message) -> None:
    pedal_is_down = message.value >= PEDAL_ON_THRESHOLD
    
    if pedal_is_down and not is_pedal_on:
        handle_pedal_on()
    elif not pedal_is_down and is_pedal_on:
        handle_pedal_off()

def listen_to_midi():
    with mido.open_input(MIDI_INPUT_NAME) as midi_input:
        print_python_log(f"Listening to {MIDI_INPUT_NAME}")
        for message in midi_input:
            print_python_log(f"Message type received: {message.type}")
            print_python_log(f"Whole message: {message}")
            if message.type == "note_on":
                handle_note(message)
                    
            elif (
                message.type == "control_change"
                and message.control == PEDAL_CONTROL_VALUE
            ):
                handle_pedal(message)
