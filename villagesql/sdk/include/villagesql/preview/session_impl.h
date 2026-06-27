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
// PREVIEW CAPABILITY — UNSTABLE API
// =============================================================================
// This header is part of the VEF preview surface. Its API and ABI may change
// or be removed without notice. See villagesql/preview/README.md for details.
// =============================================================================

#ifndef VILLAGESQL_PREVIEW_SESSION_IMPL_H
#define VILLAGESQL_PREVIEW_SESSION_IMPL_H

#include <utility>

#include <villagesql/abi/preview/session.h>
#include <villagesql/preview/session.h>

namespace vsql::preview_session {

// Calls a string getter, growing the buffer once if the server reports the
// value did not fit (VEF_SESSION_TRUNCATED with a needed size).
inline SessionCapability::StringResult SessionCapability::call_str(
    vef_session_get_str_fn fn) const {
  std::string value;
  value.resize(256);
  size_t out_len = 0;
  vef_session_result_t r = fn(value.data(), value.size(), &out_len);
  if (r == VEF_SESSION_TRUNCATED && out_len + 1 > value.size()) {
    value.resize(out_len + 1);
    r = fn(value.data(), value.size(), &out_len);
  }
  if (r == VEF_SESSION_OK) {
    value.resize(out_len);
    return {Status::OK, std::move(value)};
  }
  return {static_cast<Status>(r), {}};
}

inline SessionCapability::StringResult SessionCapability::query_text() const {
  return call_str(abi_->query_text);
}
inline SessionCapability::StringResult SessionCapability::query_digest() const {
  return call_str(abi_->query_digest);
}
inline SessionCapability::StringResult SessionCapability::host_or_ip() const {
  return call_str(abi_->host_or_ip);
}
inline SessionCapability::StringResult SessionCapability::query_charset()
    const {
  return call_str(abi_->query_charset);
}
inline SessionCapability::StringResult SessionCapability::sql_command() const {
  return call_str(abi_->sql_command);
}
inline SessionCapability::StringResult SessionCapability::command() const {
  return call_str(abi_->command);
}
inline SessionCapability::StringResult SessionCapability::login_user() const {
  return call_str(abi_->login_user);
}
inline SessionCapability::StringResult SessionCapability::external_user()
    const {
  return call_str(abi_->external_user);
}
inline SessionCapability::StringResult SessionCapability::proxy_user() const {
  return call_str(abi_->proxy_user);
}
inline SessionCapability::StringResult SessionCapability::active_roles() const {
  return call_str(abi_->active_roles);
}

inline SessionCapability::U32Result SessionCapability::da_status() const {
  uint32_t v = 0;
  vef_session_result_t r = abi_->da_status(&v);
  return {static_cast<Status>(r), (r == VEF_SESSION_OK) ? v : 0u};
}

}  // namespace vsql::preview_session

#endif  // VILLAGESQL_PREVIEW_SESSION_IMPL_H
