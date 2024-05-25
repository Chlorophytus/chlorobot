from google.protobuf.internal import containers as _containers
from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class ChlorobotCommandEnum(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = ()
    SEND_NOTHING: _ClassVar[ChlorobotCommandEnum]
    SEND_VERSION: _ClassVar[ChlorobotCommandEnum]
SEND_NOTHING: ChlorobotCommandEnum
SEND_VERSION: ChlorobotCommandEnum

class ChlorobotPacket(_message.Message):
    __slots__ = ("prefix", "non_numeric", "numeric", "parameters", "trailing_parameter")
    PREFIX_FIELD_NUMBER: _ClassVar[int]
    NON_NUMERIC_FIELD_NUMBER: _ClassVar[int]
    NUMERIC_FIELD_NUMBER: _ClassVar[int]
    PARAMETERS_FIELD_NUMBER: _ClassVar[int]
    TRAILING_PARAMETER_FIELD_NUMBER: _ClassVar[int]
    prefix: str
    non_numeric: str
    numeric: int
    parameters: _containers.RepeatedScalarFieldContainer[str]
    trailing_parameter: str
    def __init__(self, prefix: _Optional[str] = ..., non_numeric: _Optional[str] = ..., numeric: _Optional[int] = ..., parameters: _Optional[_Iterable[str]] = ..., trailing_parameter: _Optional[str] = ...) -> None: ...

class ChlorobotAuthentication(_message.Message):
    __slots__ = ("token",)
    TOKEN_FIELD_NUMBER: _ClassVar[int]
    token: str
    def __init__(self, token: _Optional[str] = ...) -> None: ...

class ChlorobotRequest(_message.Message):
    __slots__ = ("auth", "packet", "command_type")
    AUTH_FIELD_NUMBER: _ClassVar[int]
    PACKET_FIELD_NUMBER: _ClassVar[int]
    COMMAND_TYPE_FIELD_NUMBER: _ClassVar[int]
    auth: ChlorobotAuthentication
    packet: ChlorobotPacket
    command_type: ChlorobotCommandEnum
    def __init__(self, auth: _Optional[_Union[ChlorobotAuthentication, _Mapping]] = ..., packet: _Optional[_Union[ChlorobotPacket, _Mapping]] = ..., command_type: _Optional[_Union[ChlorobotCommandEnum, str]] = ...) -> None: ...

class ChlorobotVersion(_message.Message):
    __slots__ = ("major", "minor", "patch", "pretty")
    MAJOR_FIELD_NUMBER: _ClassVar[int]
    MINOR_FIELD_NUMBER: _ClassVar[int]
    PATCH_FIELD_NUMBER: _ClassVar[int]
    PRETTY_FIELD_NUMBER: _ClassVar[int]
    major: int
    minor: int
    patch: int
    pretty: str
    def __init__(self, major: _Optional[int] = ..., minor: _Optional[int] = ..., patch: _Optional[int] = ..., pretty: _Optional[str] = ...) -> None: ...

class ChlorobotAcknowledgement(_message.Message):
    __slots__ = ("version",)
    VERSION_FIELD_NUMBER: _ClassVar[int]
    version: ChlorobotVersion
    def __init__(self, version: _Optional[_Union[ChlorobotVersion, _Mapping]] = ...) -> None: ...
