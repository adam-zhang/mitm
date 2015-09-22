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
#include <iostream>
#include <fstream>
#include <random>
#include <cassert>
#include <getopt.h>

namespace {

class NQueenProblem
{
public:
    mitm::index m;    // constraints
    mitm::index n;    // variables
    mitm::NegativeCoefficient state;

    NQueenProblem(long int problem_size)
        : m(problem_size + problem_size + 2 * (problem_size - 2))
        , n(problem_size * problem_size)
    {
        if (problem_size < 3)
            throw std::runtime_error("problem size too small");

        state.init(m, n);

        std::fill(state.a.begin(), state.a.end(), false);

        // one constraint is valid per line and per column.
        for (mitm::index i = 0; i != problem_size; ++i) {
            for (mitm::index j = 0; j != problem_size; ++j) {
                state.a[i * n + i * problem_size + j] = true;
                state.a[(m * n / 2) + (i * n) + i + (j * problem_size)] = true;
            }
        }

        //for (mitm::index i = 0, ie = 2 * problem_size; i != ie; ++i) {
            //state.b.lower_bound = 1;
            //state.b.upper_bound= 1;
        //}

        //// one constraint may be valid per diagonal.
        //for (mitm::index i = 2 * problem_size, ie = n; i != ie; ++i) {
            //state.b.lower_bound = 0;
            //state.b.upper_bound= 1;
        //}
    }

    bool run(mitm::index limit, float kappa, float delta, float theta)
    {
        try {
            mitm::result r(mitm::heuristic_algorithm(state, limit, kappa, delta,
                                                     theta, std::string{}));

            std::cout << "solution founded in " << r.loop << " loops !\n";
            for (mitm::index i = 0; i != n; ++i) {
                std::cout << r.x[i] << ' ';
                if ((i + 1) % (m / 2) == 0)
                    std::cout << '\n';
            }
            std::cout << '\n';

            for (mitm::index i = 0; i != n; ++i) {
                std::cout << state.c[i] << ' ';
                if ((i + 1) % (m / 2) == 0)
                    std::cout << '\n';
            }
            std::cout << '\n';
            return true;
        } catch (const std::exception& e) {
            std::cerr << "mitm error:" << e.what() << '\n';
            return false;
        }
    }
};

} // anonymous namespace

int main(int argc, char **argv)
{
    long int limit = 5;
    float kappa = 0.01;
    float delta = 0.0001;
    float theta = 0.0001;
    int x = 4;

    try {
        int option;

        while ((option = ::getopt(argc, argv, "l:k:d:t:x:")) != -1) {
            switch (option) {
            case 'l':
                limit = std::stoi(::optarg);
                break;

            case 'k':
                kappa = std::stof(::optarg);
                break;

            case 'd':
                delta = std::stof(::optarg);
                break;

            case 't':
                theta = std::stof(::optarg);
                break;

            case 'x':
                x = std::stoi(::optarg);
                break;

            case 'h':
                std::cout << "usage:\n"
                          << "\tassignment_problem [Options...] [Files...]\n"
                          << "\t[-l\t\nLimit ]0..+oo[\n"
                          << "\t[-k\t\tKappa parameter [0..1[\n"
                          << "\t[-d\t\tDelta parameter [0..+oo[\n"
                          << "\t[-t\t\tTheta parameter [0..1]\n"
                          << "\t[-x\t\tnumber of resources]\n"
                          << "\t[files]\t\tlist of input files\n";
                return EXIT_SUCCESS;
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "argument error\n";
        return EXIT_FAILURE;
    }

    std::cout << "limit: " << limit << " kappa: "<< kappa << " delta: "
        << delta << " theta: " << theta << '\n';

    ::NQueenProblem pb(x);

    return (pb.run(limit, kappa, delta, theta)) ? EXIT_SUCCESS : EXIT_FAILURE;
}

