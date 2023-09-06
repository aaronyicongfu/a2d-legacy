#ifndef A2D_TEST_H
#define A2D_TEST_H

#include <iomanip>

#include "a2dvartuple.h"

namespace A2D {

namespace Test {

template <typename T, class Output, class... Inputs>
class A2DTest {
 public:
  /**
   * @brief Construct the Test
   */
  A2DTest(double dh = 1e-50, double rtol = 1e-10, double atol = 1e-30)
      : dh(dh), rtol(rtol), atol(atol) {}

  /**
   * @brief Get the complex-step step size
   */
  double get_step_size() const { return dh; }

  /**
   * @brief Get the point to perform the test at - defaults to random
   *
   * @param x Variable tuple for the inputs
   */

  virtual void get_point(VarTuple<T, Inputs...>& x) { x.set_rand(); }

  /**
   * @brief Get the name of the test
   *
   * @returns The name of the test
   */
  virtual std::string name() = 0;

  /**
   * @brief Evaluate the output as a function of the inputs
   *
   * @param x Variable tuple for the input
   * @return The output value
   */
  virtual VarTuple<T, Output> eval(const VarTuple<T, Inputs...>& x) = 0;

  /**
   * @brief Compute the derivative of the output as a function of the inputs
   *
   * @param seed The seed value for the input
   * @param x Variable tuple for the input
   * @param g Derivative of the output w.r.t. the input
   */
  virtual void deriv(const VarTuple<T, Output>& seed,
                     const VarTuple<T, Inputs...>& x,
                     VarTuple<T, Inputs...>& g) = 0;

  /**
   * @brief Compute the derivative of the output as a function of the inputs
   *
   * @param x Variable tuple for the input
   * @param g Derivative of the output w.r.t. the input
   */
  virtual void hprod(const VarTuple<T, Output>& seed,
                     const VarTuple<T, Output>& hval,
                     const VarTuple<T, Inputs...>& x,
                     const VarTuple<T, Inputs...>& p,
                     VarTuple<T, Inputs...>& h) = 0;

  /**
   * @brief Check whether two values are close to one another
   *
   * @param test_value The test value (computed using AD)
   * @param ref_value The reference value (usually complex-step)
   * @return true If the test passes
   * @return false If the test fails
   */
  bool is_close(const T test_value, const T ref_value) {
    T abs_err = fabs(std::real(test_value - ref_value));
    T combo = atol + rtol * fabs(std::real(ref_value));
    if (std::real(abs_err) > std::real(combo)) {
      return false;
    }
    return true;
  }

  /**
   * @brief Write a brief, one-line summary of the result
   *
   * @param out Where to write the result
   * @param test_value The test value (computed using AD)
   * @param ref_value The reference value (usually complex-step)
   */
  void write_result(std::string str, std::ostream& out, const T test_value,
                    const T ref_value) {
    // Find the result of the test
    bool passed = is_close(test_value, ref_value);

    // Compute the relative error
    T abs_err = fabs(std::real(test_value - ref_value));
    T rel_err = fabs(std::real((test_value - ref_value) / ref_value));

    out << std::scientific << std::setprecision(9) << str
        << " AD: " << std::setw(17) << std::real(test_value)
        << " CS: " << std::setw(17) << std::real(ref_value)
        << " Rel Err: " << std::setw(17) << std::real(rel_err)
        << " Abs Err: " << std::setw(17) << std::real(abs_err);
    if (passed) {
      out << "  PASSED." << std::endl;
    } else {
      out << "  FAILED." << std::endl;
    }
  }

 private:
  double dh;    // Complex-step size
  double rtol;  // relative tolerance
  double atol;  // absolute tolerance
};

/**
 * Run the AD test
 */
template <typename T, class Output, class... Inputs>
bool Run(A2DTest<std::complex<T>, Output, Inputs...>& test,
         bool component = false, bool write_output = true) {
  // Declare all of the variables needed
  VarTuple<std::complex<T>, Inputs...> x, g, x1, p, h;
  VarTuple<std::complex<T>, Output> seed, hvalue;

  // Set a random seed input
  seed.set_rand();
  hvalue.set_rand();

  // Get the starting point
  test.get_point(x);

  bool passed = true;

  // Perform a component-by-component test for gradients and the Hessian-vector
  // products
  if (component) {
    for (int k = 0; k < p.get_num_components(); k++) {
      p.zero();
      p[k] = 1.0;

      test.deriv(seed, x, g);

      // Set x1 = x + dh * p1
      double dh = test.get_step_size();
      for (index_t i = 0; i < x.get_num_components(); i++) {
        x1[i] = std::complex<double>(std::real(x[i]), std::real(dh * p[i]));
      }

      // Compute the complex-step result: fd = p^{T} * df/dx
      VarTuple<std::complex<T>, Output> value = test.eval(x1);
      T fd = 0.0;
      for (index_t i = 0; i < value.get_num_components(); i++) {
        fd += (std::imag(value[i]) / dh) * std::real(seed[i]);
      }

      // Compute the solution from the AD
      T ans = 0.0;
      for (index_t i = 0; i < x.get_num_components(); i++) {
        ans += std::real(g[i] * p[i]);
      }

      passed = passed && test.is_close(ans, fd);

      if (write_output) {
        std::string str = test.name();
        test.write_result(str + " first-order", std::cout, ans, fd);
      }
    }

    if (!passed) {
      return passed;
    }

    for (int k = 0; k < p.get_num_components(); k++) {
      p.zero();
      p[k] = 1.0;

      test.hprod(seed, hvalue, x, p, h);

      // Set x1 = x + dh * p1
      double dh = test.get_step_size();
      for (index_t i = 0; i < x.get_num_components(); i++) {
        x1[i] = std::complex<double>(std::real(x[i]), std::real(dh * p[i]));
      }

      // Set the seed and include the second-derivative parts
      VarTuple<std::complex<T>, Output> seedh;
      for (index_t i = 0; i < seed.get_num_components(); i++) {
        seedh[i] =
            seed[i] + std::complex<double>(0.0, std::real(dh * hvalue[i]));
      }
      test.deriv(seedh, x1, g);

      for (index_t i = 0; i < x.get_num_components(); i++) {
        T ans = std::real(h[i]);
        T fd = std::imag(g[i]) / dh;

        passed = passed && test.is_close(ans, fd);

        if (write_output) {
          std::stringstream s;
          s << " second-order [" << i << "]";
          std::string str = test.name() + s.str();
          test.write_result(str, std::cout, ans, fd);
        }
      }
      if (write_output) {
        std::cout << " " << std::endl;
      }
    }
  } else {
    // Set a random direction for the test
    p.set_rand();

    // Evaluate the function and its derivatives
    test.deriv(seed, x, g);
    test.hprod(seed, hvalue, x, p, h);

    // Set x1 = x + dh * p1
    double dh = test.get_step_size();
    for (index_t i = 0; i < x.get_num_components(); i++) {
      x1[i] = std::complex<double>(std::real(x[i]), std::real(dh * p[i]));
    }

    // Compute the complex-step result: fd = p^{T} * df/dx
    VarTuple<std::complex<T>, Output> value = test.eval(x1);
    T fd = 0.0;
    for (index_t i = 0; i < value.get_num_components(); i++) {
      fd += (std::imag(value[i]) / dh) * std::real(seed[i]);
    }

    // Compute the solution from the AD
    T ans = 0.0;
    for (index_t i = 0; i < x.get_num_components(); i++) {
      ans += std::real(g[i] * p[i]);
    }

    passed = test.is_close(ans, fd);

    if (write_output) {
      std::string str = test.name();
      test.write_result(str + " first-order", std::cout, ans, fd);
    }

    // Set the seed and include the second-derivative parts
    for (index_t i = 0; i < seed.get_num_components(); i++) {
      seed[i] = seed[i] + std::complex<double>(0.0, std::real(dh * hvalue[i]));
    }
    test.deriv(seed, x1, g);

    for (index_t i = 0; i < x.get_num_components(); i++) {
      T ans = std::real(h[i]);
      T fd = std::imag(g[i]) / dh;

      passed = passed && test.is_close(ans, fd);

      if (write_output) {
        std::string str = test.name();
        test.write_result(str + " second-order", std::cout, ans, fd);
      }
    }
  }

  return passed;
}

}  // namespace Test

}  // namespace A2D

#endif  // A2D_TEST_H