#ifndef A2D_ENUM_H
#define A2D_ENUM_H
namespace A2D {

/**
 * @brief Whether a variable should be automatically differentiated or not
 */
enum class ADiffType { PASSIVE, ACTIVE };

/**
 * @brief Whether we compute first or second order derivatives
 */
enum class ADorder { FIRST, SECOND };

/**
 * @brief Is the matrix normal (not transposed) or transposed
 */
enum class MatOp { NORMAL, TRANSPOSE };

/**
 * @brief Specify bvalue, pvalue or hvalue
 */
enum class ADseed { b, p, h };

/**
 * @brief Create an opposite MatOp enum
 */
template <MatOp op>
struct negate_op {};

template <>
struct negate_op<MatOp::NORMAL> {
  static constexpr MatOp value = MatOp::TRANSPOSE;
};

template <>
struct negate_op<MatOp::TRANSPOSE> {
  static constexpr MatOp value = MatOp::NORMAL;
};

}  // namespace A2D
#endif  // A2D_ENUM_H