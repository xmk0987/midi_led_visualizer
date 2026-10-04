from python.constants import (
    ARDUINO_COLOR,
    ERROR_COLOR,
    PYTHON_COLOR,
    RESET_COLOR,
)

def print_arduino_log(message: str, is_error: bool = False) -> None:
    message_color = ERROR_COLOR if is_error else ARDUINO_COLOR

    print(
        f"{ARDUINO_COLOR}Arduino:{RESET_COLOR} "
        f"{message_color}{message}{RESET_COLOR}"
    )

def print_python_log(message: str, is_error: bool = False) -> None:
    message_color = ERROR_COLOR if is_error else PYTHON_COLOR

    print(
        f"{PYTHON_COLOR}Python:{RESET_COLOR} "
        f"{message_color}{message}{RESET_COLOR}"
    )