/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C_NO_DEPRECATED
#define PROTOBUF_C_NO_DEPRECATED
#endif

#include "lspmessage.pb-c.h"
#include <google/protobuf-c/protobuf-c.h>
void   lspmessage__init
                     (LSPMessage         *message)
{
  static LSPMessage init_value = LSPMESSAGE__INIT;
  *message = init_value;
}
size_t lspmessage__get_packed_size
                     (const LSPMessage *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &lspmessage__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t lspmessage__pack
                     (const LSPMessage *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &lspmessage__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t lspmessage__pack_to_buffer
                     (const LSPMessage *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &lspmessage__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
LSPMessage *
       lspmessage__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (LSPMessage *)
     protobuf_c_message_unpack (&lspmessage__descriptor,
                                allocator, len, data);
}
void   lspmessage__free_unpacked
                     (LSPMessage *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &lspmessage__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor lspmessage__field_descriptors[3] =
{
  {
    "connid",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(LSPMessage, connid),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "seqnum",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(LSPMessage, seqnum),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "payload",
    3,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(LSPMessage, payload),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned lspmessage__field_indices_by_name[] = {
  0,   /* field[0] = connid */
  2,   /* field[2] = payload */
  1,   /* field[1] = seqnum */
};
static const ProtobufCIntRange lspmessage__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor lspmessage__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "LSPMessage",
  "LSPMessage",
  "LSPMessage",
  "",
  sizeof(LSPMessage),
  3,
  lspmessage__field_descriptors,
  lspmessage__field_indices_by_name,
  1,  lspmessage__number_ranges,
  (ProtobufCMessageInit) lspmessage__init,
  NULL,NULL,NULL    /* reserved[123] */
};
