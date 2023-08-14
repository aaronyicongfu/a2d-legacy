#ifndef A2D_MAT_H
#define A2D_MAT_H

#include "a2denum.h"
#include "a2dobjs.h"

namespace A2D {

template <typename T, int M, int N>
class Mat {
 public:
  typedef T type;

  A2D_INLINE_FUNCTION Mat() {
    for (int i = 0; i < M * N; i++) {
      A[i] = 0.0;
    }
  }
  A2D_INLINE_FUNCTION Mat(const T* vals) {
    for (int i = 0; i < M * N; i++) {
      A[i] = vals[i];
    }
  }
  template <class MatType>
  A2D_INLINE_FUNCTION Mat(const MatType& mat) {
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < N; j++) {
        A[N * i + j] = mat(i, j);
      }
    }
  }
  A2D_INLINE_FUNCTION void zero() {
    for (int i = 0; i < M * N; i++) {
      A[i] = 0.0;
    }
  }
  template <class MatType>
  A2D_INLINE_FUNCTION void set(const MatType& mat) {
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < N; j++) {
        A[N * i + j] = mat(i, j);
      }
    }
  }
  template <class MatType>
  A2D_INLINE_FUNCTION void get(MatType& mat) {
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < N; j++) {
        mat(i, j) = A[N * i + j];
      }
    }
  }
  template <class IdxType1, class IdxType2>
  A2D_INLINE_FUNCTION T& operator()(const IdxType1 i, const IdxType2 j) {
    return A[N * i + j];
  }
  template <class IdxType1, class IdxType2>
  A2D_INLINE_FUNCTION const T& operator()(const IdxType1 i,
                                          const IdxType2 j) const {
    return A[N * i + j];
  }

  T* data() { return A; }

  T A[M * N];
};

template <class MatType>
class ADMat {
 public:
  A2D_INLINE_FUNCTION ADMat(MatType& A, MatType& Ab) : A(A), Ab(Ab) {}

  A2D_INLINE_FUNCTION MatType& value() { return A; }
  A2D_INLINE_FUNCTION const MatType& value() const { return A; }

  A2D_INLINE_FUNCTION MatType& bvalue() { return Ab; }
  A2D_INLINE_FUNCTION const MatType& bvalue() const { return Ab; }

  MatType& A;   // Matrix
  MatType& Ab;  // Reverse mode derivative value
};

template <class MatType>
class A2DMat {
 public:
  A2D_INLINE_FUNCTION A2DMat() {}
  A2D_INLINE_FUNCTION A2DMat(const MatType& A) : A(A) {}
  A2D_INLINE_FUNCTION A2DMat(const MatType& A, const MatType& Ab)
      : A(A), Ab(Ab) {}
  A2D_INLINE_FUNCTION A2DMat(const MatType& A, const MatType& Ab,
                             const MatType& Ap)
      : A(A), Ab(Ab), Ap(Ap) {}
  A2D_INLINE_FUNCTION A2DMat(const MatType& A, const MatType& Ab,
                             const MatType& Ap, const MatType& Ah)
      : A(A), Ab(Ab), Ap(Ap), Ah(Ah) {}

  A2D_INLINE_FUNCTION MatType& value() { return A; }
  A2D_INLINE_FUNCTION const MatType& value() const { return A; }

  A2D_INLINE_FUNCTION void set_bvalue(const MatType& val) { Ab.set(val); }
  A2D_INLINE_FUNCTION void get_bvalue(MatType& val) { Ab.get(val); }
  A2D_INLINE_FUNCTION MatType& bvalue() { return Ab; }
  A2D_INLINE_FUNCTION const MatType& bvalue() const { return Ab; }

  A2D_INLINE_FUNCTION void set_pvalue(const MatType& val) { Ap.set(val); }
  A2D_INLINE_FUNCTION void get_pvalue(MatType& val) { Ap.get(val); }
  A2D_INLINE_FUNCTION MatType& pvalue() { return Ap; }
  A2D_INLINE_FUNCTION const MatType& pvalue() const { return Ap; }

  A2D_INLINE_FUNCTION void set_hvalue(const MatType& val) { Ah.set(val); }
  A2D_INLINE_FUNCTION void get_hvalue(MatType& val) { Ah.get(val); }
  A2D_INLINE_FUNCTION MatType& hvalue() { return Ah; }
  A2D_INLINE_FUNCTION const MatType& hvalue() const { return Ah; }

  MatType A;   // Matrix
  MatType Ab;  // Reverse mode derivative value
  MatType Ap;  // Projected second derivative value
  MatType Ah;  // Reverse mode second derivative
};

/**
 * @brief Get data pointers from Mat/ADMat/A2DMat objects
 */
template <typename T, int m, int n>
A2D_INLINE_FUNCTION T* get_data(Mat<T, m, n>& mat) {
  return mat.A;
}

template <typename T, int m, int n>
A2D_INLINE_FUNCTION T* get_data(ADMat<Mat<T, m, n>>& mat) {
  return mat.A.A;
}

template <typename T, int m, int n>
A2D_INLINE_FUNCTION T* get_data(A2DMat<Mat<T, m, n>>& mat) {
  return mat.A.A;
}

/**
 * @brief Get pointer to seed data (bvalue, pvalue, hvalue) from
 * Mat/ADMat/A2DMat objects
 */
template <ADseed seed>
class GetSeed {
 public:
  template <typename T, int m, int n>
  static A2D_INLINE_FUNCTION T* get_data(ADMat<Mat<T, m, n>>& mat) {
    static_assert(seed == ADseed::b, "Incompatible seed type for ADMat");
    return mat.Ab.A;
  }

  template <typename T, int m, int n>
  static A2D_INLINE_FUNCTION T* get_data(A2DMat<Mat<T, m, n>>& mat) {
    static_assert(seed == ADseed::b or seed == ADseed::p or seed == ADseed::h,
                  "Incompatible seed type for A2DMat");
    if constexpr (seed == ADseed::b) {
      return mat.Ab.A;
    } else if constexpr (seed == ADseed::p) {
      return mat.Ap.A;
    } else {  // seed == ADseed::h
      return mat.Ah.A;
    }
  }
};

/**
 * @brief Select type based on whether the matrix is passive or active (can be
 * differentiated)
 *
 * For example, the following types are equivalent:
 *
 * Mat<...>    == ADMatType<ADiffType::PASSIVE, *,               Mat<...>>;
 * ADMat<...>  == ADMatType<ADiffType::ACTIVE,  ADorder::FIRST,  Mat<...>>;
 * A2DMat<...> == ADMatType<ADiffType::ACTIVE,  ADorder::SECOND, Mat<...>>;
 *
 * @tparam adiff_type passive or active
 * @tparam order first (AD) or second (A2D)
 * @tparam MatType the numeric type of the matrix
 */
template <ADiffType adiff_type, ADorder order, class MatType>
using ADMatType = typename std::conditional<
    adiff_type == ADiffType::ACTIVE,
    typename std::conditional<order == ADorder::FIRST, ADMat<MatType>,
                              A2DMat<MatType>>::type,
    MatType>::type;
}  // namespace A2D

#endif  // A2D_MAT_H