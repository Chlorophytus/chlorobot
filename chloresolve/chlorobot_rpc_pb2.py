# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: chlorobot_rpc.proto
# Protobuf Python Version: 5.26.1
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x13\x63hlorobot_rpc.proto\"\xc1\x02\n\x0f\x43hlorobotPacket\x12\x15\n\x08prefix_0\x18\x01 \x01(\tH\x01\x88\x01\x01\x12\x13\n\x06prefix\x18\x06 \x01(\x0cH\x02\x88\x01\x01\x12\x15\n\x0bnon_numeric\x18\x07 \x01(\x0cH\x00\x12\x17\n\rnon_numeric_0\x18\x02 \x01(\tH\x00\x12\x11\n\x07numeric\x18\x03 \x01(\rH\x00\x12\x12\n\nparameters\x18\x08 \x03(\x0c\x12\x14\n\x0cparameters_0\x18\x04 \x03(\t\x12\x1f\n\x12trailing_parameter\x18\t \x01(\x0cH\x03\x88\x01\x01\x12!\n\x14trailing_parameter_0\x18\x05 \x01(\tH\x04\x88\x01\x01\x42\t\n\x07\x63ommandB\x0b\n\t_prefix_0B\t\n\x07_prefixB\x15\n\x13_trailing_parameterB\x17\n\x15_trailing_parameter_0\"\x95\x01\n\x10\x43hlorobotRequest\x12&\n\x04\x61uth\x18\x01 \x01(\x0b\x32\x18.ChlorobotAuthentication\x12\"\n\x06packet\x18\x02 \x01(\x0b\x32\x10.ChlorobotPacketH\x00\x12-\n\x0c\x63ommand_type\x18\x03 \x01(\x0e\x32\x15.ChlorobotCommandEnumH\x00\x42\x06\n\x04\x64\x61ta\"Y\n\x17\x43hlorobotAuthentication\x12\x12\n\x05token\x18\x01 \x01(\tH\x00\x88\x01\x01\x12\x14\n\x07version\x18\x02 \x01(\rH\x01\x88\x01\x01\x42\x08\n\x06_tokenB\n\n\x08_version\"O\n\x10\x43hlorobotVersion\x12\r\n\x05major\x18\x01 \x01(\r\x12\r\n\x05minor\x18\x02 \x01(\r\x12\r\n\x05patch\x18\x03 \x01(\r\x12\x0e\n\x06pretty\x18\x04 \x01(\t\"O\n\x18\x43hlorobotAcknowledgement\x12\'\n\x07version\x18\x01 \x01(\x0b\x32\x11.ChlorobotVersionH\x00\x88\x01\x01\x42\n\n\x08_version*I\n\x14\x43hlorobotCommandEnum\x12\x10\n\x0cSEND_NOTHING\x10\x00\x12\x10\n\x0cSEND_VERSION\x10\x01\x12\r\n\tSEND_QUIT\x10\x02\x32\x80\x01\n\x0c\x43hlorobotRPC\x12\x38\n\x06Listen\x12\x18.ChlorobotAuthentication\x1a\x10.ChlorobotPacket\"\x00\x30\x01\x12\x36\n\x04Send\x12\x11.ChlorobotRequest\x1a\x19.ChlorobotAcknowledgement\"\x00\x62\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'chlorobot_rpc_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  DESCRIPTOR._loaded_options = None
  _globals['_CHLOROBOTCOMMANDENUM']._serialized_start=752
  _globals['_CHLOROBOTCOMMANDENUM']._serialized_end=825
  _globals['_CHLOROBOTPACKET']._serialized_start=24
  _globals['_CHLOROBOTPACKET']._serialized_end=345
  _globals['_CHLOROBOTREQUEST']._serialized_start=348
  _globals['_CHLOROBOTREQUEST']._serialized_end=497
  _globals['_CHLOROBOTAUTHENTICATION']._serialized_start=499
  _globals['_CHLOROBOTAUTHENTICATION']._serialized_end=588
  _globals['_CHLOROBOTVERSION']._serialized_start=590
  _globals['_CHLOROBOTVERSION']._serialized_end=669
  _globals['_CHLOROBOTACKNOWLEDGEMENT']._serialized_start=671
  _globals['_CHLOROBOTACKNOWLEDGEMENT']._serialized_end=750
  _globals['_CHLOROBOTRPC']._serialized_start=828
  _globals['_CHLOROBOTRPC']._serialized_end=956
# @@protoc_insertion_point(module_scope)
