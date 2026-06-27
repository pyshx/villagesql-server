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

#include "villagesql/services/preview/session.h"

#include <cstring>
#include <string>

#include "mysql/components/my_service.h"
#include "mysql/components/services/defs/mysql_string_defs.h"
#include "mysql/components/services/mysql_string.h"
#include "mysql/components/services/mysql_thd_attributes.h"
#include "mysql/service_plugin_registry.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"
#include "sql/sql_class.h"
#include "villagesql/services/preview/session_string.h"

namespace villagesql::services {

namespace {

// Copies a server-owned LEX_CSTRING (Security_context identity) into buf.
vef_session_result_t copy_lex_cstring(LEX_CSTRING s, char *buf, size_t buf_len,
                                      size_t *out_len) {
  if (s.str == nullptr) {
    if (out_len != nullptr) *out_len = 0;
    return VEF_SESSION_NOT_FOUND;
  }
  return copy_utf8(buf, buf_len, s.str, s.length, out_len);
}

// --- Security_context getters ---

vef_session_result_t get_login_user(char *buf, size_t buf_len,
                                    size_t *out_len) {
  THD *thd = current_thd;
  if (thd == nullptr) return VEF_SESSION_UNAVAILABLE;
  return copy_lex_cstring(thd->security_context()->user(), buf, buf_len,
                          out_len);
}
vef_session_result_t get_external_user(char *buf, size_t buf_len,
                                       size_t *out_len) {
  THD *thd = current_thd;
  if (thd == nullptr) return VEF_SESSION_UNAVAILABLE;
  return copy_lex_cstring(thd->security_context()->external_user(), buf,
                          buf_len, out_len);
}
vef_session_result_t get_proxy_user(char *buf, size_t buf_len,
                                    size_t *out_len) {
  THD *thd = current_thd;
  if (thd == nullptr) return VEF_SESSION_UNAVAILABLE;
  return copy_lex_cstring(thd->security_context()->proxy_user(), buf, buf_len,
                          out_len);
}

// Comma-separated active roles ("user`@`host,..." or "NONE"), formatted by the
// same server routine that backs CURRENT_ROLE().
vef_session_result_t get_active_roles(char *buf, size_t buf_len,
                                      size_t *out_len) {
  THD *thd = current_thd;
  if (thd == nullptr) return VEF_SESSION_UNAVAILABLE;
  String roles;
  func_current_role(thd, &roles);
  return copy_utf8(buf, buf_len, roles.ptr(), roles.length(), out_len);
}

// --- mysql_thd_attributes: my_h_string family
// (sql_text/query_digest/host_or_ip) ---

vef_session_result_t get_thd_owned_string(const char *name, char *buf,
                                          size_t buf_len, size_t *out_len) {
  THD *thd = current_thd;
  if (thd == nullptr) return VEF_SESSION_UNAVAILABLE;
  SERVICE_TYPE(registry) *registry = mysql_plugin_registry_acquire();
  if (registry == nullptr) return VEF_SESSION_ERROR;

  vef_session_result_t result = VEF_SESSION_ERROR;
  {
    my_service<SERVICE_TYPE(mysql_thd_attributes)> attrs("mysql_thd_attributes",
                                                         registry);
    my_service<SERVICE_TYPE(mysql_string_factory)> sfactory(
        "mysql_string_factory", registry);
    my_service<SERVICE_TYPE(mysql_string_converter)> sconv(
        "mysql_string_converter", registry);
    my_service<SERVICE_TYPE(mysql_string_byte_access)> sbytes(
        "mysql_string_byte_access", registry);
    if (attrs.is_valid() && sfactory.is_valid() && sconv.is_valid() &&
        sbytes.is_valid()) {
      my_h_string h = nullptr;
      // get() returns true on failure / no value.
      if (attrs->get(thd, name, &h) || h == nullptr) {
        result = VEF_SESSION_NOT_FOUND;
      } else {
        uint blen = 0;
        if (sbytes->get_byte_length(h, &blen)) {
          result = VEF_SESSION_ERROR;
        } else {
          std::string tmp(static_cast<size_t>(blen) + 1, '\0');
          if (sconv->convert_to_buffer(h, tmp.data(), tmp.size(), "utf8mb4")) {
            result = VEF_SESSION_ERROR;
          } else {
            result = copy_utf8(buf, buf_len, tmp.data(),
                               std::strlen(tmp.data()), out_len);
          }
        }
        sfactory->destroy(h);
      }
    }
  }
  mysql_plugin_registry_release(registry);
  return result;
}

// --- mysql_thd_attributes: non-owning cstring family
// (query_charset/sql_command/command) ---

vef_session_result_t get_thd_cstring(const char *name, char *buf,
                                     size_t buf_len, size_t *out_len) {
  THD *thd = current_thd;
  if (thd == nullptr) return VEF_SESSION_UNAVAILABLE;
  SERVICE_TYPE(registry) *registry = mysql_plugin_registry_acquire();
  if (registry == nullptr) return VEF_SESSION_ERROR;

  vef_session_result_t result = VEF_SESSION_ERROR;
  {
    my_service<SERVICE_TYPE(mysql_thd_attributes)> attrs("mysql_thd_attributes",
                                                         registry);
    if (attrs.is_valid()) {
      mysql_cstring_with_length val{nullptr, 0};
      if (attrs->get(thd, name, &val) || val.str == nullptr) {
        result = VEF_SESSION_NOT_FOUND;
      } else {
        result = copy_utf8(buf, buf_len, val.str, val.length, out_len);
      }
    }
  }
  mysql_plugin_registry_release(registry);
  return result;
}

vef_session_result_t get_query_text(char *b, size_t l, size_t *o) {
  return get_thd_owned_string("sql_text", b, l, o);
}
vef_session_result_t get_query_digest(char *b, size_t l, size_t *o) {
  return get_thd_owned_string("query_digest", b, l, o);
}
vef_session_result_t get_host_or_ip(char *b, size_t l, size_t *o) {
  return get_thd_owned_string("host_or_ip", b, l, o);
}
vef_session_result_t get_query_charset(char *b, size_t l, size_t *o) {
  return get_thd_cstring("query_charset", b, l, o);
}
vef_session_result_t get_sql_command(char *b, size_t l, size_t *o) {
  return get_thd_cstring("sql_command", b, l, o);
}
vef_session_result_t get_command(char *b, size_t l, size_t *o) {
  return get_thd_cstring("command", b, l, o);
}

// --- da_status (uint16 widened to uint32) ---

vef_session_result_t get_da_status(uint32_t *out) {
  THD *thd = current_thd;
  if (thd == nullptr) return VEF_SESSION_UNAVAILABLE;
  SERVICE_TYPE(registry) *registry = mysql_plugin_registry_acquire();
  if (registry == nullptr) return VEF_SESSION_ERROR;

  vef_session_result_t result = VEF_SESSION_ERROR;
  {
    my_service<SERVICE_TYPE(mysql_thd_attributes)> attrs("mysql_thd_attributes",
                                                         registry);
    if (attrs.is_valid()) {
      uint16_t st = 0;
      if (attrs->get(thd, "da_status", &st)) {
        result = VEF_SESSION_NOT_FOUND;
      } else {
        *out = st;
        result = VEF_SESSION_OK;
      }
    }
  }
  mysql_plugin_registry_release(registry);
  return result;
}

vef_preview_session_t g_session_vtable{VEF_PREVIEW_SESSION_ABI_VERSION,
                                       get_query_text,
                                       get_query_digest,
                                       get_host_or_ip,
                                       get_query_charset,
                                       get_sql_command,
                                       get_command,
                                       get_da_status,
                                       get_login_user,
                                       get_external_user,
                                       get_proxy_user,
                                       get_active_roles};

}  // namespace

vef_preview_session_t *preview_session_vtable() { return &g_session_vtable; }

}  // namespace villagesql::services
