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
#include "cstream.hpp"
#include "internal.hpp"

namespace mitm {

#ifndef MITM_HAVE_CUDA
mitm::result
heuristic_algorithm_gpgu(const SimpleState&s, index limit, float kappa,
                         float delta, float theta)
{
    (void)s;
    (void)limit;
    (void)kappa;
    (void)delta;
    (void)theta;

    out() << "heuristic_algorithm_gpgu is unavailable. "
        "Install cuda package and rerun CMake\n";

    return mitm::result{};
}
#endif

mitm::result
default_algorithm(const SimpleState &s, index limit,
                  const std::string &impl)
{
    if (not impl.empty()) {
        out().printf("%s is undefined for the default algorithm. Back to"
                     " classic implementation\n", impl.c_str());
    }

    out().printf("default_algorithm using the `%s' implementation\n",
                 impl.c_str());

    return default_algorithm_default(s, limit);
}

mitm::result
heuristic_algorithm(const SimpleState &s, index limit,
                    float kappa, float delta, float theta,
                    const std::string &impl)
{
    cstream cs(-27);
    cs << "hello\n";

    out().printf("heuristic_algorithm using the `%s' implementation\n",
                 impl.c_str());

    if (impl == "gpgpu")
        return heuristic_algorithm_gpgu(s, limit, kappa, delta, theta);

    return heuristic_algorithm_default(s, limit, kappa, delta, theta);
}

}
