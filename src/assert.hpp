/* Copyright (C) 2015 INRA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FR_INRA_MITM_ASSERT_HPP
#define FR_INRA_MITM_ASSERT_HPP

#include <exception>

#ifdef MITM_ASSERT_THROWS
#include <stdexcept>
#endif

namespace mitm {

#ifndef MITM_ASSERT_THROWS

struct fail_fast : public std::runtime_error
{
    fail_fast()
        : std::runtime_error("")
    {}

    explicit
    fail_fast(char const* const message)
        : std::runtime_error(message)
    {}
};

inline void fast_assert(bool cond)
{
    if (!cond)
        throw fail_fast();
}

inline void fast_assert(bool cond, const char* const message)
{
    if (!cond)
        throw fail_fast(message);
}
#else
inline void fast_assert(bool cond)
{
    if (!cond)
        std::terminate();
}
inline void fast_assert(bool cond, const char* const)
{
    if (!cond)
        std::terminate();
}
#endif

#define Expects(x, y)  ::mitm::fast_assert((x), (y))
#define Ensures(x, y)  ::mitm::fast_assert((x), (y))

}

#endif
