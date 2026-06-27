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
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <https://www.gnu.org/licenses/>.

#ifndef VILLAGESQL_SERVICES_PREVIEW_SESSION_STRING_H
#define VILLAGESQL_SERVICES_PREVIEW_SESSION_STRING_H

#include <cstddef>
#include <cstring>

#include "villagesql/sdk/include/villagesql/abi/preview/session.h"

namespace villagesql::services {

// Copies [src, src+src_len) into buf as a NUL-terminated UTF-8 string without
// splitting a code point. *out_len is always set to src_len (bytes needed,
// excluding NUL). Returns VEF_SESSION_OK if the whole string fit, else
// VEF_SESSION_TRUNCATED with buf holding the largest UTF-8-safe prefix.
inline vef_session_result_t copy_utf8(char *buf, size_t buf_len,
                                      const char *src, size_t src_len,
                                      size_t *out_len) {
  if (out_len != nullptr) *out_len = src_len;
  if (buf == nullptr || buf_len == 0) return VEF_SESSION_TRUNCATED;
  if (src_len + 1 <= buf_len) {
    std::memcpy(buf, src, src_len);
    buf[src_len] = '\0';
    return VEF_SESSION_OK;
  }
  // Truncate: back up off any UTF-8 continuation byte (0b10xxxxxx) so the
  // copied prefix ends on a code-point boundary.
  size_t n = buf_len - 1;
  while (n > 0 && (static_cast<unsigned char>(src[n]) & 0xC0) == 0x80) --n;
  std::memcpy(buf, src, n);
  buf[n] = '\0';
  return VEF_SESSION_TRUNCATED;
}

}  // namespace villagesql::services

#endif  // VILLAGESQL_SERVICES_PREVIEW_SESSION_STRING_H
