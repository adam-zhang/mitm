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

#ifndef FR_INRA_MITM_LOG_HPP
#define FR_INRA_MITM_LOG_HPP

#include <ostream>
#include <cstdio>
#include <unistd.h>

namespace mitm {

enum color_code {
    FG_RED      = 31,
    FG_GREEN    = 32,
    FG_BLUE     = 34,
    FG_DEFAULT  = 39,
    BG_RED      = 41,
    BG_GREEN    = 42,
    BG_BLUE     = 44,
    BG_DEFAULT  = 49
};

struct color_modifier
{
    color_code code;

    constexpr
    color_modifier(color_code code_)
        : code(code_)
    {}

    friend std::ostream&
        operator<<(std::ostream& os, const color_modifier& mod)
        {
            if ((&os == &std::cout && ::isatty(::fileno(stdout)))
                || (&os == &std::cerr && ::isatty(::fileno(stderr)))
                || (&os == &std::clog && ::isatty(::fileno(stderr))))
                return os << "\033[" << static_cast<int>(mod.code) << "m";

            return os;
        }
};

struct color_message
{
    const char *msg;
    color_modifier modifier;

    constexpr
        color_message(const char *msg_, const color_modifier &modifier_)
        : msg(msg_)
        , modifier(modifier_)
    {}

    friend std::ostream&
        operator<<(std::ostream& os, const color_message& cm)
        {
            return os << cm.modifier << cm.msg << color_modifier(FG_DEFAULT)
                << color_modifier(BG_DEFAULT);
        }
};

}

#endif
