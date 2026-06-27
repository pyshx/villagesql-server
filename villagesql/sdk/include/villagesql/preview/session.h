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

#ifndef VILLAGESQL_PREVIEW_SESSION_H
#define VILLAGESQL_PREVIEW_SESSION_H

#include <cstdint>
#include <string>

#include <villagesql/abi/preview/session.h>
#include <villagesql/detail/capability_base.h>
#include <villagesql/detail/capability_traits.h>

namespace vsql::preview_session {

// Declare a SessionCapability by value and pass it to .with(). VEF populates it
// during registration. Getters read the session executing the current VDF call.
class SessionCapability
    : public ::vsql::detail::CapabilityBase<SessionCapability> {
 public:
  enum class Status {
    OK = VEF_SESSION_OK,
    UNAVAILABLE = VEF_SESSION_UNAVAILABLE,
    NOT_FOUND = VEF_SESSION_NOT_FOUND,
    TRUNCATED = VEF_SESSION_TRUNCATED,
    ERROR = VEF_SESSION_ERROR,
  };

  // On Status::OK, value holds the attribute; otherwise value is empty.
  struct StringResult {
    Status status;
    std::string value;
  };
  struct U32Result {
    Status status;
    uint32_t value;
  };

  [[nodiscard]] StringResult query_text() const;
  [[nodiscard]] StringResult query_digest() const;
  [[nodiscard]] StringResult host_or_ip() const;
  [[nodiscard]] StringResult query_charset() const;
  [[nodiscard]] StringResult sql_command() const;
  [[nodiscard]] StringResult command() const;
  [[nodiscard]] U32Result da_status() const;
  [[nodiscard]] StringResult login_user() const;
  [[nodiscard]] StringResult external_user() const;
  [[nodiscard]] StringResult proxy_user() const;
  [[nodiscard]] StringResult active_roles() const;

 private:
  template <typename Capability>
  friend struct ::vsql::detail::CapabilityTraits;

  StringResult call_str(vef_session_get_str_fn fn) const;

  const vef_preview_session_t *abi_ = nullptr;
};

}  // namespace vsql::preview_session

#include <villagesql/preview/detail/session_register.h>
#include <villagesql/preview/session_impl.h>

#endif  // VILLAGESQL_PREVIEW_SESSION_H
