#ifndef A2D_GEMM_H
#define A2D_GEMM_H

#include <type_traits>

#include "a2denum.h"
#include "a2dmat.h"
#include "a2dobjs.h"
#include "ad/core/a2dgemmcore.h"

namespace A2D {

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL>
A2D_INLINE_FUNCTION void MatMatMult(Mat<T, N, M>& A, Mat<T, K, L>& B,
                                    Mat<T, P, Q>& C) {
  MatMatMultCore<T, N, M, K, L, P, Q, opA, opB>(get_data(A), get_data(B),
                                                get_data(C));
}

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL,
          ADiffType adA = ADiffType::ACTIVE, ADiffType adB = ADiffType::ACTIVE>
class ADMatMatMultExpr {
 private:
  static constexpr MatOp not_opA = negate_op<opA>::value;
  static constexpr MatOp not_opB = negate_op<opB>::value;

  using Atype = ADMatType<adA, Mat<T, N, M>>;
  using Btype = ADMatType<adB, Mat<T, K, L>>;
  using Ctype = ADMat<Mat<T, P, Q>>;

 public:
  A2D_INLINE_FUNCTION ADMatMatMultExpr(Atype& A, Btype& B, Ctype& C)
      : A(A), B(B), C(C) {
    MatMatMultCore<T, N, M, K, L, P, Q, opA, opB>(get_data(A), get_data(B),
                                                  get_data(C));
  }

  A2D_INLINE_FUNCTION void forward() {
    if constexpr (adA == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, opA, opB, MatOp::NORMAL, false>(
          get_bdata(A), get_data(B), get_bdata(C));
    }
    if constexpr (adB == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, opA, opB, MatOp::NORMAL,
                     adA == ADiffType::ACTIVE>(get_data(A), get_bdata(B),
                                               get_bdata(C));
    }
  }

  A2D_INLINE_FUNCTION void reverse() {
    if constexpr (adA == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, MatOp::NORMAL, not_opB, opA, true>(
          get_bdata(C), get_data(B), get_bdata(A));
    }
    if constexpr (adB == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, not_opA, MatOp::NORMAL, opB, true>(
          get_data(A), get_bdata(C), get_bdata(B));
    }
  }

 private:
  Atype& A;
  Btype& B;
  Ctype& C;
};

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL>
A2D_INLINE_FUNCTION auto MatMatMult(ADMat<Mat<T, N, M>>& A,
                                    ADMat<Mat<T, K, L>>& B,
                                    ADMat<Mat<T, P, Q>>& C) {
  return ADMatMatMultExpr<T, N, M, K, L, P, Q, opA, opB, ADiffType::ACTIVE,
                          ADiffType::ACTIVE>(A, B, C);
}

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL>
A2D_INLINE_FUNCTION auto MatMatMult(Mat<T, N, M>& A, ADMat<Mat<T, K, L>>& B,
                                    ADMat<Mat<T, P, Q>>& C) {
  return ADMatMatMultExpr<T, N, M, K, L, P, Q, opA, opB, ADiffType::PASSIVE,
                          ADiffType::ACTIVE>(A, B, C);
}

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL>
A2D_INLINE_FUNCTION auto MatMatMult(ADMat<Mat<T, N, M>>& A, Mat<T, K, L>& B,
                                    ADMat<Mat<T, P, Q>>& C) {
  return ADMatMatMultExpr<T, N, M, K, L, P, Q, opA, opB, ADiffType::ACTIVE,
                          ADiffType::PASSIVE>(A, B, C);
}

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL>
A2D_INLINE_FUNCTION auto MatMatMult(Mat<T, N, M>& A, Mat<T, K, L>& B,
                                    ADMat<Mat<T, P, Q>>& C) {
  return ADMatMatMultExpr<T, N, M, K, L, P, Q, opA, opB, ADiffType::PASSIVE,
                          ADiffType::PASSIVE>(A, B, C);
}

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL,
          ADiffType adA = ADiffType::ACTIVE, ADiffType adB = ADiffType::ACTIVE>
class A2DMatMatMultExpr {
 private:
  static constexpr MatOp not_opA = negate_op<opA>::value;
  static constexpr MatOp not_opB = negate_op<opB>::value;

  using Atype = A2DMatType<adA, Mat<T, N, M>>;
  using Btype = A2DMatType<adB, Mat<T, K, L>>;
  using Ctype = A2DMat<Mat<T, P, Q>>;

 public:
  A2D_INLINE_FUNCTION A2DMatMatMultExpr(Atype& A, Btype& B, Ctype& C)
      : A(A), B(B), C(C) {
    MatMatMultCore<T, N, M, K, L, P, Q, opA, opB>(get_data(A), get_data(B),
                                                  get_data(C));
  }

  A2D_INLINE_FUNCTION void forward() {
    if constexpr (adA == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, opA, opB, MatOp::NORMAL, false>(
          get_bdata(A), get_data(B), get_bdata(C));
    }
    if constexpr (adB == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, opA, opB, MatOp::NORMAL,
                     adA == ADiffType::ACTIVE>(get_data(A), get_bdata(B),
                                               get_bdata(C));
    }
  }

  A2D_INLINE_FUNCTION void reverse() {
    if constexpr (adA == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, MatOp::NORMAL, not_opB, opA, true>(
          get_bdata(C), get_data(B), get_bdata(A));
    }
    if constexpr (adB == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, not_opA, MatOp::NORMAL, opB, true>(
          get_data(A), get_bdata(C), get_bdata(B));
    }
  }

  A2D_INLINE_FUNCTION void hforward() {
    if constexpr (adA == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, opA, opB, MatOp::NORMAL, false>(
          get_pdata(A), get_data(B), get_pdata(C));
    }
    if constexpr (adB == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, opA, opB, MatOp::NORMAL,
                     adA == ADiffType::ACTIVE>(get_data(A), get_pdata(B),
                                               get_pdata(C));
    }
  }

  A2D_INLINE_FUNCTION void hreverse() {
    if constexpr (adA == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, MatOp::NORMAL, not_opB, opA, true>(
          get_hdata(C), get_data(B), get_hdata(A));
    }
    if constexpr (adB == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, not_opA, MatOp::NORMAL, opB, true>(
          get_data(A), get_hdata(C), get_hdata(B));
    }
    if constexpr (adA == ADiffType::ACTIVE and adB == ADiffType::ACTIVE) {
      MatMatMultCore<T, N, M, K, L, P, Q, MatOp::NORMAL, not_opB, opA, true>(
          get_bdata(C), get_pdata(B), get_hdata(A));
      MatMatMultCore<T, N, M, K, L, P, Q, not_opA, MatOp::NORMAL, opB, true>(
          get_pdata(A), get_bdata(C), get_hdata(B));
    }
  }

 private:
  Atype& A;
  Btype& B;
  Ctype& C;
};

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL>
A2D_INLINE_FUNCTION auto MatMatMult(A2DMat<Mat<T, N, M>>& A,
                                    A2DMat<Mat<T, K, L>>& B,
                                    A2DMat<Mat<T, P, Q>>& C) {
  return A2DMatMatMultExpr<T, N, M, K, L, P, Q, opA, opB, ADiffType::ACTIVE,
                           ADiffType::ACTIVE>(A, B, C);
}

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL>
A2D_INLINE_FUNCTION auto MatMatMult(Mat<T, N, M>& A, A2DMat<Mat<T, K, L>>& B,
                                    A2DMat<Mat<T, P, Q>>& C) {
  return A2DMatMatMultExpr<T, N, M, K, L, P, Q, opA, opB, ADiffType::PASSIVE,
                           ADiffType::ACTIVE>(A, B, C);
}

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL>
A2D_INLINE_FUNCTION auto MatMatMult(A2DMat<Mat<T, N, M>>& A, Mat<T, K, L>& B,
                                    A2DMat<Mat<T, P, Q>>& C) {
  return A2DMatMatMultExpr<T, N, M, K, L, P, Q, opA, opB, ADiffType::ACTIVE,
                           ADiffType::PASSIVE>(A, B, C);
}

template <typename T, int N, int M, int K, int L, int P, int Q,
          MatOp opA = MatOp::NORMAL, MatOp opB = MatOp::NORMAL>
A2D_INLINE_FUNCTION auto MatMatMult(Mat<T, N, M>& A, Mat<T, K, L>& B,
                                    A2DMat<Mat<T, P, Q>>& C) {
  return A2DMatMatMultExpr<T, N, M, K, L, P, Q, opA, opB, ADiffType::PASSIVE,
                           ADiffType::PASSIVE>(A, B, C);
}

}  // namespace A2D
#endif  // A2D_GEMM_H