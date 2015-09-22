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
#include <fstream>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <climits>
#include <cmath>
#include <getopt.h>

namespace {

void
help_show() noexcept
{
    std::cout << "mitm [options...]\n"
              << "-m method    h_classic, h_gpgpu\n"
              << "-l limit     number of loop\n"
              << "-k kappa     kappa init value [0..1[ (float)\n"
              << "-d delta     delta value [0..+oo[ (float)\n"
              << "-t theta     theta value [0..1] (float)\n"
              << std::endl;
}

}

int
main(int argc, char *argv[])
{
    std::string option_method;
    long int option_limit = 100;
    float kappa = 0.001;
    float delta = 0.0001;
    float theta = 0.001;
    int option;
    char *c;

    while ((option = ::getopt(argc, argv, "l:k:d:t:m:h")) != -1) {
        switch (option) {
        case 'l':
            errno = 0;
            option_limit = std::strtol(::optarg, &c, 10);

            if ((errno == ERANGE && (option_limit == LONG_MAX
                                     || option_limit == LONG_MIN))
                || (errno != 0 && option_limit == 0)
                || (c == ::optarg)
                || (option_limit < 0)) {
                std::cerr << "fail to convert parameter `"
                          << ::optarg << " for parameter l\n";
                exit(EXIT_FAILURE);
            }

        case 'k':
            errno = 0;
            kappa = std::strtof(::optarg, &c);

            if ((errno == ERANGE && (kappa == HUGE_VALF ||
                                     kappa == -HUGE_VALF))
                || (errno != 0 && kappa == 0)
                || (c == ::optarg)
                || (kappa < 0)
                || (kappa > 1)) {
                std::cerr << "fail to convert parameter `"
                          << ::optarg << " for parameter kappa (or k)\n";
                exit(EXIT_FAILURE);
            }

        case 'd':
            errno = 0;
            delta = std::strtof(::optarg, &c);

            if ((errno == ERANGE && (delta == HUGE_VALF ||
                                     delta == -HUGE_VALF))
                || (errno != 0 && delta == 0)
                || (c == ::optarg)
                || (delta < 0)) {
                std::cerr << "fail to convert parameter `"
                          << ::optarg << " for parameter delta (or l)\n";
                exit(EXIT_FAILURE);
            }

        case 't':
            errno = 0;
            theta = std::strtof(::optarg, &c);

            if ((errno == ERANGE && (delta == HUGE_VALF ||
                                     delta == -HUGE_VALF))
                || (errno != 0 && delta == 0)
                || (c == ::optarg)
                || (theta < 0 || theta > 1)) {
                std::cerr << "fail to convert parameter `"
                          << ::optarg << " for parameter theta\n";
                exit(EXIT_FAILURE);
            }

        case 'm':
            option_method = ::optarg;
            break;

        case 'h':
            ::help_show();
            return EXIT_SUCCESS;
        }
    }

    for (int i = ::optind; i < argc; ++i) {
        std::ifstream ifs(argv[i]);

        if (not ifs.is_open()) {
            std::cerr << "fail to open '" << argv[i] << "'\n";
            continue;
        }

        try {
            mitm::SimpleState state;
            ifs >> state;

            if (option_method == "default") {
                mitm::default_algorithm(state, option_limit, std::string{});
            } else {
                mitm::heuristic_algorithm(state, option_limit, delta,
                                          kappa, theta, std::string{});
            }
        } catch (const std::exception &e) {
            std::cerr << "/!\\ fail: " << e.what() << '\n';
        }
    }

    return EXIT_SUCCESS;
}
