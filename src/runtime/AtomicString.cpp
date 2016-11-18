#include "Escargot.h"
#include "AtomicString.h"
#include "Context.h"

namespace Escargot {

AtomicString::AtomicString(ExecutionState& ec, const char16_t* src, size_t len)
{
    init(&ec.context()->m_atomicStringMap, src, len);
}

AtomicString::AtomicString(ExecutionState& ec, const char* src, size_t len)
{
    init(&ec.context()->m_atomicStringMap, src, len);
}

void AtomicString::init(AtomicStringMap* ec, const char* src, size_t len)
{
    auto iter = ec->find(std::make_pair(src, len));
    if (iter == ec->end()) {
        ASCIIStringData s(src, &src[len]);
        String* newData = new ASCIIString(std::move(s));
        m_string = newData;
        ec->insert(std::make_pair(std::make_pair(newData->asASCIIString()->data(), len), newData));
    } else {
        m_string = iter->second;
    }
}

void AtomicString::init(AtomicStringMap* ec, const char16_t* src, size_t len)
{
    if (isAllASCII(src, len)) {
        char* abuf = ALLOCA(len, char, ec);
        for (unsigned i = 0 ; i < len ; i ++) {
            abuf[i] = src[i];
        }
        init(ec, abuf, len);
        return;
    }
    UTF8StringData buf = utf16StringToUTF8String(src, len);
    auto iter = ec->find(std::make_pair(buf.data(), buf.size()));
    if (iter == ec->end()) {
        UTF16StringData s(src, &src[len]);
        String* newData = new UTF16String(std::move(s));
        m_string = newData;
        char* ptr = gc_malloc_atomic_ignore_off_page_allocator<char>().allocate(buf.size());
        memcpy(ptr, buf.data(), buf.size());
        ec->insert(std::make_pair(std::make_pair(ptr, buf.size()), newData));
    } else {
        m_string = iter->second;
    }
}

}