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

#ifndef FR_INRA_MITM_MITM_HPP
#define FR_INRA_MITM_MITM_HPP

#if defined _WIN32 || defined __CYGWIN__
#define MITM_HELPER_DLL_IMPORT __declspec(dllimport)
#define MITM_HELPER_DLL_EXPORT __declspec(dllexport)
#define MITM_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define MITM_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
#define MITM_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
#define MITM_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define MITM_HELPER_DLL_IMPORT
#define MITM_HELPER_DLL_EXPORT
#define MITM_HELPER_DLL_LOCAL
#endif
#endif

#ifdef MITM_DLL
#ifdef libmitm_EXPORTS
#define MITM_API MITM_HELPER_DLL_EXPORT
#else
#define MITM_API MITM_HELPER_DLL_IMPORT
#endif
#define MITM_LOCAL MITM_HELPER_DLL_LOCAL
#define MITM_MODULE MITM_HELPER_DLL_EXPORT
#else
#define MITM_API
#define MITM_LOCAL
#define MITM_MODULE MITM_HELPER_DLL_EXPORT
#endif

#include <stdexcept>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace mitm {

class MITM_API io_error : public std::runtime_error
{
public:
    io_error(const char *message, int line);
    virtual ~io_error() throw();

    std::string message() const;
    int line() const;

private:
    std::string m_message;
    int m_line;
};

#ifdef MITM_REAL_TYPE
typedef MITM_REAL_TYPE real
#else
typedef float real;
#endif

typedef std::ptrdiff_t index;
typedef std::vector<bool> A_type;
typedef std::vector<int> b_type;
typedef std::vector<real> c_type;
typedef std::vector<int> x_type;

class MITM_API SimpleState
{
public:
    /** Try to initialize the constraints matrix @e A, equalities and costs
     * vectors @b and @c.
     *
     * return 0 if success otherwise -EDOM if m and n are bad or -ENOMEM if
     * not enough memory.
     */
    int init(index m, index n) noexcept;

    index constraints() const noexcept;

    index variables() const noexcept;

    std::vector<bool> a;
    std::vector<int> b;
    std::vector<real> c;
};

class MITM_API NegativeCoefficient
{
public:
    struct b_bounds {
        real lower_bound;
        real upper_bound;
    };

    void init(index m, index n)
    {
        try {
            a.resize(m * n);
            b.resize(m);
            c.resize(n);
        } catch(const std::bad_alloc& e) {
            std::vector<int>().swap(a);
            std::vector<b_bounds>().swap(b);
            std::vector<real>().swap(c);

            throw std::runtime_error("not enough memory");
        }
    }

    /// Constraints matrix allows -1, 0 or +1 values.
    std::vector<int> a;

    /// The lower bound, value and upper bound for the equality or inequality
    /// constraint.
    std::vector<b_bounds> b;

    /// The cost vector.
    std::vector<real> c;
};

struct result
{
    /// The solution vector.
    std::vector<bool> x;

    /// Number of loop necessary.
    index loop;
};

MITM_API std::istream &operator>>(std::istream &is, SimpleState &s);

MITM_API result
heuristic_algorithm(const SimpleState &s, index limit,
                    real kappa, real delta, real theta,
                    const std::string &impl);

MITM_API result
heuristic_algorithm(const NegativeCoefficient& s, index limit,
                    real kappa, real delta, real theta,
                    const std::string &impl);


inline int
SimpleState::init(index m, index n) noexcept
{
    if (m <= 0 or n <= 0)
        return -EDOM;

    try {
        a.resize(m * n);
        b.resize(m);
        c.resize(n);
    } catch(const std::bad_alloc& e) {
        std::vector<bool>().swap(a);
        std::vector<int>().swap(b);
        std::vector<real>().swap(c);
        return -ENOMEM;
    }

    return 0;
}

inline index
SimpleState::constraints() const noexcept
{
    return static_cast<index>(b.size());
}

inline index
SimpleState::variables() const noexcept
{
    return static_cast<index>(c.size());
}

}

#endif
