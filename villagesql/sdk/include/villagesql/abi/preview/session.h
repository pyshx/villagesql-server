// Copyright (c) 2026 VillageSQL Contributors
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <https://www.gnu.org/licenses/>.

// =============================================================================
// VEF PREVIEW ABI HEADER — UNSTABLE BINARY INTERFACE
// =============================================================================
// This header is both:
//   - an ABI header — extension authors should use the C++ API in
//     <villagesql/vsql.h>, not these raw types. See villagesql/abi/README.md.
//   - a preview capability — API and ABI may change or be removed without
//     notice. See villagesql/preview/README.md.
// =============================================================================

#ifndef VILLAGESQL_ABI_PREVIEW_SESSION_H
#define VILLAGESQL_ABI_PREVIEW_SESSION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Preview capability: "vsql::preview::session"
//
// Read-only access to expensive/variable session attributes that do not belong
// on the hot vef_context_t path. Extensions declare a SessionCapability and
// call the getters below; the server reads current_thd at call time.
//
// Capability name: VEF_PREVIEW_SESSION_NAME

#define VEF_PREVIEW_SESSION_NAME "vsql::preview::session"
#define VEF_PREVIEW_SESSION_ABI_VERSION 1

typedef enum {
  VEF_SESSION_OK = 0,
  VEF_SESSION_UNAVAILABLE = 1,  // no bound session (no current_thd)
  VEF_SESSION_NOT_FOUND = 2,    // attribute has no value (e.g. no query digest)
  VEF_SESSION_TRUNCATED = 3,  // buffer too small; *out_len is the needed bytes
  VEF_SESSION_ERROR = 4,      // service/conversion failure
} vef_session_result_t;

// String getter. Copies UTF-8 into buf (NUL-terminated), never splitting a
// code point. *out_len is set to the byte count excluding the NUL; on
// VEF_SESSION_TRUNCATED it is the total bytes needed and buf holds the largest
// UTF-8-safe prefix. No my_h_string crosses this ABI.
typedef vef_session_result_t (*vef_session_get_str_fn)(char *buf,
                                                       size_t buf_len,
                                                       size_t *out_len);

// Unsigned 32-bit getter (e.g. da_status bitmask).
typedef vef_session_result_t (*vef_session_get_u32_fn)(uint32_t *out);

typedef struct {
  // Capability ABI version. Always the first field in every capability vtable.
  uint32_t version;

  // version >= 1 — values from mysql_thd_attributes
  vef_session_get_str_fn query_text;     // "sql_text"
  vef_session_get_str_fn query_digest;   // "query_digest"
  vef_session_get_str_fn host_or_ip;     // "host_or_ip"
  vef_session_get_str_fn query_charset;  // "query_charset"
  vef_session_get_str_fn sql_command;    // "sql_command"
  vef_session_get_str_fn command;        // "command"
  vef_session_get_u32_fn da_status;      // "da_status" bitmask

  // version >= 1 — values from Security_context
  vef_session_get_str_fn login_user;     // login user
  vef_session_get_str_fn external_user;  // external user
  vef_session_get_str_fn proxy_user;     // proxy user
  vef_session_get_str_fn active_roles;   // comma-separated user@host, or "NONE"
} vef_preview_session_t;

#ifdef __cplusplus
}
#endif

#endif  // VILLAGESQL_ABI_PREVIEW_SESSION_H
