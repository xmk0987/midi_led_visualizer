from python.led_config import get_and_send_config_to_arduino
from python.midi_listener import listen_to_midi

def main() -> None:
    get_and_send_config_to_arduino()
    listen_to_midi()
    
if __name__ == "__main__":
    main()