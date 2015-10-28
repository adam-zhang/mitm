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

#ifndef FR_INRA_MITM_MATRIX_HPP
#define FR_INRA_MITM_MATRIX_HPP

#include <vector>
#include <cassert>

namespace mitm {

/** matrix is a simple container adaptator.
 *
 * To improve inline operation, compile with the NDEBUG symbol to remove
 * standard assertion.
 */
template <typename T, typename Container = std::vector<T>>
class matrix_adapter
{
public:
    using container_type = Container;
    using value_type = typename Container::value_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using size_type = typename Container::size_type;

private:
    container_type &m_c;
    std::size_t m_rows;
    std::size_t m_cols;

public:
    /** Construct the adaptator of the container @c.
     *
     * @param c [in] the linear container to adapt into a matrix.
     * @param rows [in] number of row.
     * @param cols [in] number of column.
     */
    matrix_adapter(container_type &c_, std::size_t rows, std::size_t columns);

    matrix_adapter() = delete;
    matrix_adapter(const matrix_adapter& q) = default;
    matrix_adapter(matrix_adapter&& q) = default;
    matrix_adapter& operator=(const matrix_adapter& q) = default;
    matrix_adapter& operator=(matrix_adapter&& q) = default;
    ~matrix_adapter() = default;

    void resize(std::size_t rows_, std::size_t cols_);

    size_type rows() const;
    size_type cols() const;

    bool empty() const;
    size_type size() const;
    reference operator()(std::size_t i, std::size_t j);
    const_reference operator()(std::size_t i, std::size_t j) const;
};

//
// implementation part
//

template <typename T, typename Container>
matrix_adapter<T, Container>::matrix_adapter(container_type &c_,
                             std::size_t rows_,
                             std::size_t columns_)
    : m_c(c_)
    , m_rows(rows_)
    , m_cols(columns_)
{
    assert(c_.size() == (rows_ * columns_));
}

template <typename T, typename Container>
void
matrix_adapter<T, Container>::resize(std::size_t rows_, std::size_t cols_)
{
    assert(m_c.size() == (rows_ * cols_));

    m_rows = rows_;
    m_cols = cols_;
}

template <typename T, typename Container>
typename matrix_adapter<T, Container>::size_type
matrix_adapter<T, Container>::rows() const
{
    return m_rows;
}

template <typename T, typename Container>
typename matrix_adapter<T, Container>::size_type
matrix_adapter<T, Container>::cols() const
{
    return m_cols;
}

template <typename T, typename Container>
bool
matrix_adapter<T, Container>::empty() const
{
    return m_c.empty();
}

template <typename T, typename Container>
typename matrix_adapter<T, Container>::size_type
matrix_adapter<T, Container>::size() const
{
    return m_c.size();
}

template <typename T, typename Container>
typename matrix_adapter<T, Container>::reference
matrix_adapter<T, Container>::operator()(std::size_t i, std::size_t j)
{
    assert(i < m_rows && j < m_cols);

    return m_c[i * m_cols + j];
}

template <typename T, typename Container>
typename matrix_adapter<T, Container>::const_reference
matrix_adapter<T, Container>::operator()(std::size_t i, std::size_t j) const
{
    assert(i < m_rows && j < m_cols);

    return m_c[i * m_cols + j];
}

} // namespace matrix_adapter

#endif
