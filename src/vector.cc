#include "vector.h"

#include <random>
#include <cmath>
#include <assert.h>

#include <iomanip>
#include <cmath>
#include <limits>

namespace poincare {

    Vector::Vector(int64_t m) {
        dimension_ = m;
        data_ = new real[m];
        zero();
    }

    Vector::Vector(const Vector& v) {
        dimension_ = v.dimension_;
        data_ = new real[dimension_];
        for (int64_t i = 0; i < dimension_; ++i) {
            data_[i] = v[i];
        }
    }

    Vector& Vector::operator=(const Vector& v) {
        delete[] data_;
        dimension_ = v.dimension_;
        data_ = new real[dimension_];
        for (int64_t i = 0; i < dimension_; ++i) {
            data_[i] = v[i];
        }
        return *this;
    }

    Vector::~Vector() {
        delete[] data_;
    }

    int64_t Vector::size() const {
        return dimension_;
    }

    void Vector::zero() {
        for (int64_t i = 0; i < dimension_; i++) {
            data_[i] = 0.0;
        }
    }

    void Vector::multiply(real a) {
        for (int64_t i = 0; i < dimension_; i++) {
            data_[i] *= a;
        }
    }

    void Vector::add(const Vector& source) {
        assert(dimension_ == source.dimension_);
        for (int64_t i = 0; i < dimension_; i++) {
            data_[i] += source.data_[i];
        }
    }

    void Vector::add(const Vector& source, real s) {
        assert(dimension_ == source.dimension_);
        for (int64_t i = 0; i < dimension_; i++) {
            data_[i] += s * source.data_[i];
        }
    }

    void Vector::to_ball_point() {
        real denom = data_[dimension_ - 1] + 1;
        data_[dimension_ - 1] = 0;
        multiply(1. / denom);
    }

    void Vector::to_hyperboloid_point() {
        real norm_sqd = minkowski_dot(*this, *this);
        multiply(2. / (1 - norm_sqd));
        data_[dimension_ - 1] = (1 + norm_sqd) / (1 - norm_sqd);
    }

    void Vector::to_ball_tangent(const Vector& hyperboloid_point) {
        real denom = hyperboloid_point[dimension_ - 1] + 1;
        for (int64_t i = 0; i < dimension_ - 1; i++) {
            data_[i] = (data_[i] - hyperboloid_point[i] * data_[dimension_ - 1] / denom) / denom;
        }
        data_[dimension_ - 1] = 0;
    }

    void Vector::to_hyperboloid_tangent(const Vector& ball_point) {
        Vector hyperboloid_pt(ball_point);
        hyperboloid_pt.to_hyperboloid_point();
        real dot_with_bp = dot(ball_point, *this);
        real multiplier = hyperboloid_pt[dimension_ - 1] + 1;
        for (int i = 0; i < dimension_ - 1; i++) {
            data_[i] = multiplier * (hyperboloid_pt[i] * dot_with_bp + data_[i]);
        }
        data_[dimension_ - 1] = pow(multiplier, 2) * dot_with_bp;
    }


    void Vector::geodesic_update(const Vector& tangent_unit_vec, real step_size) {
        multiply(std::cosh(step_size));
        add(tangent_unit_vec, std::sinh(step_size));
        ensure_on_hyperboloid(); // needed?
    }

    void Vector::project_onto_tangent_space(const Vector& hyperboloid_point) {
        real mdp = minkowski_dot(hyperboloid_point, *this);
        add(hyperboloid_point, mdp);
    }

    void Vector::ensure_on_hyperboloid() {
        real euc_norm_sq = 0;
        for (int64_t i = 0; i < dimension_ - 1; ++i) {
            euc_norm_sq += std::pow(data_[i], 2);
        }
        data_[dimension_ - 1] = std::sqrt(euc_norm_sq + 1);
    }

    real& Vector::operator[](int64_t i) {
        return data_[i];
    }

    const real& Vector::operator[](int64_t i) const {
        return data_[i];
    }

    std::ostream& operator<<(std::ostream& os, const Vector& v) {
        os.precision(std::numeric_limits<real>::digits10 + 1);
        for (int64_t j = 0; j < v.dimension_ - 1; j++) {
            os << v.data_[j] << ' ';
        }
        os << v.data_[v.dimension_ - 1];
        return os;
    }

    void random_hyperboloid_point(Vector& vector, std::minstd_rand& rng, real std_dev) {
        std::normal_distribution<real> normal_dist(0, std_dev);
        int64_t n = vector.size();
        // sample a tangent vector at the basepoint from a normal
        // distribution, i.e. sample the first dimension_-1 components
        Vector tangent(n);
        real tangent_norm = 0;
        for (int64_t j = 0; j < n - 1; ++j) {
            tangent[j] = normal_dist(rng);
            tangent_norm += tangent[j] * tangent[j];
        }
        tangent[n - 1] = 0;
        tangent_norm = std::sqrt(tangent_norm);
        tangent.multiply(1. / tangent_norm);
        vector.zero();
        vector[n - 1] = 1;
        vector.geodesic_update(tangent, tangent_norm);
    }

    real distance(const Vector& point0, const Vector& point1) {
        return std::acosh(-minkowski_dot(point0, point1));
    }

    real Vector::squared_norm() const {
        real res = 0;
        for (int32_t i = 0; i < size(); i++) {
            res += pow(data_[i], 2);
        }
        return res;
    }
}
