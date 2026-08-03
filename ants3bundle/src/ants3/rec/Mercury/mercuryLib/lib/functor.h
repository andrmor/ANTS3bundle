#ifndef LM_FUNCTOR
#define LM_FUNCTOR

#include <Eigen/Core>

// Define a functor for LM optimization
// this particualar structure is needed by LM optimizer
// plus some additional stuff required by numerical differentiation module
template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct Functor {
    typedef _Scalar Scalar;
    enum {
        InputsAtCompileTime = NX,
        ValuesAtCompileTime = NY
    };
    typedef Eigen::Matrix<Scalar, NX, 1> InputType;
    typedef Eigen::Matrix<Scalar, NY, 1> ValueType;
    typedef Eigen::Matrix<Scalar, NY, NX> JacobianType;

    int m_inputs, m_values;
    Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}
    int inputs() const { return m_inputs; }
    int values() const { return m_values; }
};
#endif // 