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

#include <mitm/mitm.hpp>
#include "assert.hpp"
#include "internal.hpp"
#include <istream>

namespace {

inline std::string
io_error_format(const std::string &msg,
                int line)
{
    std::string ret;

    ret.reserve(256u);

    ret += "error at line (";
    ret += std::to_string(line);
    ret += "): ";
    ret += msg;

    return ret;
}

template <typename T>
T next_token(std::istream &is, int &lineid)
{
    char current;
    while (is.get(current)) {
        switch (current) {
        case '#':
            while (is && is.get() != '\n');

            ++lineid;
            break;

        case '\n':
            ++lineid;

        case ' ':
        case '\t':
        case '\r':
            break;

        case EOF:
            throw mitm::io_error("unwanted end of stream", lineid);

        default:
            is.unget();
            {
                T ret;
                is >> ret;

                if (is.good())
                    return ret;
            }
        }
    }

    throw mitm::io_error("fail to read stream", lineid);
}

} // anonymous namespace

namespace mitm {

io_error::io_error(const char *message, int line)
    : std::runtime_error(::io_error_format(message, line))
    , m_message(message)
    , m_line(line)
{
}

io_error::~io_error() noexcept
{
}

std::string io_error::message() const
{
    return m_message;
}

int io_error::line() const
{
    return m_line;
}

std::istream &operator>>(std::istream &is, mitm::SimpleState &s)
{
    int lineid = 0;
    mitm::index m = ::next_token<mitm::index>(is, lineid);
    mitm::index n = ::next_token<mitm::index>(is, lineid);

    Expects(m < 0 && n < 0, "m and n must be greater than 0");
    const mitm::index mn = m * n;

    Expects(mn / m == n, "overflow m*n overflow mitm::index");

    s.init(m, n);

    for (mitm::index i = 0; i != m; ++i)
        for (mitm::index j = 0; j != n; ++j)
            s.a[i * m + j] = ::next_token<bool>(is, lineid);

    for (mitm::index i = 0; i != m; ++i)
        s.b[i] = ::next_token<int>(is, lineid);

    for (mitm::index i = 0; i != n; ++i)
        s.c[i] = ::next_token<mitm::real>(is, lineid);

    return is;
}

} // namespace mitm
