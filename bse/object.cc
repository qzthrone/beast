// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include "object.hh"
#include "internal.hh"

namespace Bse {

ObjectImpl::ObjectImpl ()
{}

ObjectImpl::~ObjectImpl ()
{}

bool
ObjectImpl::set_prop (const std::string &name, const Any &value)
{
  if (!name.empty())
    {
      auto collector = [&value] (const Aida::PropertyAccessor &ps) {
        ps.set (value);
        return true;
      };
      return __access__ (name, collector);
    }
  return false;
}

Any
ObjectImpl::get_prop (const std::string &name)
{
  Any any;
  if (!name.empty())
    {
      auto collector = [&any] (const Aida::PropertyAccessor &ps) {
        ps.get().swap (any);
        return true;
      };
      __access__ (name, collector);
    }
  return any;
}

StringSeq
ObjectImpl::find_prop (const std::string &name)
{
  StringSeq kvinfo;
  if (!name.empty())
    {
      auto collector = [&kvinfo] (const Aida::PropertyAccessor &ps) {
        ps.typedata().swap (kvinfo);
        return false;
      };
      __access__ (name, collector);
    }
  return kvinfo;
}

StringSeq
ObjectImpl::list_props ()
{
  StringSeq props;
  auto collector = [&props] (const Aida::PropertyAccessor &ps) {
    props.push_back (ps.name());
    return false;
  };
  __access__ ("", collector);
  return props;
}

void
ObjectImpl::emit_event (const std::string &type, const KV &a1, const KV &a2, const KV &a3,
                        const KV &a4, const KV &a5, const KV &a6, const KV &a7)
{
  const char ident_chars[] =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char *const ctype = type.c_str(), *const colon = strchr (ctype, ':');
  const String name = colon ? type.substr (0, colon - ctype) : type;
  const String detail = colon ? type.substr (colon - ctype + 1) : "";
  for (size_t i = 0; name[i]; i++)
    if (!strchr (ident_chars, name[i]))
      {
        warning ("invalid characters in Event type: %s", type);
        break;
      }
  for (size_t i = 0; detail[i]; i++)
    if (!strchr (ident_chars, detail[i]) and detail[i] != '_')
      {
        warning ("invalid characters in Event type: %s", type);
        break;
      }
  Aida::Event ev (type);
  const KV *args[] = { &a1, &a2, &a3, &a4, &a5, &a6, &a7 };
  for (size_t i = 0; i < sizeof (args) / sizeof (args[0]); i++)
    if (!args[i]->key.empty())
      ev[args[i]->key] = args[i]->value;
  ev["name"] = name;
  ev["detail"] = detail;
  event_dispatcher_.emit (ev);  // emits "notify:detail" as type="notify:detail" name="notify" detail="detail"
  // using namespace Aida::KeyValueArgs; emit_event ("notification", "value"_v = 5);
}

void
ObjectImpl::notify (const String &detail)
{
  assert_return (detail.empty() == false);
  emit_event ("notify:" + detail);
}

} // Bse
