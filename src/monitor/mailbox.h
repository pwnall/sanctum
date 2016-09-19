#if !defined(MONITOR_MAILBOX_H_INCLUDED)
#define MONITOR_MAILBOX_H_INCLUDED

#include "bare/base_types.h"
#include "crypto/hash.h"
#include "public/api.h"

namespace sanctum {
namespace internal {

using sanctum::api::enclave_id_t;
using sanctum::api::enclave::mailbox_message_size;

// Metadata for one mailbox.
struct mailbox_t {
  size_t state;

  // The OS-assigned enclave ID of the expected sender.
  //
  // This is necessary to prevent a malicious enclave from DoSing other
  // enclaves in the system by spamming their mailboxes. This enclave ID should
  // not be trusted to identify the software inside the sender.
  enclave_id_t sender_id;

  // The measurement of the expected sender.
  //
  // This is a secure identifier for the software inside the sender enclave.
  hash_state_t sender_hash;

  // The message held by the mailbox.
  uintptr_t message[mailbox_message_size / sizeof(uintptr_t)];
};

};  // namespace sanctum::internal
};  // namespace sanctum
#endif  // !defined(MONITOR_MAILBOX_H_INCLUDED)
