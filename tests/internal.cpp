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

#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "matrix.hpp"
#include "io.hpp"

TEST_CASE("Matrix test", "[matrix]")
{
    std::vector <bool> a(5 * 2, false);
    mitm::matrix_adapter <bool, std::vector<bool>> adapt(a, 5, 2);

    for (std::size_t i = 0, ei = adapt.rows(); i != ei; ++i)
        for (std::size_t j = 0, ej = adapt.cols(); j != ej; ++j)
            adapt(i, j) = true;

    auto it = std::find(a.cbegin(), a.cend(), false);
    REQUIRE(it == a.cend());

    mitm::out() << adapt << "\n";

    adapt(0, 0) = false;
    adapt(1, 1) = false;

    REQUIRE(a[0] == false);
    REQUIRE(a[1 * 2 + 1] == false);
}
