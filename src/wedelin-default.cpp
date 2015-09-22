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
#include <Eigen/Core>
#include "internal.hpp"
#include "assert.hpp"
#include <iostream>
#include <limits>

namespace {

template <typename CBAR, typename A, typename C>
inline void
initialize_c_bar(CBAR &cbar, const A &a, const C &c,
                 mitm::index m, mitm::index n)
{
    for (mitm::index i = 0; i != m; ++i)
        for (mitm::index j = 0; j != n; ++j)
            cbar(i, j) = (a(i, j) != 0) ? c[j] : 0;
}

template <typename A, typename X, typename B>
inline bool
is_ax_equal_b(const A &a, const X &x, const B &b)
{
    Eigen::VectorXi ax = (a * x).transpose();

    for (mitm::index i = 0, endi = b.rows(); i != endi; ++i)
        if (ax(i) != b(i))
            return false;

    return true;
}

template <typename A, typename CBAR, typename R, typename B, typename X>
inline void
update_row(const A &a, CBAR &c_bar, R &r, const B &b, X &x,
           mitm::index n, mitm::index i)
{
    r.clear();

    for (mitm::index j = 0; j != n; ++j)
        if (a(i, j) != 0.)
            r.emplace_back(c_bar(i, j) / a(i, j), j);

    std::sort(r.begin(), r.end(),
              [](const std::pair<float, int> &lhs,
    const std::pair<float, int> &rhs) {
        return lhs.first < rhs.first;
    });


    Ensures(r.size() < std::numeric_limits<mitm::index>::max() &&
            b(i) + 1 < static_cast<mitm::index>(r.size()),
            "wedelin_default: update_row internal problem");

    float r_plus = r[r.size() - b(i)].first;
    float r_minus = r[r.size() - 1 - b(i)].first;
    float delta_y_i = (r_plus + r_minus) / 2.0;

    for (mitm::index j = 0; j != n; ++j)
        c_bar(i, j) -= a(i, j) * delta_y_i;

    for (mitm::index j = 0, endj = b(i); j != endj; ++j)
        x(r[j].second) = true;

    for (mitm::index j = b(i), endj =
             static_cast<mitm::index>(r.size()); j != endj; ++j)
        x(r[j].second) = false;
}

}

namespace mitm {

mitm::result
default_algorithm_default(const SimpleState &s, index limit)
{
    mitm::index m = static_cast<mitm::index>(s.b.size());
    mitm::index n = static_cast<mitm::index>(s.c.size());

    Eigen::MatrixXi A = Eigen::MatrixXi(s.b.size(), s.c.size());
    mitm::index longi = 0;

    for (mitm::index i = 0; i != m; ++i)
        for (mitm::index j = 0; j != n; ++j, ++longi)
            A(i, j) = s.a[longi];

    Eigen::ArrayXXf c_bar = Eigen::ArrayXXf(s.b.size(), s.c.size());
    std::vector <std::pair<float, int>> r(n);

    Eigen::VectorXi b = Eigen::VectorXi(m);
    for (index i = 0; i != m; ++i)
        b(i) = s.b[i];

    ::initialize_c_bar(c_bar, A, s.c, m, n);

    Eigen::VectorXi x(n);

    for (long int it = 0, end = limit; it != end; ++it) {
        x = Eigen::VectorXi::Zero(n);

        for (index i = 0; i != m; ++i)
            ::update_row(A, c_bar, r, b, x, n, i);

        if (::is_ax_equal_b(A, x, b))
            return mitm::result();
    }

    return mitm::result();
}

}
