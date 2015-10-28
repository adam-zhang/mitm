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
#include <iterator>
#include "cstream.hpp"
#include "internal.hpp"
#include "assert.hpp"

namespace mitm {
namespace negative {
struct constraint
{
    std::vector<mitm::index> I;
    std::vector<mitm::index> C;
    std::vector<std::tuple<float, mitm::index>> r;
    mitm::index k;
    mitm::index n;
    float bk_lower_bound;
    float bk_upper_bound;

    constraint() = default;

    constraint(mitm::index k_, mitm::index n_,
               float bk_lower_bound_, float bk_upper_bound_,
               const Eigen::MatrixXi& a)
        : k(k_)
        , bk_lower_bound(bk_lower_bound_)
        , bk_upper_bound(bk_upper_bound_)
    {
        for (mitm::index i = 0; i != n_; ++i) {
            if (a(k, i) != 0) {
                I.emplace_back(i);

                if (a(k, i) < 0)           // Find variables with negative
                    C.emplace_back(i);     // coefficient.

                r.emplace_back(0, i);
            }
        }
    }

    void update(Eigen::MatrixXi& A, const Eigen::RowVectorXf& c,
                Eigen::MatrixXf& P, Eigen::VectorXf& pi, Eigen::VectorXi& x,
                float kappa, float l, float theta)
    {
        P.row(k) *= theta;

        for (mitm::index i = 0; i != static_cast<mitm::index>(I.size()); ++i) {
            float sum_a_hi_pi_h = 0;
            float sum_a_hi_p_hi = 0;
            for (mitm::index h = 0, endh = A.rows(); h != endh; ++h) {
                if (A(h, i)) {
                    sum_a_hi_pi_h += A(h, I[i]) * pi(h);
                    sum_a_hi_p_hi += A(h, I[i]) * P(h, I[i]);
                }
            }

            r[i] = std::make_tuple(c(I[i]) - sum_a_hi_pi_h - sum_a_hi_p_hi,
                                   I[i]);
        }

        auto bk_lower_bound_tmp = bk_lower_bound;
        auto bk_upper_bound_tmp = bk_upper_bound;

        if (not C.empty()) {
            // Find variable with negative coefficient and negate reduced
            // costs and coefficients of these variables.
            for (mitm::index i : C) {
                std::get<0>(r[i]) = -std::get<0>(r[i]);
                A(k, i) = -A(k, i);
                P(k, i) = -P(k, i);
            }

            // TODO u(i) = 1 now but we need to update the state structure
            // to insert a u(i) to handle general bounded integer variable
            // (see. 3.1 Bastert).
            float sum = 0;
            for (mitm::index i : C)
                sum += A(k, i) * (1);

            bk_lower_bound_tmp += sum;
            bk_upper_bound_tmp += sum;
        }

        //
        // End of update
        //

        std::sort(r.begin(), r.end(),
                  [](const std::tuple<float, mitm::index>& lhs,
                     const std::tuple<float, mitm::index>& rhs)
                  {
                      return std::get<0>(lhs) < std::get<0>(rhs);
                  });

        std::vector<std::tuple<float, mitm::index>> computer;
        std::vector<std::tuple<float, mitm::index>> no_computer;

        for (auto& sr : r) {
            if (std::get<0>(sr) >= bk_lower_bound_tmp and
                std::get<0>(sr) <= bk_upper_bound_tmp)
                computer.emplace_back(sr);
            else
                no_computer.emplace_back(sr);
        }

        assert(computer.size() >= 2);

        const auto& max_1 = computer[computer.size() - 1];
        const auto& max_2 = computer[computer.size() - 2];

        pi(k) += (std::get<0>(max_1) + std::get<0>(max_2)) / 2.0;

        const float delta = ((kappa / (1 - kappa)) * (
                                 std::get<0>(max_1) -
                                 std::get<0>(max_2))) + l;

        for (const auto& sr : computer) {
            x(std::get<1>(sr)) = 1;
            P(k, std::get<1>(sr)) -= +delta;
        }

        for (const auto& sr : no_computer) {
            x(std::get<1>(sr)) = 0;
            P(k, std::get<1>(sr)) -= -delta;
        }

        // clean up: correct negated costs and adjust value of negated
        // variables.
        for (mitm::index i : C) {
            A(k, i) = -A(k, i);
            P(k, i) = -P(k, i);

            // TODO u(i) = 1 now but we need to update the state structure
            // to insert a u(i) to handle general bounded integer variable
            // (see. 3.1 Bastert).
            x(i) = (1) - x(i);
        }
    }

    friend std::ostream&
        operator<<(std::ostream& os, const constraint& c)
        {
            os << "k: " << c.k << " n: " << c.n << " bk: " << c.bk_lower_bound
               << ' ' << c.bk_upper_bound << '\n';
            os << "I: ";
            std::copy(c.I.cbegin(), c.I.cend(),
                      std::ostream_iterator<mitm::index>(os, " "));

            os << "\nr: ";
            for (const auto& t : c.r)
                os << '[' << std::get<0>(t) << ',' << std::get<1>(t) << "] ";

            return os << '\n';
        }
};

struct wedelin_heuristic_with_negative_coeff
{
    std::vector <constraint> constraints;
    Eigen::MatrixXi A;
    std::vector <NegativeCoefficient::b_bounds> b;
    Eigen::RowVectorXf c;
    Eigen::VectorXi x;
    Eigen::MatrixXf P;
    Eigen::VectorXf pi;
    index m;
    index n;
    float kappa;
    float l;
    float theta;

    wedelin_heuristic_with_negative_coeff(const NegativeCoefficient &s,
                                          mitm::index m_, mitm::index n_,
                                          float k_, float l_, float theta_)
        : constraints(m_)
        , A(Eigen::MatrixXi::Zero(m_, n_))
        , b(s.b)
        , c(Eigen::RowVectorXf::Zero(n_))
        , x(Eigen::VectorXi::Zero(n_))
        , P(Eigen::MatrixXf::Zero(m_, n_))
        , pi(Eigen::VectorXf::Zero(m_))
        , m(m_)
        , n(n_)
        , kappa(k_)
        , l(l_)
        , theta(theta_)
    {
        {
            mitm::index longi = 0;
            for (mitm::index i = 0; i != m; ++i)
                for (mitm::index j = 0; j != n; ++j, ++longi)
                    A(i, j) = s.a[longi];
        }

        for (mitm::index j = 0; j != n; ++j) {
            c(j) = s.c[j];
            x(j) = c(j) <= 0;
        }

        // TODO: intialize parameters delta, kappa.
        Ensures(kappa >= 0 && kappa < 1, "kappa must be [0..1[");
        Ensures(l >= 0, "l must be [0..+oo[");
        Ensures(theta >= 0 && theta <= 1, "theta must be [0..1]");

        constraints.clear();
        for (mitm::index i = 0; i != m; ++i)
            constraints.emplace_back(i, n,
                                     b[i].lower_bound, b[i].upper_bound,
                                     A);
    }

    inline bool
    is_constraint_need_update(mitm::index k) const
    {
        // TODO: Found a Eigen API (sum(A.row(k) * x(k)) == b(k).
        int sum = 0;
        for (mitm::index i = 0; i != n; ++i)
            sum += A(k, i) * x(i);

        return b[k].lower_bound <= sum and sum <= b[k].upper_bound;
    }

    bool next()
    {
        for (mitm::index k = 0; k != m; ++k)
            if (is_constraint_need_update(k))
                constraints[k].update(A, c, P, pi, x, kappa, l, theta);

        // TODO: adjust parameters kappa, delta, theta

        return false;
    }
};

}

mitm::result
heuristic_algorithm_default(const NegativeCoefficient& s, index limit,
                            float kappa, float delta, float theta,
                            const std::string &impl)
{
    (void)impl;

    Expects(s.b.size() > 0 && s.c.size() > 0 &&
            s.a.size() == s.b.size() * s.c.size(),
            "heuristic_algorithm_default: state not initialized");

    mitm::negative::wedelin_heuristic_with_negative_coeff wh(
        s,
        static_cast<mitm::index>(s.b.size()),
        static_cast<mitm::index>(s.c.size()),
        kappa, delta, theta);

    for (long int it = 0; it != limit; ++it) {
        if (wh.next()) {
            mitm::result ret;

            ret.x.resize(wh.x.size());

            for (mitm::index j = 0;
                 j != static_cast<mitm::index>(s.c.size()); ++j)
                ret.x[j] = wh.x(j);

            ret.loop = it;
            return ret;
        }
    }

    throw std::runtime_error("no solution founded");
}


}
