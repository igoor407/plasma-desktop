#include "xcbatom.h"

#include <cstdlib>
#include <cstring>

XcbAtom::XcbAtom() : m_connection(0), m_reply(0), m_fetched(false)
{
}

XcbAtom::XcbAtom(xcb_connection_t *c, const char *name, bool onlyIfExists)
    : m_reply(0), m_fetched(false)
{
    intern(c, name, onlyIfExists);
}

void XcbAtom::intern(xcb_connection_t *c, const char *name, bool onlyIfExists)
{
    m_connection = c;
    m_cookie = xcb_intern_atom(c, onlyIfExists, std::strlen(name), name);
}

XcbAtom::~XcbAtom()
{
    std::free(m_reply);
}

xcb_atom_t XcbAtom::atom()
{
    if (!m_fetched) {
        m_fetched = true;
        m_reply = xcb_intern_atom_reply(m_connection, m_cookie, 0);
    }
    if (m_reply) {
        return m_reply->atom;
    } else {
        return 0;
    }
}
