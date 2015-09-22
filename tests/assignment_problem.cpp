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
#include <getopt.h>
#include "assert.hpp"

namespace {

/// A function to read template token from an input stream where new lines,
/// tabulations, chariots and spaces are forget.
///
/// \code
/// long int size;
/// if (next_token <long,int>(std::cin, size) == false)
///     std::cerr << "io failure\n";
/// \endcode
///
/// \return true on success, then \e t is updated, false otherwise.
///
template <typename Token>
bool next_token(std::istream &is, Token& t) noexcept
{
    char current;

    while (is.get(current)) {
        switch (current) {
        case '#':
            while (is && is.get() != '\n');
            break;

        case '\n':
        case ' ':
        case '\t':
        case '\r':
            break;

        case EOF:
            return false;

        default:
            is.unget();
            is >> t;
            return is.good();
        }
    }

    return false;
}

} // anonymous namespace

class AssignmentProblem
{
private:
    mitm::index m;     // constraints
    mitm::index n;     // variables
    mitm::SimpleState state;

public:
    AssignmentProblem()
        : m(0)
        , n(0)
    {}

    void init(int X)
    {
        m = 2 * X;
        n = X * X;
        state.init(m, n);

        std::fill(state.a.begin(), state.a.end(), false);

        for (mitm::index i = 0; i != X; ++i) {
            for (mitm::index j = 0; j != X; ++j) {
                state.a[i * n + i * X + j] = true;
                state.a[(m * n / 2) + (i * n) + i + (j * X)] = true;
            }
        }

        for (mitm::index i = 0; i != m; ++i)
            state.b[i] = 1;
    }

    void random_init(int X)
    {
        init(X);

        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<> dist(1, 20);

        for (int i = 0; i != n; ++i)
            state.c[i] = dist(mt);
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

    friend
    std::ostream &operator<<(std::ostream &os, const AssignmentProblem &pb)
    {
        return os << "\nconstraints: " << pb.m
               << "\nvariables: " << pb.n
               << "\ntask/resources: " << pb.m / 2;
    }

    friend
    std::istream &operator>>(std::istream &is, AssignmentProblem &pb)
    {
        mitm::index x;

        if (not ::next_token<mitm::index>(is, x) or x <= 0) {
            is.setstate(std::ios::failbit);
            return is;
        }

        pb.init(x);

        for (mitm::index i = 0; i != pb.n; ++i) {
            if (not ::next_token<float>(is, pb.state.c[i]))
                return is;

            if (pb.state.c[i] < 0.) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        return is;
    }
};

bool start_from_istream(std::istream& is, long int limit, float kappa,
                        float delta, float theta)
{
    AssignmentProblem pb;
    is >> pb;

    if (is.eof())
        return true;

    if (is.fail()) {
        std::cerr << "input stream error\n";
        return false;
    }

    return pb.run(limit, kappa, delta, theta);
}

bool start_from_random(int problem_size, long int limit, float kappa,
                       float delta, float theta)
{
    AssignmentProblem pb;
    pb.random_init(problem_size);
    return pb.run(limit, kappa, delta, theta);
}

int main(int argc, char **argv)
{
    long int limit = 5;
    float kappa = 0.01;
    float delta = 0.0001;
    float theta = 0.0001;
    int x = -1;  // number of task or resources.

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

    bool success = true;
    if (::optind < argc) {
        std::cout << "Read file\n";

        for (int i = ::optind; i < argc; ++i) {
            std::ifstream ifs(argv[i]);
            std::cout << "[" << argv[i] << "]\n";

            for (;;) {
                if (not start_from_istream(ifs, limit, kappa, delta, theta))
                    success = false;

                if (not ifs.good())
                    break;
            }
        }
    } else if (x < 0) {
        std::cout << "Read input stream\n";

        for (;;) {
            if (not start_from_istream(std::cin, limit, kappa, delta, theta))
                success = false;

            if (not std::cin.good()) {
                std::cout << "input stream close\n";
                break;
            }
        }
    } else {
        std::cout << "Generate " << x << '*' << x << " assignment problem\n";

        success = start_from_random(x, limit, kappa, delta, theta);
    }

    return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
