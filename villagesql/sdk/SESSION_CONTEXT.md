# Session Context for Extensions

Read-only access to the session executing a VDF, in two tiers. Both are
read-only; session mutation is out of scope.

## Tier 1 — `vsql::Session` (hot path, no opt-in)

Declare a `vsql::Session` as the first parameter of a VDF. The SDK populates it
from the call context (available when `protocol >= VEF_PROTOCOL_4`).

```cpp
void who(vsql::Session s, StringResult out) {
  if (!s.available()) { out.error("no session"); return; }  // fail closed
  std::string user(s.priv_user());   // copy-before-retain
  out.set(user);
}
```

Fields: `schema()`, `connection_id()`, `priv_user()`, `priv_host()`,
`kill_status()`. Strings are valid only for the duration of the call.

## Tier 2 — `vsql::preview_session::SessionCapability` (opt-in)

For expensive/variable values, declare a `SessionCapability` and pass it to
`.with()`. Requires `vsql_allow_preview_extensions = ON`.

```cpp
static vsql::preview_session::SessionCapability g_session;

void audit_line(Session s, StringResult out) {
  if (!s.available()) { out.error("session context unavailable"); return; }
  std::string line(s.priv_user());
  line.append(": ");
  auto qt = g_session.query_text();
  if (qt.status == SessionCapability::Status::OK) {
    line.append(qt.value);
  } else {
    line.append("<unknown>");
  }
  out.set(line);
}

// In the extension entry point:
//   .with(g_session)
```

String getters (`query_text`, `query_digest`, `host_or_ip`, `query_charset`,
`sql_command`, `command`, `login_user`, `external_user`, `proxy_user`,
`active_roles`) return `StringResult{Status, std::string value}`.
`da_status()` returns `U32Result{Status, uint32_t value}`.

## Security rules

- `schema` is the connection default database, not the schema of each
  referenced object.
- `priv_user` is the active privilege identity; inside a DEFINER/SUID context
  it can differ from the login user.
- `login_user`, `proxy_user`, and `external_user` are distinct concepts — do
  not collapse them.
- `connection_id` is reused after disconnect; it is not a stable tenant or
  user key.
- Copy string fields before retaining them beyond the callback.
- Enforcement extensions must fail closed when required fields are unavailable.
- Session context is read-only.
