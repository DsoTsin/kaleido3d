// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: request.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "request.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace protocol {

namespace {

const ::google_public::protobuf::Descriptor* meta_request_descriptor_ = NULL;
const ::google_public::protobuf::internal::GeneratedMessageReflection*
  meta_request_reflection_ = NULL;
const ::google_public::protobuf::EnumDescriptor* meta_request_Type_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_request_2eproto() {
  protobuf_AddDesc_request_2eproto();
  const ::google_public::protobuf::FileDescriptor* file =
    ::google_public::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "request.proto");
  GOOGLE_CHECK(file != NULL);
  meta_request_descriptor_ = file->message_type(0);
  static const int meta_request_offsets_[4] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(meta_request, uin_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(meta_request, type_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(meta_request, registerinfo_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(meta_request, statisticinfo_),
  };
  meta_request_reflection_ =
    new ::google_public::protobuf::internal::GeneratedMessageReflection(
      meta_request_descriptor_,
      meta_request::default_instance_,
      meta_request_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(meta_request, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(meta_request, _unknown_fields_),
      -1,
      ::google_public::protobuf::DescriptorPool::generated_pool(),
      ::google_public::protobuf::MessageFactory::generated_factory(),
      sizeof(meta_request));
  meta_request_Type_descriptor_ = meta_request_descriptor_->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google_public::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_request_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google_public::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    meta_request_descriptor_, &meta_request::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_request_2eproto() {
  delete meta_request::default_instance_;
  delete meta_request_reflection_;
}

void protobuf_AddDesc_request_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::protocol::protobuf_AddDesc_req_5fregister_2eproto();
  ::protocol::protobuf_AddDesc_req_5fstatistic_2eproto();
  ::google_public::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\rrequest.proto\022\010protocol\032\022req_register."
    "proto\032\023req_statistic.proto\"\272\001\n\014meta_requ"
    "est\022\013\n\003uin\030\001 \002(\t\022)\n\004type\030\002 \002(\0162\033.protoco"
    "l.meta_request.Type\022,\n\014registerInfo\030\003 \001("
    "\0132\026.protocol.req_register\022.\n\rstatisticIn"
    "fo\030\004 \001(\0132\027.protocol.req_statistic\"\024\n\004Typ"
    "e\022\014\n\010REGISTER\020\000", 255);
  ::google_public::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "request.proto", &protobuf_RegisterTypes);
  meta_request::default_instance_ = new meta_request();
  meta_request::default_instance_->InitAsDefaultInstance();
  ::google_public::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_request_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_request_2eproto {
  StaticDescriptorInitializer_request_2eproto() {
    protobuf_AddDesc_request_2eproto();
  }
} static_descriptor_initializer_request_2eproto_;

// ===================================================================

const ::google_public::protobuf::EnumDescriptor* meta_request_Type_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return meta_request_Type_descriptor_;
}
bool meta_request_Type_IsValid(int value) {
  switch(value) {
    case 0:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const meta_request_Type meta_request::REGISTER;
const meta_request_Type meta_request::Type_MIN;
const meta_request_Type meta_request::Type_MAX;
const int meta_request::Type_ARRAYSIZE;
#endif  // _MSC_VER
#ifndef _MSC_VER
const int meta_request::kUinFieldNumber;
const int meta_request::kTypeFieldNumber;
const int meta_request::kRegisterInfoFieldNumber;
const int meta_request::kStatisticInfoFieldNumber;
#endif  // !_MSC_VER

meta_request::meta_request()
  : ::google_public::protobuf::Message() {
  SharedCtor();
}

void meta_request::InitAsDefaultInstance() {
  registerinfo_ = const_cast< ::protocol::req_register*>(&::protocol::req_register::default_instance());
  statisticinfo_ = const_cast< ::protocol::req_statistic*>(&::protocol::req_statistic::default_instance());
}

meta_request::meta_request(const meta_request& from)
  : ::google_public::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
}

void meta_request::SharedCtor() {
  _cached_size_ = 0;
  uin_ = const_cast< ::std::string*>(&::google_public::protobuf::internal::kEmptyString);
  type_ = 0;
  registerinfo_ = NULL;
  statisticinfo_ = NULL;
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

meta_request::~meta_request() {
  SharedDtor();
}

void meta_request::SharedDtor() {
  if (uin_ != &::google_public::protobuf::internal::kEmptyString) {
    delete uin_;
  }
  if (this != default_instance_) {
    delete registerinfo_;
    delete statisticinfo_;
  }
}

void meta_request::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google_public::protobuf::Descriptor* meta_request::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return meta_request_descriptor_;
}

const meta_request& meta_request::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_request_2eproto();
  return *default_instance_;
}

meta_request* meta_request::default_instance_ = NULL;

meta_request* meta_request::New() const {
  return new meta_request;
}

void meta_request::Clear() {
  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (has_uin()) {
      if (uin_ != &::google_public::protobuf::internal::kEmptyString) {
        uin_->clear();
      }
    }
    type_ = 0;
    if (has_registerinfo()) {
      if (registerinfo_ != NULL) registerinfo_->::protocol::req_register::Clear();
    }
    if (has_statisticinfo()) {
      if (statisticinfo_ != NULL) statisticinfo_->::protocol::req_statistic::Clear();
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool meta_request::MergePartialFromCodedStream(
    ::google_public::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) return false
  ::google_public::protobuf::uint32 tag;
  while ((tag = input->ReadTag()) != 0) {
    switch (::google_public::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required string uin = 1;
      case 1: {
        if (::google_public::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google_public::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
          DO_(::google_public::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_uin()));
          ::google_public::protobuf::internal::WireFormat::VerifyUTF8String(
            this->uin().data(), this->uin().length(),
            ::google_public::protobuf::internal::WireFormat::PARSE);
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(16)) goto parse_type;
        break;
      }

      // required .protocol.meta_request.Type type = 2;
      case 2: {
        if (::google_public::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google_public::protobuf::internal::WireFormatLite::WIRETYPE_VARINT) {
         parse_type:
          int value;
          DO_((::google_public::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google_public::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::protocol::meta_request_Type_IsValid(value)) {
            set_type(static_cast< ::protocol::meta_request_Type >(value));
          } else {
            mutable_unknown_fields()->AddVarint(2, value);
          }
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(26)) goto parse_registerInfo;
        break;
      }

      // optional .protocol.req_register registerInfo = 3;
      case 3: {
        if (::google_public::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google_public::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_registerInfo:
          DO_(::google_public::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_registerinfo()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectTag(34)) goto parse_statisticInfo;
        break;
      }

      // optional .protocol.req_statistic statisticInfo = 4;
      case 4: {
        if (::google_public::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google_public::protobuf::internal::WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
         parse_statisticInfo:
          DO_(::google_public::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
               input, mutable_statisticinfo()));
        } else {
          goto handle_uninterpreted;
        }
        if (input->ExpectAtEnd()) return true;
        break;
      }

      default: {
      handle_uninterpreted:
        if (::google_public::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google_public::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          return true;
        }
        DO_(::google_public::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
  return true;
#undef DO_
}

void meta_request::SerializeWithCachedSizes(
    ::google_public::protobuf::io::CodedOutputStream* output) const {
  // required string uin = 1;
  if (has_uin()) {
    ::google_public::protobuf::internal::WireFormat::VerifyUTF8String(
      this->uin().data(), this->uin().length(),
      ::google_public::protobuf::internal::WireFormat::SERIALIZE);
    ::google_public::protobuf::internal::WireFormatLite::WriteString(
      1, this->uin(), output);
  }

  // required .protocol.meta_request.Type type = 2;
  if (has_type()) {
    ::google_public::protobuf::internal::WireFormatLite::WriteEnum(
      2, this->type(), output);
  }

  // optional .protocol.req_register registerInfo = 3;
  if (has_registerinfo()) {
    ::google_public::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      3, this->registerinfo(), output);
  }

  // optional .protocol.req_statistic statisticInfo = 4;
  if (has_statisticinfo()) {
    ::google_public::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      4, this->statisticinfo(), output);
  }

  if (!unknown_fields().empty()) {
    ::google_public::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
}

::google_public::protobuf::uint8* meta_request::SerializeWithCachedSizesToArray(
    ::google_public::protobuf::uint8* target) const {
  // required string uin = 1;
  if (has_uin()) {
    ::google_public::protobuf::internal::WireFormat::VerifyUTF8String(
      this->uin().data(), this->uin().length(),
      ::google_public::protobuf::internal::WireFormat::SERIALIZE);
    target =
      ::google_public::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->uin(), target);
  }

  // required .protocol.meta_request.Type type = 2;
  if (has_type()) {
    target = ::google_public::protobuf::internal::WireFormatLite::WriteEnumToArray(
      2, this->type(), target);
  }

  // optional .protocol.req_register registerInfo = 3;
  if (has_registerinfo()) {
    target = ::google_public::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        3, this->registerinfo(), target);
  }

  // optional .protocol.req_statistic statisticInfo = 4;
  if (has_statisticinfo()) {
    target = ::google_public::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        4, this->statisticinfo(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google_public::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  return target;
}

int meta_request::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required string uin = 1;
    if (has_uin()) {
      total_size += 1 +
        ::google_public::protobuf::internal::WireFormatLite::StringSize(
          this->uin());
    }

    // required .protocol.meta_request.Type type = 2;
    if (has_type()) {
      total_size += 1 +
        ::google_public::protobuf::internal::WireFormatLite::EnumSize(this->type());
    }

    // optional .protocol.req_register registerInfo = 3;
    if (has_registerinfo()) {
      total_size += 1 +
        ::google_public::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
          this->registerinfo());
    }

    // optional .protocol.req_statistic statisticInfo = 4;
    if (has_statisticinfo()) {
      total_size += 1 +
        ::google_public::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
          this->statisticinfo());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google_public::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void meta_request::MergeFrom(const ::google_public::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const meta_request* source =
    ::google_public::protobuf::internal::dynamic_cast_if_available<const meta_request*>(
      &from);
  if (source == NULL) {
    ::google_public::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void meta_request::MergeFrom(const meta_request& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_uin()) {
      set_uin(from.uin());
    }
    if (from.has_type()) {
      set_type(from.type());
    }
    if (from.has_registerinfo()) {
      mutable_registerinfo()->::protocol::req_register::MergeFrom(from.registerinfo());
    }
    if (from.has_statisticinfo()) {
      mutable_statisticinfo()->::protocol::req_statistic::MergeFrom(from.statisticinfo());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void meta_request::CopyFrom(const ::google_public::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void meta_request::CopyFrom(const meta_request& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool meta_request::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;

  if (has_registerinfo()) {
    if (!this->registerinfo().IsInitialized()) return false;
  }
  if (has_statisticinfo()) {
    if (!this->statisticinfo().IsInitialized()) return false;
  }
  return true;
}

void meta_request::Swap(meta_request* other) {
  if (other != this) {
    std::swap(uin_, other->uin_);
    std::swap(type_, other->type_);
    std::swap(registerinfo_, other->registerinfo_);
    std::swap(statisticinfo_, other->statisticinfo_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google_public::protobuf::Metadata meta_request::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google_public::protobuf::Metadata metadata;
  metadata.descriptor = meta_request_descriptor_;
  metadata.reflection = meta_request_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace protocol

// @@protoc_insertion_point(global_scope)
