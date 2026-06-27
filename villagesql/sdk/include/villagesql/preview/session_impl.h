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

// Calls a string getter identified by a pointer-to-member of
// vef_preview_session_t. Guards against a null abi_ or an unset slot in a
// single place. Grows the buffer once if the server reports
// VEF_SESSION_TRUNCATED.
inline SessionCapability::StringResult SessionCapability::call_str(
    vef_session_get_str_fn vef_preview_session_t::*field) const {
  if (abi_ == nullptr) return {Status::ERROR, {}};
  vef_session_get_str_fn fn = abi_->*field;
  if (fn == nullptr) return {Status::ERROR, {}};
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
  return call_str(&vef_preview_session_t::query_text);
}
inline SessionCapability::StringResult SessionCapability::query_digest() const {
  return call_str(&vef_preview_session_t::query_digest);
}
inline SessionCapability::StringResult SessionCapability::host_or_ip() const {
  return call_str(&vef_preview_session_t::host_or_ip);
}
inline SessionCapability::StringResult SessionCapability::query_charset()
    const {
  return call_str(&vef_preview_session_t::query_charset);
}
inline SessionCapability::StringResult SessionCapability::sql_command() const {
  return call_str(&vef_preview_session_t::sql_command);
}
inline SessionCapability::StringResult SessionCapability::command() const {
  return call_str(&vef_preview_session_t::command);
}
inline SessionCapability::StringResult SessionCapability::login_user() const {
  return call_str(&vef_preview_session_t::login_user);
}
inline SessionCapability::StringResult SessionCapability::external_user()
    const {
  return call_str(&vef_preview_session_t::external_user);
}
inline SessionCapability::StringResult SessionCapability::proxy_user() const {
  return call_str(&vef_preview_session_t::proxy_user);
}
inline SessionCapability::StringResult SessionCapability::active_roles() const {
  return call_str(&vef_preview_session_t::active_roles);
}

inline SessionCapability::U32Result SessionCapability::da_status() const {
  if (abi_ == nullptr || abi_->da_status == nullptr) return {Status::ERROR, 0};
  uint32_t v = 0;
  vef_session_result_t r = abi_->da_status(&v);
  return {static_cast<Status>(r), (r == VEF_SESSION_OK) ? v : 0u};
}

}  // namespace vsql::preview_session

#endif  // VILLAGESQL_PREVIEW_SESSION_IMPL_H
