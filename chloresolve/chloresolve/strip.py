import re

CODE_NORMAL = b'\x0F'
CODE_BOLD = b'\x02'
CODE_ITALICS = b'\x1D'
CODE_UNDERLINE = b'\x1F'
CODE_COLOR = b'\x03'

CODE_ATTRIBS = frozenset(byte for byte in b"\x0F\x02\x1D\x1F")
CODE_STRIP_COLORS = re.compile(rb"\x03(?P<fg>\d{2})(,(?P<bg>\d{2}))?")


def strip_irc_attributes(buffer: bytes) -> bytes:
    """
    Strips mIRC colors and attributes from the message.
    """
    # Strip attributes first
    stripped: bytes = buffer
    for attrib in (CODE_NORMAL, CODE_BOLD, CODE_ITALICS, CODE_UNDERLINE):
        stripped = stripped.replace(attrib, b'')

    # Time to strip colors
    return re.sub(CODE_STRIP_COLORS, b'', stripped)


def is_action(buffer: bytes) -> bool:
    """
    Returns whether this message is a '/me' action command or not.
    """
    return buffer.startswith(b'\x01ACTION') and buffer.endswith('\x01')


def strip_action(buffer: bytes) -> bytes:
    """
    Returns a message with its '/me' action command encoding stripped.
    """
    return buffer.removeprefix(b'\x01ACTION').removesuffix(b'\x01')
