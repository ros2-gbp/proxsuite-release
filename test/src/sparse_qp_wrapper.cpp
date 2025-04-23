//
// Copyright (c) 2022-2023 INRIA
//
#include <iostream>
#include <proxsuite/proxqp/sparse/sparse.hpp>
#include <proxsuite/proxqp/utils/random_qp_problems.hpp>
#include <doctest.hpp>
#include <proxsuite/linalg/veg/util/dynstack_alloc.hpp>

using namespace proxsuite;
using namespace proxsuite::proxqp;
using namespace proxsuite::proxqp::utils;
using T = double;
using I = c_int;
using namespace proxsuite::linalg::sparse::tags;

DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with inequality constraints"
  "and empty equality constraints")
{
  std::cout << "---testing sparse random strongly convex qp with inequality "
               "constraints "
               "and empty equality constraints---"
            << std::endl;
  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 0, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    // Testing with empty but properly sized matrix A  of size (0, 10)
    std::cout << "Solving QP with" << std::endl;
    std::cout << "n: " << n << std::endl;
    std::cout << "n_eq: " << n_eq << std::endl;
    std::cout << "n_in: " << n_in << std::endl;
    std::cout << "H :  " << qp_random.H << std::endl;
    std::cout << "g :  " << qp_random.g << std::endl;
    std::cout << "A :  " << qp_random.A << std::endl;
    std::cout << "b :  " << qp_random.b << std::endl;
    std::cout << "C :  " << qp_random.C << std::endl;
    std::cout << "u :  " << qp_random.u << std::endl;
    std::cout << "l :  " << qp_random.l << std::endl;

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.settings.verbose = false;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            true,
            T(1.e-7),
            nullopt,
            nullopt);
    std::cout << "after upating" << std::endl;
    std::cout << "rho :  " << qp.results.info.rho << std::endl;
    qp.solve();
    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    // Testing with empty matrix A  of size (0, 0)
    qp_random.A = Eigen::SparseMatrix<double>();
    qp_random.b = Eigen::VectorXd();

    std::cout << "Solving QP with" << std::endl;
    std::cout << "n: " << n << std::endl;
    std::cout << "n_eq: " << n_eq << std::endl;
    std::cout << "n_in: " << n_in << std::endl;
    std::cout << "H :  " << qp_random.H << std::endl;
    std::cout << "g :  " << qp_random.g << std::endl;
    std::cout << "A :  " << qp_random.A << std::endl;
    std::cout << "b :  " << qp_random.b << std::endl;
    std::cout << "C :  " << qp_random.C << std::endl;
    std::cout << "u :  " << qp_random.u << std::endl;
    std::cout << "l :  " << qp_random.l << std::endl;

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in);
    qp2.settings.eps_abs = 1.E-9;
    qp2.settings.verbose = false;
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             true,
             T(1.e-7),
             nullopt,
             nullopt);
    std::cout << "after upating" << std::endl;
    std::cout << "rho :  " << qp2.results.info.rho << std::endl;
    qp2.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.C.transpose() * qp.results.z);
    pri_res = proxqp::dense::infty_norm(
      helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
      helpers::negative_part(qp_random.C * qp.results.x - qp_random.l));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    // Testing with nullopt
    proxqp::sparse::QP<T, I> qp3(n, n_eq, n_in);
    qp3.settings.eps_abs = 1.E-9;
    qp3.settings.verbose = false;
    qp3.init(qp_random.H,
             qp_random.g,
             nullopt,
             nullopt,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             true,
             T(1.e-7),
             nullopt,
             nullopt);
    std::cout << "after upating" << std::endl;
    std::cout << "rho :  " << qp3.results.info.rho << std::endl;
    qp3.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.C.transpose() * qp.results.z);
    pri_res = proxqp::dense::infty_norm(
      helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
      helpers::negative_part(qp_random.C * qp.results.x - qp_random.l));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test update rho")
{
  std::cout << "------------------------sparse random strongly convex qp with "
               "equality and inequality constraints: test update rho"
            << std::endl;
  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.settings.verbose = false;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            true,
            T(1.e-7),
            nullopt,
            nullopt);
    std::cout << "after upating" << std::endl;
    std::cout << "rho :  " << qp.results.info.rho << std::endl;
    qp.solve();
    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test setting SparseBackend")
{
  std::cout << "------------------------sparse random strongly convex qp with "
               "equality and inequality constraints: test setting SparseBackend"
            << std::endl;
  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.settings.verbose = true;
    CHECK(qp.settings.sparse_backend ==
          proxsuite::proxqp::SparseBackend::Automatic);
    qp.settings.sparse_backend = proxsuite::proxqp::SparseBackend::MatrixFree;
    CHECK(qp.settings.sparse_backend ==
          proxsuite::proxqp::SparseBackend::MatrixFree);
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            true,
            T(1.e-7),
            nullopt,
            nullopt);
    qp.solve();
    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    CHECK(qp.results.info.sparse_backend ==
          proxsuite::proxqp::SparseBackend::MatrixFree);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "dual residual " << dua_res << "; primal residual " << pri_res
              << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in);
    qp2.settings.eps_abs = 1.E-9;
    qp2.settings.verbose = true;
    CHECK(qp2.settings.sparse_backend ==
          proxsuite::proxqp::SparseBackend::Automatic);
    qp2.settings.sparse_backend =
      proxsuite::proxqp::SparseBackend::SparseCholesky;
    CHECK(qp2.settings.sparse_backend ==
          proxsuite::proxqp::SparseBackend::SparseCholesky);
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             true,
             T(1.e-7),
             nullopt,
             nullopt);
    qp2.solve();
    T dua_res2 = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
      qp_random.g + qp_random.A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    T pri_res2 = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    CHECK(dua_res2 <= 1e-9);
    CHECK(pri_res2 <= 1E-9);
    CHECK(qp2.results.info.sparse_backend ==
          proxsuite::proxqp::SparseBackend::SparseCholesky);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "dual residual " << dua_res2 << "; primal residual "
              << pri_res2 << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;
    CHECK(qp.results.x.isApprox(qp2.results.x, 1E-4));
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test update mus")
{

  std::cout << "------------------------sparse random strongly convex qp with "
               "equality and inequality constraints: test update mus"
            << std::endl;
  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            true,
            nullopt,
            T(1.E-2),
            T(1.E-3));
    std::cout << "after upating" << std::endl;
    std::cout << "mu_eq :  " << qp.results.info.mu_eq << std::endl;
    std::cout << "mu_in :  " << qp.results.info.mu_in << std::endl;
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}
TEST_CASE(
  "sparse random strongly convex qp with equality and "
  "inequality constraints: test with no equilibration at initialization")
{

  std::cout << "------------------------sparse random strongly convex qp with "
               "equality and inequality constraints: test with no "
               "equilibration at initialization"
            << std::endl;
  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            false);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test with equilibration at initialization")
{

  std::cout
    << "------------------------sparse random strongly convex qp with equality "
       "and inequality constraints: test with equilibration at initialization"
    << std::endl;
  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            true);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}
TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test with no initial guess")
{

  std::cout << "------------------------sparse random strongly convex qp with "
               "equality and inequality constraints: test with no initial guess"
            << std::endl;
  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test update g for unconstrained problem")
{

  std::cout << "------------------------sparse random strongly convex qp with "
               "equality and inequality constraints: test with no initial guess"
            << std::endl;
  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();
    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    auto g = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    std::cout << "H before update " << qp_random.H << std::endl;
    auto H_new = 2. * qp_random.H; // keep same sparsity structure
    std::cout << "H generated " << H_new << std::endl;
    qp.update(H_new,
              g,
              qp_random.A,
              qp_random.b,
              qp_random.C,
              qp_random.l,
              qp_random.u,
              false);
    proxsuite::linalg::sparse::MatMut<T, I> kkt_unscaled =
      qp.model.kkt_mut_unscaled();
    auto kkt_top_n_rows =
      proxsuite::proxqp::sparse::detail::top_rows_mut_unchecked(
        proxsuite::linalg::veg::unsafe, kkt_unscaled, n);

    proxsuite::linalg::sparse::MatMut<T, I> H_unscaled =
      proxsuite::proxqp::sparse::detail::middle_cols_mut(
        kkt_top_n_rows, 0, n, qp.model.H_nnz);
    std::cout << " H_unscaled " << H_unscaled.to_eigen() << std::endl;
    qp.solve();

    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test warm starting")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    auto x_wm = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    auto y_wm = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n_eq);
    auto z_wm = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n_in);
    std::cout << "proposed warm start" << std::endl;
    std::cout << "x_wm :  " << x_wm << std::endl;
    std::cout << "y_wm :  " << y_wm << std::endl;
    std::cout << "z_wm :  " << z_wm << std::endl;
    qp.solve(x_wm, y_wm, z_wm);

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

DOCTEST_TEST_CASE(
  "sparse random strongly convex qp with equality and "
  "inequality constraints: test with warm start with previous result")
{

  std::cout
    << "---testing sparse random strongly convex qp with equality and "
       "inequality constraints: test with warm start with previous result---"
    << std::endl;

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);
    T eps_abs = 1.E-9;

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in); // creating QP object
    qp.settings.eps_abs = eps_abs;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with qp: " << n
              << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in); // creating QP object
    qp2.settings.eps_abs = 1.E-9;
    qp2.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             true);

    auto x = qp.results.x;
    auto y = qp.results.y;
    auto z = qp.results.z;
    qp2.solve(x, y, z);
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
    qp.solve();
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    std::cout << "------using API solving qp with dim with qp after warm start "
                 "with previous result: "
              << n << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
    pri_res = std::max(
      (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
               qp_random.g + qp_random.A.transpose() * qp2.results.y +
               qp_random.C.transpose() * qp2.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with qp2: " << n
              << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;
  }
}

DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test with cold start option")
{

  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test with cold start option---"
            << std::endl;

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    T eps_abs = 1.E-9;
    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in); // creating QP object
    qp.settings.eps_abs = eps_abs;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
                 qp_random.g + qp_random.A.transpose() * qp.results.y +
                 qp_random.C.transpose() * qp.results.z)
                  .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with qp: " << n
              << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in); // creating QP object
    qp2.settings.eps_abs = 1.E-9;
    qp2.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);

    auto x = qp.results.x;
    auto y = qp.results.y;
    auto z = qp.results.z;
    // std::cout << "after scaling x " << x <<  " qp.results.x " << qp.results.x
    // << std::endl;
    qp2.ruiz.scale_primal_in_place({ proxsuite::proxqp::from_eigen, x });
    qp2.ruiz.scale_dual_in_place_eq({ proxsuite::proxqp::from_eigen, y });
    qp2.ruiz.scale_dual_in_place_in({ proxsuite::proxqp::from_eigen, z });
    // std::cout << "after scaling x " << x <<  " qp.results.x " << qp.results.x
    // << std::endl;
    qp2.solve(x, y, z);

    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
    qp.solve();
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    std::cout << "------using API solving qp with dim with qp after warm start "
                 "with cold start option: "
              << n << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
    pri_res = std::max(
      (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
               qp_random.g + qp_random.A.transpose() * qp2.results.y +
               qp_random.C.transpose() * qp2.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with cold start option: "
              << n << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;
  }
}

DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test equilibration option")
{

  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test equilibration option---"
            << std::endl;

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    T eps_abs = 1.E-9;
    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in); // creating QP object
    qp.settings.eps_abs = eps_abs;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            true);
    qp.solve();

    T pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
                 qp_random.g + qp_random.A.transpose() * qp.results.y +
                 qp_random.C.transpose() * qp.results.z)
                  .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with qp: " << n
              << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in); // creating QP object
    qp2.settings.eps_abs = 1.E-9;
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             false);
    qp2.solve();
    pri_res = std::max(
      (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
               qp_random.g + qp_random.A.transpose() * qp2.results.y +
               qp_random.C.transpose() * qp2.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with qp2: " << n
              << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;
  }
}

DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test equilibration option at update")
{

  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test equilibration option at update---"
            << std::endl;

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    T eps_abs = 1.E-9;
    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in); // creating QP object
    qp.settings.eps_abs = eps_abs;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            true);
    qp.solve();
    T pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
                 qp_random.g + qp_random.A.transpose() * qp.results.y +
                 qp_random.C.transpose() * qp.results.z)
                  .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with qp: " << n
              << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
    qp.solve();

    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with qp: " << n
              << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in); // creating QP object
    qp2.settings.eps_abs = 1.E-9;
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             false);

    qp2.solve();
    pri_res = std::max(
      (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
               qp_random.g + qp_random.A.transpose() * qp2.results.y +
               qp_random.C.transpose() * qp2.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with qp2: " << n
              << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;

    qp2.update(
      nullopt, nullopt, nullopt, nullopt, nullopt, nullopt, nullopt, false);
    qp2.solve();
    pri_res = std::max(
      (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
               qp_random.g + qp_random.A.transpose() * qp2.results.y +
               qp_random.C.transpose() * qp2.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
    std::cout << "------using API solving qp with dim with qp2: " << n
              << " neq: " << n_eq << " nin: " << n_in << std::endl;
    std::cout << "primal residual: " << pri_res << std::endl;
    std::cout << "dual residual: " << dua_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test new init")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(n, n_eq, n_in);
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    auto x_wm = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    auto y_wm = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n_eq);
    auto z_wm = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n_in);
    std::cout << "proposed warm start" << std::endl;
    std::cout << "x_wm :  " << x_wm << std::endl;
    std::cout << "y_wm :  " << y_wm << std::endl;
    std::cout << "z_wm :  " << z_wm << std::endl;
    qp.solve(x_wm, y_wm, z_wm);

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test new init")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in);
    qp2.settings.eps_abs = 1.E-9;

    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp2.solve();

    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
      qp_random.g + qp_random.A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;
  }
}
/////  TESTS ALL INITIAL GUESS OPTIONS FOR MULTIPLE SOLVES AT ONCE
TEST_CASE(
  "sparse random strongly convex qp with equality and "
  "inequality constraints: test multiple solve at once with no initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;

    std::cout << "Test with no initial guess" << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test multiple solve at once with equality "
          "constrained initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;

    std::cout << "Test with equality constrained initial guess" << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test multiple solve at once with equality "
          "constrained initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;

    std::cout << "Test with warm start with previous result and first solve "
                 "with equality constrained initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}
TEST_CASE(
  "sparse random strongly convex qp with equality and "
  "inequality constraints: test multiple solve at once with no initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;

    std::cout << "Test with warm start with previous result and first solve "
                 "with no initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}
TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test multiple solve at once with cold start "
          "initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;

    std::cout << "Test with cold start with previous result and first solve "
                 "with equality constrained initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test multiple solve at once with warm start")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;

    std::cout << "Test with warm start and first solve with no initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve(qp.results.x, qp.results.y, qp.results.z);
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve(qp.results.x, qp.results.y, qp.results.z);
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve(qp.results.x, qp.results.y, qp.results.z);
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: warm start test from init")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;

    std::cout << "Test with warm start and first solve with no initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp2(qp_random.H.cast<bool>(),
                                 qp_random.A.cast<bool>(),
                                 qp_random.C.cast<bool>());
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp2.settings.eps_abs = 1.E-9;
    qp2.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    std::cout << "dirty workspace for qp2 : " << qp2.work.internal.dirty
              << std::endl;
    qp2.solve(qp.results.x, qp.results.y, qp.results.z);
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
      qp_random.g + qp_random.A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve with new QP object" << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;
  }
}

/// TESTS WITH UPDATE + INITIAL GUESS OPTIONS

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test update and multiple solve at once with "
          "no initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;

    std::cout << "Test with no initial guess" << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    auto H_new = 2. * qp_random.H; // keep same sparsity structure
    auto g_new = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    bool update_preconditioner = true;
    qp.update(H_new,
              g_new,
              qp_random.A,
              qp_random.b,
              qp_random.C,
              qp_random.l,
              qp_random.u,
              update_preconditioner);
    std::cout << "dirty workspace after update : " << qp.work.internal.dirty
              << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test update + multiple solve at once with "
          "equality constrained initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;

    std::cout << "Test with equality constrained initial guess" << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    auto H_new = 2. * qp_random.H; // keep same sparsity structure
    auto g_new = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    bool update_preconditioner = true;
    qp.update(H_new,
              g_new,
              qp_random.A,
              qp_random.b,
              qp_random.C,
              qp_random.l,
              qp_random.u,
              update_preconditioner);
    std::cout << "dirty workspace after update : " << qp.work.internal.dirty
              << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE(
  "sparse random strongly convex qp with equality and "
  "inequality constraints: test update + multiple solve at once with equality "
  "constrained initial guess and then warm start with previous results")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;

    std::cout << "Test with warm start with previous result and first solve "
                 "with equality constrained initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    auto H_new = 2. * qp_random.H; // keep same sparsity structure
    auto g_new = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    bool update_preconditioner = true;
    qp.update(H_new,
              g_new,
              qp_random.A,
              qp_random.b,
              qp_random.C,
              qp_random.l,
              qp_random.u,
              update_preconditioner);
    std::cout << "dirty workspace after update : " << qp.work.internal.dirty
              << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}
TEST_CASE(
  "sparse random strongly convex qp with equality and "
  "inequality constraints: test multiple solve at once with no initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;

    std::cout << "Test with warm start with previous result and first solve "
                 "with no initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    auto H_new = 2. * qp_random.H; // keep same sparsity structure
    auto g_new = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    bool update_preconditioner = true;
    qp.update(H_new,
              g_new,
              qp_random.A,
              qp_random.b,
              qp_random.C,
              qp_random.l,
              qp_random.u,
              update_preconditioner);
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}
TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test update + multiple solve at once with "
          "cold start initial guess and then cold start option")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;

    std::cout << "Test with cold start with previous result and first solve "
                 "with equality constrained initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    auto H_new = 2. * qp_random.H; // keep same sparsity structure
    auto g_new = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    bool update_preconditioner = true;
    qp.update(H_new,
              g_new,
              nullopt,
              nullopt,
              nullopt,
              nullopt,
              nullopt,
              update_preconditioner);
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: sparse random strongly convex qp with equality and "
          "inequality constraints: test update + multiple solve at once with "
          "warm start")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = 1.E-9;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;

    std::cout << "Test with warm start and first solve with no initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    auto x_wm = qp.results.x; // keep previous result
    auto y_wm = qp.results.y;
    auto z_wm = qp.results.z;
    // try with a false update, the warm start should give the exact solution
    bool update_preconditioner = true;
    qp.update(qp_random.H,
              qp_random.g,
              qp_random.A,
              qp_random.b,
              qp_random.C,
              qp_random.l,
              qp_random.u,
              update_preconditioner);
    std::cout << "dirty workspace after update: " << qp.work.internal.dirty
              << std::endl;
    qp.solve(x_wm, y_wm, z_wm);
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Second solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    x_wm = qp.results.x; // keep previous result
    y_wm = qp.results.y;
    z_wm = qp.results.z;
    auto H_new = 2. * qp_random.H; // keep same sparsity structure
    auto g_new = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    update_preconditioner = true;
    qp.update(H_new,
              g_new,
              qp_random.A,
              qp_random.b,
              qp_random.C,
              qp_random.l,
              qp_random.u,
              update_preconditioner);
    std::cout << "dirty workspace after update: " << qp.work.internal.dirty
              << std::endl;
    qp.solve(x_wm, y_wm, z_wm);
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Third solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve(qp.results.x, qp.results.y, qp.results.z);
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fourth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    std::cout << "dirty workspace : " << qp.work.internal.dirty << std::endl;
    qp.solve(qp.results.x, qp.results.y, qp.results.z);
    dua_res = proxqp::dense::infty_norm(
      H_new.selfadjointView<Eigen::Upper>() * qp.results.x + g_new +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= 1e-9);
    CHECK(pri_res <= 1E-9);
    std::cout << "Fifth solve " << std::endl;
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
  }
}

TEST_CASE(
  "ProxQP::sparse: Test initializaton with rho for different initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    double eps_abs = 1.e-9;

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = eps_abs;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;

    std::cout << "Test initializaton with rho for different initial guess"
              << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            true,
            T(1.E-7));
    qp.solve();
    CHECK(qp.results.info.rho == T(1.E-7));
    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in);
    qp2.settings.eps_abs = eps_abs;
    qp2.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             true,
             T(1.E-7));
    qp2.solve();
    CHECK(qp2.results.info.rho == T(1.E-7));
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
      qp_random.g + qp_random.A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp3(n, n_eq, n_in);
    qp3.settings.eps_abs = eps_abs;
    qp3.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
    qp3.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             true,
             T(1.E-7));
    qp3.solve();
    CHECK(qp3.results.info.rho == T(1.E-7));
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
      qp_random.g + qp_random.A.transpose() * qp3.results.y +
      qp_random.C.transpose() * qp3.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp3.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp3.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp3.results.info.setup_time
              << " solve time " << qp3.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp4(n, n_eq, n_in);
    qp4.settings.eps_abs = eps_abs;
    qp4.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
    qp4.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             true,
             T(1.E-7));
    qp4.solve();
    CHECK(qp4.results.info.rho == T(1.E-7));
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp4.results.x +
      qp_random.g + qp_random.A.transpose() * qp4.results.y +
      qp_random.C.transpose() * qp4.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp4.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp4.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp4.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp4.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp4.results.info.setup_time
              << " solve time " << qp4.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp5(n, n_eq, n_in);
    qp5.settings.eps_abs = eps_abs;
    qp5.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    qp5.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u,
             true,
             T(1.E-7));
    qp5.solve(qp3.results.x, qp3.results.y, qp3.results.z);
    CHECK(qp5.results.info.rho == T(1.E-7));
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp5.results.x +
      qp_random.g + qp_random.A.transpose() * qp5.results.y +
      qp_random.C.transpose() * qp5.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp5.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp5.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp5.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp5.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp5.results.info.setup_time
              << " solve time " << qp5.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: Test g update for different initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 2, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    double eps_abs = 1.e-9;

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = eps_abs;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::NO_INITIAL_GUESS;

    std::cout << "Test g update for different initial guess" << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();
    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    auto g = ::proxsuite::proxqp::utils::rand::vector_rand<T>(n);
    qp.update(nullopt, g, nullopt, nullopt, nullopt, nullopt, nullopt);
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK((qp.model.g - g).lpNorm<Eigen::Infinity>() <= eps_abs);
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in);
    qp2.settings.eps_abs = eps_abs;
    qp2.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp2.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
      qp_random.g + qp_random.A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp2.update(nullopt, g, nullopt, nullopt, nullopt, nullopt, nullopt);
    qp2.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x + g +
      qp_random.A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    CHECK((qp2.model.g - g).lpNorm<Eigen::Infinity>() <= eps_abs);
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp3(n, n_eq, n_in);
    qp3.settings.eps_abs = eps_abs;
    qp3.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
    qp3.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp3.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
      qp_random.g + qp_random.A.transpose() * qp3.results.y +
      qp_random.C.transpose() * qp3.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp3.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp3.update(nullopt, g, nullopt, nullopt, nullopt, nullopt, nullopt);
    qp3.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x + g +
      qp_random.A.transpose() * qp3.results.y +
      qp_random.C.transpose() * qp3.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp3.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l)));
    CHECK((qp3.model.g - g).lpNorm<Eigen::Infinity>() <= eps_abs);
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp3.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp3.results.info.setup_time
              << " solve time " << qp3.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp4(n, n_eq, n_in);
    qp4.settings.eps_abs = eps_abs;
    qp4.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
    qp4.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp4.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp4.results.x +
      qp_random.g + qp_random.A.transpose() * qp4.results.y +
      qp_random.C.transpose() * qp4.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp4.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp4.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp4.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp4.update(nullopt, g, nullopt, nullopt, nullopt, nullopt, nullopt);
    qp4.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp4.results.x + g +
      qp_random.A.transpose() * qp4.results.y +
      qp_random.C.transpose() * qp4.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp4.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp4.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp4.results.x - qp_random.l)));
    CHECK((qp4.model.g - g).lpNorm<Eigen::Infinity>() <= eps_abs);
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp4.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp4.results.info.setup_time
              << " solve time " << qp4.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp5(n, n_eq, n_in);
    qp5.settings.eps_abs = eps_abs;
    qp5.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    qp5.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp5.solve(qp3.results.x, qp3.results.y, qp3.results.z);
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp5.results.x +
      qp_random.g + qp_random.A.transpose() * qp5.results.y +
      qp_random.C.transpose() * qp5.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp5.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp5.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp5.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp5.update(nullopt, g, nullopt, nullopt, nullopt, nullopt, nullopt);
    qp5.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp5.results.x + g +
      qp_random.A.transpose() * qp5.results.y +
      qp_random.C.transpose() * qp5.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp5.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp5.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp5.results.x - qp_random.l)));
    CHECK((qp5.model.g - g).lpNorm<Eigen::Infinity>() <= eps_abs);
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp5.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp5.results.info.setup_time
              << " solve time " << qp5.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: Test A update for different initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 3, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    double eps_abs = 1.e-9;

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    // proxqp::sparse::QP<T,I> qp(n,n_eq,n_in);
    qp.settings.eps_abs = eps_abs;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;

    std::cout << "Test A update for different initial guess" << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();
    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    SparseMat<T> A = 2 * qp_random.A; // keep same sparsity structure
    qp.update(nullopt, nullopt, A, nullopt, nullopt, nullopt, nullopt);
    qp.settings.verbose = false;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      A.transpose() * qp.results.y + qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
    // get stored A from KKT matrix
    auto kkt_unscaled = qp.model.kkt_mut_unscaled();
    auto kkt_top_n_rows =
      proxsuite::proxqp::sparse::detail::top_rows_mut_unchecked(
        proxsuite::linalg::veg::unsafe, kkt_unscaled, n);
    SparseMat<T> A_unscaled =
      proxsuite::proxqp::sparse::detail::middle_cols_mut(
        kkt_top_n_rows, n, n_eq, qp.model.A_nnz)
        .to_eigen()
        .transpose();
    SparseMat<T> diff_mat = A_unscaled - A;
    T diff = std::max(std::abs(diff_mat.coeffs().maxCoeff()),
                      std::abs(diff_mat.coeffs().minCoeff()));
    CHECK(diff <= eps_abs);

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in);
    qp2.settings.eps_abs = eps_abs;
    qp2.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp2.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
      qp_random.g + qp_random.A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp2.update(nullopt, nullopt, A, nullopt, nullopt, nullopt, nullopt);
    qp2.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
      qp_random.g + A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    // get stored A from KKT matrix
    kkt_unscaled = qp2.model.kkt_mut_unscaled();
    kkt_top_n_rows = proxsuite::proxqp::sparse::detail::top_rows_mut_unchecked(
      proxsuite::linalg::veg::unsafe, kkt_unscaled, n);
    A_unscaled = proxsuite::proxqp::sparse::detail::middle_cols_mut(
                   kkt_top_n_rows, n, n_eq, qp2.model.A_nnz)
                   .to_eigen()
                   .transpose();
    diff_mat = A_unscaled - A;
    diff = std::max(std::abs(diff_mat.coeffs().maxCoeff()),
                    std::abs(diff_mat.coeffs().minCoeff()));
    CHECK(diff <= eps_abs);
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp3(n, n_eq, n_in);
    qp3.settings.eps_abs = eps_abs;
    qp3.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
    qp3.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp3.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
      qp_random.g + qp_random.A.transpose() * qp3.results.y +
      qp_random.C.transpose() * qp3.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp3.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp3.update(nullopt, nullopt, A, nullopt, nullopt, nullopt, nullopt);
    qp3.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
      qp_random.g + A.transpose() * qp3.results.y +
      qp_random.C.transpose() * qp3.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(A * qp3.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l)));
    // get stored A from KKT matrix
    kkt_unscaled = qp3.model.kkt_mut_unscaled();
    kkt_top_n_rows = proxsuite::proxqp::sparse::detail::top_rows_mut_unchecked(
      proxsuite::linalg::veg::unsafe, kkt_unscaled, n);
    A_unscaled = proxsuite::proxqp::sparse::detail::middle_cols_mut(
                   kkt_top_n_rows, n, n_eq, qp3.model.A_nnz)
                   .to_eigen()
                   .transpose();
    diff_mat = A_unscaled - A;
    diff = std::max(std::abs(diff_mat.coeffs().maxCoeff()),
                    std::abs(diff_mat.coeffs().minCoeff()));
    CHECK(diff <= eps_abs);
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp3.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp3.results.info.setup_time
              << " solve time " << qp3.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp4(n, n_eq, n_in);
    qp4.settings.eps_abs = eps_abs;
    qp4.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
    qp4.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp4.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp4.results.x +
      qp_random.g + qp_random.A.transpose() * qp4.results.y +
      qp_random.C.transpose() * qp4.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp4.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp4.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp4.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp4.update(nullopt, nullopt, A, nullopt, nullopt, nullopt, nullopt);
    qp4.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp4.results.x +
      qp_random.g + A.transpose() * qp4.results.y +
      qp_random.C.transpose() * qp4.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(A * qp4.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp4.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp4.results.x - qp_random.l)));
    // get stored A from KKT matrix
    kkt_unscaled = qp4.model.kkt_mut_unscaled();
    kkt_top_n_rows = proxsuite::proxqp::sparse::detail::top_rows_mut_unchecked(
      proxsuite::linalg::veg::unsafe, kkt_unscaled, n);
    A_unscaled = proxsuite::proxqp::sparse::detail::middle_cols_mut(
                   kkt_top_n_rows, n, n_eq, qp4.model.A_nnz)
                   .to_eigen()
                   .transpose();
    diff_mat = A_unscaled - A;
    diff = std::max(std::abs(diff_mat.coeffs().maxCoeff()),
                    std::abs(diff_mat.coeffs().minCoeff()));
    CHECK(diff <= eps_abs);
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp4.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp4.results.info.setup_time
              << " solve time " << qp4.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp5(n, n_eq, n_in);
    qp5.settings.eps_abs = eps_abs;
    qp5.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    qp5.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp5.solve(qp3.results.x, qp3.results.y, qp3.results.z);
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp5.results.x +
      qp_random.g + qp_random.A.transpose() * qp5.results.y +
      qp_random.C.transpose() * qp5.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp5.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp5.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp5.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp5.update(nullopt, nullopt, A, nullopt, nullopt, nullopt, nullopt);
    qp5.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp5.results.x +
      qp_random.g + A.transpose() * qp5.results.y +
      qp_random.C.transpose() * qp5.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(A * qp5.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp5.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp5.results.x - qp_random.l)));
    // get stored A from KKT matrix
    kkt_unscaled = qp5.model.kkt_mut_unscaled();
    kkt_top_n_rows = proxsuite::proxqp::sparse::detail::top_rows_mut_unchecked(
      proxsuite::linalg::veg::unsafe, kkt_unscaled, n);
    A_unscaled = proxsuite::proxqp::sparse::detail::middle_cols_mut(
                   kkt_top_n_rows, n, n_eq, qp5.model.A_nnz)
                   .to_eigen()
                   .transpose();
    diff_mat = A_unscaled - A;
    diff = std::max(std::abs(diff_mat.coeffs().maxCoeff()),
                    std::abs(diff_mat.coeffs().minCoeff()));
    CHECK(diff <= eps_abs);
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp5.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp5.results.info.setup_time
              << " solve time " << qp5.results.info.solve_time << std::endl;
  }
}

TEST_CASE("ProxQP::sparse: Test rho update for different initial guess")
{

  for (auto const& dims : { // proxsuite::linalg::veg::tuplify(50, 0, 0),
                            // proxsuite::linalg::veg::tuplify(50, 25, 0),
                            // proxsuite::linalg::veg::tuplify(10, 0, 10),
                            // proxsuite::linalg::veg::tuplify(50, 0, 25),
                            // proxsuite::linalg::veg::tuplify(50, 10, 25),
                            proxsuite::linalg::veg::tuplify(10, 3, 2) }) {
    VEG_BIND(auto const&, (n, n_eq, n_in), dims);

    double eps_abs = 1.e-9;

    ::proxsuite::proxqp::utils::rand::set_seed(1);
    T sparsity_factor = 0.15;
    T strong_convexity_factor = 0.01;
    ::proxsuite::proxqp::utils::rand::set_seed(1);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      n, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    // proxqp::sparse::QP<T,I> qp(n,n_eq,n_in);
    qp.settings.eps_abs = eps_abs;
    qp.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;

    std::cout << "Test rho update for different initial guess" << std::endl;
    std::cout << "dirty workspace before any solving: "
              << qp.work.internal.dirty << std::endl;

    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();
    T dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    T pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp.update(nullopt,
              nullopt,
              nullopt,
              nullopt,
              nullopt,
              nullopt,
              nullopt,
              true,
              T(1.E-7));
    qp.settings.verbose = false;
    qp.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x + qp_random.g +
      qp_random.A.transpose() * qp.results.y +
      qp_random.C.transpose() * qp.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp.results.info.setup_time << " solve time "
              << qp.results.info.solve_time << std::endl;
    CHECK(qp.results.info.rho == T(1.E-7));

    proxqp::sparse::QP<T, I> qp2(n, n_eq, n_in);
    qp2.settings.eps_abs = eps_abs;
    qp2.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
    qp2.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp2.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
      qp_random.g + qp_random.A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp2.update(nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               true,
               T(1.E-7));
    qp2.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
      qp_random.g + qp_random.A.transpose() * qp2.results.y +
      qp_random.C.transpose() * qp2.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp2.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l)));
    CHECK(qp2.results.info.rho == T(1.E-7));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp2.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp2.results.info.setup_time
              << " solve time " << qp2.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp3(n, n_eq, n_in);
    qp3.settings.eps_abs = eps_abs;
    qp3.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
    qp3.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp3.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
      qp_random.g + qp_random.A.transpose() * qp3.results.y +
      qp_random.C.transpose() * qp3.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp3.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp3.update(nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               true,
               T(1.E-7));
    qp3.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
      qp_random.g + qp_random.A.transpose() * qp3.results.y +
      qp_random.C.transpose() * qp3.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp3.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l)));
    CHECK(qp3.results.info.rho == T(1.E-7));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp3.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp3.results.info.setup_time
              << " solve time " << qp3.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp4(n, n_eq, n_in);
    qp4.settings.eps_abs = eps_abs;
    qp4.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
    qp4.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp4.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp4.results.x +
      qp_random.g + qp_random.A.transpose() * qp4.results.y +
      qp_random.C.transpose() * qp4.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp4.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp4.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp4.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp4.update(nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               true,
               T(1.E-7));
    qp4.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp4.results.x +
      qp_random.g + qp_random.A.transpose() * qp4.results.y +
      qp_random.C.transpose() * qp4.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp4.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp4.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp4.results.x - qp_random.l)));
    CHECK(qp4.results.info.rho == T(1.E-7));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp4.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp4.results.info.setup_time
              << " solve time " << qp4.results.info.solve_time << std::endl;

    proxqp::sparse::QP<T, I> qp5(n, n_eq, n_in);
    qp5.settings.eps_abs = eps_abs;
    qp5.settings.initial_guess =
      proxsuite::proxqp::InitialGuessStatus::WARM_START;
    qp5.init(qp_random.H,
             qp_random.g,
             qp_random.A,
             qp_random.b,
             qp_random.C,
             qp_random.l,
             qp_random.u);
    qp5.solve(qp3.results.x, qp3.results.y, qp3.results.z);
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp5.results.x +
      qp_random.g + qp_random.A.transpose() * qp5.results.y +
      qp_random.C.transpose() * qp5.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp5.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp5.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp5.results.x - qp_random.l)));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    qp5.update(nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               nullopt,
               true,
               T(1.E-7));
    qp5.solve();
    dua_res = proxqp::dense::infty_norm(
      qp_random.H.selfadjointView<Eigen::Upper>() * qp5.results.x +
      qp_random.g + qp_random.A.transpose() * qp5.results.y +
      qp_random.C.transpose() * qp5.results.z);
    pri_res = std::max(
      proxqp::dense::infty_norm(qp_random.A * qp5.results.x - qp_random.b),
      proxqp::dense::infty_norm(
        helpers::positive_part(qp_random.C * qp5.results.x - qp_random.u) +
        helpers::negative_part(qp_random.C * qp5.results.x - qp_random.l)));
    CHECK(qp5.results.info.rho == T(1.E-7));
    CHECK(dua_res <= eps_abs);
    CHECK(pri_res <= eps_abs);
    std::cout << "--n = " << n << " n_eq " << n_eq << " n_in " << n_in
              << std::endl;
    std::cout << "; dual residual " << dua_res << "; primal residual "
              << pri_res << std::endl;
    std::cout << "total number of iteration: " << qp5.results.info.iter
              << std::endl;
    std::cout << "setup timing " << qp5.results.info.setup_time
              << " solve time " << qp5.results.info.solve_time << std::endl;
  }
}

DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test changing default settings "
  "after updates using no initial guess")
{
  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test changing default settings after "
               "updates using no initial guess---"
            << std::endl;
  double sparsity_factor = 0.15;
  T eps_abs = T(1e-9);
  utils::rand::set_seed(1);
  dense::isize dim = 10;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  ::proxsuite::proxqp::utils::rand::set_seed(1);
  proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
    dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

  proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                              qp_random.A.cast<bool>(),
                              qp_random.C.cast<bool>());

  T rho(1.e-7);
  T mu_eq(1.e-4);
  bool compute_preconditioner = true;

  qp.settings.initial_guess = proxqp::InitialGuessStatus::NO_INITIAL_GUESS;
  DOCTEST_CHECK(qp.settings.initial_guess ==
                proxqp::InitialGuessStatus::NO_INITIAL_GUESS);
  qp.settings.eps_abs = eps_abs;
  qp.settings.eps_rel = 0;
  qp.init(qp_random.H,
          qp_random.g,
          qp_random.A,
          qp_random.b,
          qp_random.C,
          qp_random.l,
          qp_random.u,
          compute_preconditioner,
          rho);
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);

  T pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  qp.update(nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            compute_preconditioner,
            1.e-6);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
             qp_random.g + qp_random.A.transpose() * qp.results.y +
             qp_random.C.transpose() * qp.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp2(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp2.settings.initial_guess = proxqp::InitialGuessStatus::NO_INITIAL_GUESS;
  DOCTEST_CHECK(qp2.settings.initial_guess ==
                proxqp::InitialGuessStatus::NO_INITIAL_GUESS);
  qp2.settings.eps_abs = eps_abs;
  qp2.settings.eps_rel = 0;

  qp2.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           nullopt,
           mu_eq);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
  qp2.solve();
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
             qp_random.g + qp_random.A.transpose() * qp2.results.y +
             qp_random.C.transpose() * qp2.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp3(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp3.settings.initial_guess = proxqp::InitialGuessStatus::NO_INITIAL_GUESS;
  DOCTEST_CHECK(qp3.settings.initial_guess ==
                proxqp::InitialGuessStatus::NO_INITIAL_GUESS);
  qp3.settings.eps_abs = eps_abs;
  qp3.settings.eps_rel = 0;
  qp3.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           rho,
           mu_eq);
  DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);
  qp3.solve();
  DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
             qp_random.g + qp_random.A.transpose() * qp3.results.y +
             qp_random.C.transpose() * qp3.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
  qp3.update(nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             compute_preconditioner,
             1.e-6,
             1.e-3);
  DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-3 - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e3 - qp3.results.info.mu_eq_inv) <= 1.E-9);
  qp3.solve();
  pri_res = std::max(
    (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
             qp_random.g + qp_random.A.transpose() * qp3.results.y +
             qp_random.C.transpose() * qp3.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
}
DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test changing default settings "
  "after updates using EQUALITY_CONSTRAINED_INITIAL_GUESS")
{
  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test changing default settings after "
               "updates using EQUALITY_CONSTRAINED_INITIAL_GUESS---"
            << std::endl;
  double sparsity_factor = 0.15;
  T eps_abs = T(1e-9);
  utils::rand::set_seed(1);
  dense::isize dim = 10;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  ::proxsuite::proxqp::utils::rand::set_seed(1);
  proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
    dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

  proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                              qp_random.A.cast<bool>(),
                              qp_random.C.cast<bool>());

  T rho(1.e-7);
  T mu_eq(1.e-4);
  bool compute_preconditioner = true;

  qp.settings.initial_guess =
    proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
  DOCTEST_CHECK(qp.settings.initial_guess ==
                proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS);
  qp.settings.eps_abs = eps_abs;
  qp.settings.eps_rel = 0;
  qp.init(qp_random.H,
          qp_random.g,
          qp_random.A,
          qp_random.b,
          qp_random.C,
          qp_random.l,
          qp_random.u,
          compute_preconditioner,
          rho);
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);

  T pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  qp.update(nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            compute_preconditioner,
            1.e-6);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
             qp_random.g + qp_random.A.transpose() * qp.results.y +
             qp_random.C.transpose() * qp.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp2(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp2.settings.initial_guess =
    proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
  DOCTEST_CHECK(qp2.settings.initial_guess ==
                proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS);
  qp2.settings.eps_abs = eps_abs;
  qp2.settings.eps_rel = 0;
  qp2.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           nullopt,
           mu_eq);

  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  qp2.solve();
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
             qp_random.g + qp_random.A.transpose() * qp2.results.y +
             qp_random.C.transpose() * qp2.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp3(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp3.settings.initial_guess =
    proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
  DOCTEST_CHECK(qp3.settings.initial_guess ==
                proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS);
  qp3.settings.eps_abs = eps_abs;
  qp3.settings.eps_rel = 0;
  qp3.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           rho,
           mu_eq);
  DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);
  qp3.solve();
  DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
             qp_random.g + qp_random.A.transpose() * qp3.results.y +
             qp_random.C.transpose() * qp3.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
  qp3.update(nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             compute_preconditioner,
             1.e-6,
             1.e-3);
  DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-3 - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e3 - qp3.results.info.mu_eq_inv) <= 1.E-9);
  qp3.solve();
  pri_res = std::max(
    (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
             qp_random.g + qp_random.A.transpose() * qp3.results.y +
             qp_random.C.transpose() * qp3.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
}
DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test changing default settings "
  "after updates using COLD_START_WITH_PREVIOUS_RESULT")
{
  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test changing default settings after "
               "updates using COLD_START_WITH_PREVIOUS_RESULT---"
            << std::endl;
  double sparsity_factor = 0.15;
  T eps_abs = T(1e-9);
  utils::rand::set_seed(1);
  dense::isize dim = 10;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  ::proxsuite::proxqp::utils::rand::set_seed(1);
  proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
    dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

  proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                              qp_random.A.cast<bool>(),
                              qp_random.C.cast<bool>());

  T rho(1.e-7);
  T mu_eq(1.e-4);
  bool compute_preconditioner = true;

  qp.settings.initial_guess =
    proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp.settings.initial_guess ==
                proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT);
  qp.settings.eps_abs = eps_abs;
  qp.settings.eps_rel = 0;
  qp.init(qp_random.H,
          qp_random.g,
          qp_random.A,
          qp_random.b,
          qp_random.C,
          qp_random.l,
          qp_random.u,
          compute_preconditioner,
          rho);
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);

  T pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  qp.update(nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            compute_preconditioner,
            1.e-6);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
             qp_random.g + qp_random.A.transpose() * qp.results.y +
             qp_random.C.transpose() * qp.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp2(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp2.settings.initial_guess =
    proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp2.settings.initial_guess ==
                proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT);
  qp2.settings.eps_abs = eps_abs;
  qp2.settings.eps_rel = 0;

  qp2.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           nullopt,
           mu_eq);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
  qp2.solve();
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
             qp_random.g + qp_random.A.transpose() * qp2.results.y +
             qp_random.C.transpose() * qp2.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp3(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp3.settings.initial_guess =
    proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp3.settings.initial_guess ==
                proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT);
  qp3.settings.eps_abs = eps_abs;
  qp3.settings.eps_rel = 0;
  qp3.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           rho,
           mu_eq);
  DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);
  qp3.solve();
  DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
             qp_random.g + qp_random.A.transpose() * qp3.results.y +
             qp_random.C.transpose() * qp3.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
  qp3.update(nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             compute_preconditioner,
             1.e-6,
             1.e-3);
  DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-3 - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e3 - qp3.results.info.mu_eq_inv) <= 1.E-9);
  qp3.solve();
  pri_res = std::max(
    (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
             qp_random.g + qp_random.A.transpose() * qp3.results.y +
             qp_random.C.transpose() * qp3.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
}

DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test changing default settings "
  "after updates using WARM_START_WITH_PREVIOUS_RESULT")
{
  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test changing default settings after "
               "updates using WARM_START_WITH_PREVIOUS_RESULT---"
            << std::endl;
  double sparsity_factor = 0.15;
  T eps_abs = T(1e-9);
  utils::rand::set_seed(1);
  dense::isize dim = 10;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  ::proxsuite::proxqp::utils::rand::set_seed(1);
  proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
    dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

  proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                              qp_random.A.cast<bool>(),
                              qp_random.C.cast<bool>());

  T rho(1.e-7);
  T mu_eq(1.e-4);
  bool compute_preconditioner = true;

  qp.settings.initial_guess =
    proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp.settings.initial_guess ==
                proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT);
  qp.settings.eps_abs = eps_abs;
  qp.settings.eps_rel = 0;
  qp.init(qp_random.H,
          qp_random.g,
          qp_random.A,
          qp_random.b,
          qp_random.C,
          qp_random.l,
          qp_random.u,
          compute_preconditioner,
          rho);
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);

  T pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  qp.update(nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            compute_preconditioner,
            1.e-6);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
             qp_random.g + qp_random.A.transpose() * qp.results.y +
             qp_random.C.transpose() * qp.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp2(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp2.settings.initial_guess =
    proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp2.settings.initial_guess ==
                proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT);
  qp2.settings.eps_abs = eps_abs;
  qp2.settings.eps_rel = 0;

  qp2.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           nullopt,
           mu_eq);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
  qp2.solve();
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp2.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp2.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
             qp_random.g + qp_random.A.transpose() * qp2.results.y +
             qp_random.C.transpose() * qp2.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp3(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp3.settings.initial_guess =
    proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp3.settings.initial_guess ==
                proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT);
  qp3.settings.eps_abs = eps_abs;
  qp3.settings.eps_rel = 0;
  qp3.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           rho,
           mu_eq);
  DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);
  qp3.solve();
  DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);

  pri_res = std::max(
    (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
             qp_random.g + qp_random.A.transpose() * qp3.results.y +
             qp_random.C.transpose() * qp3.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
  qp3.update(nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             compute_preconditioner,
             1.e-6,
             1.e-3);
  DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e-3 - qp3.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(1.e3 - qp3.results.info.mu_eq_inv) <= 1.E-9);
  qp3.solve();
  pri_res = std::max(
    (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp3.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp3.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
             qp_random.g + qp_random.A.transpose() * qp3.results.y +
             qp_random.C.transpose() * qp3.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
}

DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test changing default settings "
  "after several solves using no initial guess")
{
  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test changing default settings after "
               "several solves using no initial guess---"
            << std::endl;

  double sparsity_factor = 0.15;
  T eps_abs = T(1e-9);
  utils::rand::set_seed(1);
  dense::isize dim = 10;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  ::proxsuite::proxqp::utils::rand::set_seed(1);
  proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
    dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

  proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                              qp_random.A.cast<bool>(),
                              qp_random.C.cast<bool>());
  // proxqp::sparse::QP<T,I> qp(n,n_eq,n_in);
  qp.settings.eps_abs = eps_abs;

  std::cout << "Test rho update for different initial guess" << std::endl;
  std::cout << "dirty workspace before any solving: " << qp.work.internal.dirty
            << std::endl;

  T rho(1.e-7);
  T mu_eq(1.e-4);
  bool compute_preconditioner = true;

  qp.settings.initial_guess = proxqp::InitialGuessStatus::NO_INITIAL_GUESS;
  DOCTEST_CHECK(qp.settings.initial_guess ==
                proxqp::InitialGuessStatus::NO_INITIAL_GUESS);
  qp.settings.eps_abs = eps_abs;
  qp.settings.eps_rel = 0;
  // qp.settings.verbose = true;
  qp.init(qp_random.H,
          qp_random.g,
          qp_random.A,
          qp_random.b,
          qp_random.C,
          qp_random.l,
          qp_random.u,
          compute_preconditioner,
          rho);
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);

  T pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  for (isize iter = 0; iter < 10; ++iter) {
    qp.solve();
    DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) < 1.e-9);
    DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) < 1.e-9);
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  qp.update(nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            compute_preconditioner,
            1.e-6);
  for (isize iter = 0; iter < 10; ++iter) {
    qp.solve();
    DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) < 1.e-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) < 1.e-9);
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp2(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp2.settings.initial_guess = proxqp::InitialGuessStatus::NO_INITIAL_GUESS;
  DOCTEST_CHECK(qp2.settings.initial_guess ==
                proxqp::InitialGuessStatus::NO_INITIAL_GUESS);
  qp2.settings.eps_abs = eps_abs;
  qp2.settings.eps_rel = 0;
  qp2.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           nullopt,
           mu_eq);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
  qp2.solve();
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);

  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
    qp2.solve();
    DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
               qp_random.g + qp_random.A.transpose() * qp2.results.y +
               qp_random.C.transpose() * qp2.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp3(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp3.settings.initial_guess = proxqp::InitialGuessStatus::NO_INITIAL_GUESS;
  DOCTEST_CHECK(qp3.settings.initial_guess ==
                proxqp::InitialGuessStatus::NO_INITIAL_GUESS);
  qp3.settings.eps_abs = eps_abs;
  qp3.settings.eps_rel = 0;
  qp3.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           rho,
           mu_eq);

  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
    qp3.solve();
    DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
               qp_random.g + qp_random.A.transpose() * qp3.results.y +
               qp_random.C.transpose() * qp3.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  qp3.update(nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             compute_preconditioner,
             1.e-6,
             1.e-3);
  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
    qp3.solve();
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
               qp_random.g + qp_random.A.transpose() * qp3.results.y +
               qp_random.C.transpose() * qp3.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }
}

DOCTEST_TEST_CASE(
  "sparse random strongly convex qp with equality and "
  "inequality constraints: test changing default settings after several solves "
  "using equality constrained initial guess")
{
  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test changing default settings after "
               "several solves using equality constrained initial guess---"
            << std::endl;

  double sparsity_factor = 0.15;
  T eps_abs = T(1e-9);
  utils::rand::set_seed(1);
  dense::isize dim = 10;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  ::proxsuite::proxqp::utils::rand::set_seed(1);
  proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
    dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

  proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                              qp_random.A.cast<bool>(),
                              qp_random.C.cast<bool>());
  // proxqp::sparse::QP<T,I> qp(n,n_eq,n_in);
  qp.settings.eps_abs = eps_abs;

  std::cout << "Test rho update for different initial guess" << std::endl;
  std::cout << "dirty workspace before any solving: " << qp.work.internal.dirty
            << std::endl;

  T rho(1.e-7);
  T mu_eq(1.e-4);
  bool compute_preconditioner = true;

  qp.settings.initial_guess =
    proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
  DOCTEST_CHECK(qp.settings.initial_guess ==
                proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS);
  qp.settings.eps_abs = eps_abs;
  qp.settings.eps_rel = 0;
  // qp.settings.verbose = true;
  qp.init(qp_random.H,
          qp_random.g,
          qp_random.A,
          qp_random.b,
          qp_random.C,
          qp_random.l,
          qp_random.u,
          compute_preconditioner,
          rho);
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);

  T pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  for (isize iter = 0; iter < 10; ++iter) {
    qp.solve();
    DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) < 1.e-9);
    DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) < 1.e-9);
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  qp.update(nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            compute_preconditioner,
            1.e-6);
  for (isize iter = 0; iter < 10; ++iter) {
    qp.solve();
    DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) < 1.e-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) < 1.e-9);
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp2(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp2.settings.initial_guess =
    proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
  DOCTEST_CHECK(qp2.settings.initial_guess ==
                proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS);
  qp2.settings.eps_abs = eps_abs;
  qp2.settings.eps_rel = 0;
  qp2.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           nullopt,
           mu_eq);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
  qp2.solve();
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);

  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
    qp2.solve();
    DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
               qp_random.g + qp_random.A.transpose() * qp2.results.y +
               qp_random.C.transpose() * qp2.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp3(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp3.settings.initial_guess =
    proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS;
  DOCTEST_CHECK(qp3.settings.initial_guess ==
                proxqp::InitialGuessStatus::EQUALITY_CONSTRAINED_INITIAL_GUESS);
  qp3.settings.eps_abs = eps_abs;
  qp3.settings.eps_rel = 0;
  qp3.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           rho,
           mu_eq);

  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
    qp3.solve();
    DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
               qp_random.g + qp_random.A.transpose() * qp3.results.y +
               qp_random.C.transpose() * qp3.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  qp3.update(nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             compute_preconditioner,
             1.e-6,
             1.e-3);
  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
    qp3.solve();
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
               qp_random.g + qp_random.A.transpose() * qp3.results.y +
               qp_random.C.transpose() * qp3.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }
}

DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test changing default settings "
  "after several solves using cold start with previous result")
{
  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test changing default settings after "
               "several solves using cold start with previous result---"
            << std::endl;

  double sparsity_factor = 0.15;
  T eps_abs = T(1e-9);
  utils::rand::set_seed(1);
  dense::isize dim = 10;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  ::proxsuite::proxqp::utils::rand::set_seed(1);
  proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
    dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

  proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                              qp_random.A.cast<bool>(),
                              qp_random.C.cast<bool>());
  // proxqp::sparse::QP<T,I> qp(n,n_eq,n_in);
  qp.settings.eps_abs = eps_abs;

  std::cout << "Test rho update for different initial guess" << std::endl;
  std::cout << "dirty workspace before any solving: " << qp.work.internal.dirty
            << std::endl;

  T rho(1.e-7);
  T mu_eq(1.e-4);
  bool compute_preconditioner = true;

  qp.settings.initial_guess =
    proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp.settings.initial_guess ==
                proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT);
  qp.settings.eps_abs = eps_abs;
  qp.settings.eps_rel = 0;
  // qp.settings.verbose = true;
  qp.init(qp_random.H,
          qp_random.g,
          qp_random.A,
          qp_random.b,
          qp_random.C,
          qp_random.l,
          qp_random.u,
          compute_preconditioner,
          rho);
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);

  T pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  for (isize iter = 0; iter < 10; ++iter) {
    qp.solve();
    DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) < 1.e-9);
    DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) < 1.e-9);
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  qp.update(nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            compute_preconditioner,
            1.e-6);
  for (isize iter = 0; iter < 10; ++iter) {
    qp.solve();
    DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) < 1.e-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) < 1.e-9);
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp2(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp2.settings.initial_guess =
    proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp2.settings.initial_guess ==
                proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT);
  qp2.settings.eps_abs = eps_abs;
  qp2.settings.eps_rel = 0;
  qp2.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           nullopt,
           mu_eq);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
  qp2.solve();
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);

  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
    qp2.solve();
    DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
               qp_random.g + qp_random.A.transpose() * qp2.results.y +
               qp_random.C.transpose() * qp2.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp3(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp3.settings.initial_guess =
    proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp3.settings.initial_guess ==
                proxqp::InitialGuessStatus::COLD_START_WITH_PREVIOUS_RESULT);
  qp3.settings.eps_abs = eps_abs;
  qp3.settings.eps_rel = 0;
  qp3.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           rho,
           mu_eq);

  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);
    qp3.solve();
    DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
               qp_random.g + qp_random.A.transpose() * qp3.results.y +
               qp_random.C.transpose() * qp3.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  qp3.update(nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             compute_preconditioner,
             1.e-6,
             1.e-3);
  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e3 - qp3.results.info.mu_eq_inv) <= 1.E-9);
    qp3.solve();
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e3 - qp3.results.info.mu_eq_inv) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
               qp_random.g + qp_random.A.transpose() * qp3.results.y +
               qp_random.C.transpose() * qp3.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }
}

DOCTEST_TEST_CASE(
  "ProxQP::sparse: sparse random strongly convex qp with equality and "
  "inequality constraints: test changing default settings "
  "after several solves using warm start with previous result")
{
  std::cout << "---testing sparse random strongly convex qp with equality and "
               "inequality constraints: test changing default settings after "
               "several solves using warm start with previous result---"
            << std::endl;

  double sparsity_factor = 0.15;
  T eps_abs = T(1e-9);
  utils::rand::set_seed(1);
  dense::isize dim = 10;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  ::proxsuite::proxqp::utils::rand::set_seed(1);
  proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
    dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

  proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                              qp_random.A.cast<bool>(),
                              qp_random.C.cast<bool>());
  // proxqp::sparse::QP<T,I> qp(n,n_eq,n_in);
  qp.settings.eps_abs = eps_abs;

  std::cout << "Test rho update for different initial guess" << std::endl;
  std::cout << "dirty workspace before any solving: " << qp.work.internal.dirty
            << std::endl;

  T rho(1.e-7);
  T mu_eq(1.e-4);
  bool compute_preconditioner = true;

  qp.settings.initial_guess =
    proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp.settings.initial_guess ==
                proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT);
  qp.settings.eps_abs = eps_abs;
  qp.settings.eps_rel = 0;
  // qp.settings.verbose = true;
  qp.init(qp_random.H,
          qp_random.g,
          qp_random.A,
          qp_random.b,
          qp_random.C,
          qp_random.l,
          qp_random.u,
          compute_preconditioner,
          rho);
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);
  qp.solve();
  DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) <= 1.E-9);
  DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) <= 1.E-9);

  T pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  for (isize iter = 0; iter < 10; ++iter) {
    qp.solve();
    DOCTEST_CHECK(std::abs(rho - qp.settings.default_rho) < 1.e-9);
    DOCTEST_CHECK(std::abs(rho - qp.results.info.rho) < 1.e-9);
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  qp.update(nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            compute_preconditioner,
            1.e-6);
  for (isize iter = 0; iter < 10; ++iter) {
    qp.solve();
    DOCTEST_CHECK(std::abs(1.e-6 - qp.settings.default_rho) < 1.e-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp.results.info.rho) < 1.e-9);
    pri_res = std::max(
      (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp2(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp2.settings.initial_guess =
    proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp2.settings.initial_guess ==
                proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT);
  qp2.settings.eps_abs = eps_abs;
  qp2.settings.eps_rel = 0;
  qp2.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           nullopt,
           mu_eq);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
  qp2.solve();
  DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
  DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);

  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
    qp2.solve();
    DOCTEST_CHECK(std::abs(mu_eq - qp2.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp2.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp2.results.info.mu_eq_inv) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp2.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp2.results.x +
               qp_random.g + qp_random.A.transpose() * qp2.results.y +
               qp_random.C.transpose() * qp2.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  // conter factual check with another QP object starting at the updated model
  proxqp::sparse::QP<T, I> qp3(qp_random.H.cast<bool>(),
                               qp_random.A.cast<bool>(),
                               qp_random.C.cast<bool>());
  qp3.settings.initial_guess =
    proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT;
  DOCTEST_CHECK(qp3.settings.initial_guess ==
                proxqp::InitialGuessStatus::WARM_START_WITH_PREVIOUS_RESULT);
  qp3.settings.eps_abs = eps_abs;
  qp3.settings.eps_rel = 0;
  qp3.init(qp_random.H,
           qp_random.g,
           qp_random.A,
           qp_random.b,
           qp_random.C,
           qp_random.l,
           qp_random.u,
           compute_preconditioner,
           rho,
           mu_eq);

  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);
    qp3.solve();
    DOCTEST_CHECK(std::abs(rho - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(rho - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(mu_eq - qp3.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(T(1) / mu_eq - qp3.results.info.mu_eq_inv) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
               qp_random.g + qp_random.A.transpose() * qp3.results.y +
               qp_random.C.transpose() * qp3.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }

  qp3.update(nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             nullopt,
             compute_preconditioner,
             1.e-6,
             1.e-3);
  for (isize iter = 0; iter < 10; ++iter) {
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e3 - qp3.results.info.mu_eq_inv) <= 1.E-9);
    qp3.solve();
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.settings.default_rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-6 - qp3.results.info.rho) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.settings.default_mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e-3 - qp3.results.info.mu_eq) <= 1.E-9);
    DOCTEST_CHECK(std::abs(1.e3 - qp3.results.info.mu_eq_inv) <= 1.E-9);
    pri_res = std::max(
      (qp_random.A * qp3.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
      (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
       helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
        .lpNorm<Eigen::Infinity>());
    dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp3.results.x +
               qp_random.g + qp_random.A.transpose() * qp3.results.y +
               qp_random.C.transpose() * qp3.results.z)
                .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= eps_abs);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }
}

TEST_CASE("ProxQP::sparse: init must be called before update")
{
  double sparsity_factor = 0.15;
  T eps_abs = T(1e-9);
  utils::rand::set_seed(1);
  dense::isize dim = 10;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  ::proxsuite::proxqp::utils::rand::set_seed(1);
  proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
    dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

  proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                              qp_random.A.cast<bool>(),
                              qp_random.C.cast<bool>());
  qp.settings.eps_abs = eps_abs;

  qp.settings.eps_abs = eps_abs;
  qp.settings.eps_rel = 0;
  qp.settings.initial_guess = InitialGuessStatus::NO_INITIAL_GUESS;

  // call update without init, update calls init internally
  qp.update(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u,
            true);

  qp.solve();

  T pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
               qp_random.g + qp_random.A.transpose() * qp.results.y +
               qp_random.C.transpose() * qp.results.z)
                .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);

  qp_random.H = 2 * qp_random.H; // keep same sparsity structure
  qp_random.g = utils::rand::vector_rand<T>(dim);
  qp.update(qp_random.H,
            qp_random.g,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            nullopt,
            true);

  qp.solve();

  pri_res = std::max(
    (qp_random.A * qp.results.x - qp_random.b).lpNorm<Eigen::Infinity>(),
    (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
     helpers::negative_part(qp_random.C * qp.results.x - qp_random.l))
      .lpNorm<Eigen::Infinity>());
  dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
             qp_random.g + qp_random.A.transpose() * qp.results.y +
             qp_random.C.transpose() * qp.results.z)
              .lpNorm<Eigen::Infinity>();
  DOCTEST_CHECK(pri_res <= eps_abs);
  DOCTEST_CHECK(dua_res <= eps_abs);
}

TEST_CASE("ProxQP::sparse: test primal infeasibility solving")
{
  double sparsity_factor = 0.15;
  T eps_abs = T(1e-5);
  utils::rand::set_seed(1);
  dense::isize dim = 20;

  dense::isize n_eq(dim / 4);
  dense::isize n_in(dim / 4);
  T strong_convexity_factor(1.e-2);
  for (isize i = 0; i < 20; ++i) {
    ::proxsuite::proxqp::utils::rand::set_seed(i);
    proxqp::sparse::SparseModel<T> qp_random = utils::sparse_strongly_convex_qp(
      dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    proxqp::sparse::QP<T, I> qp(qp_random.H.cast<bool>(),
                                qp_random.A.cast<bool>(),
                                qp_random.C.cast<bool>());
    qp.settings.eps_abs = eps_abs;
    qp.settings.eps_rel = 0;
    // create infeasible problem
    qp_random.b.array() += T(10.);
    qp_random.u.array() -= T(100.);
    qp.settings.initial_guess = InitialGuessStatus::NO_INITIAL_GUESS;
    qp.settings.primal_infeasibility_solving = true;
    qp.settings.eps_primal_inf = T(1.E-4);
    qp.settings.eps_dual_inf = T(1.E-4);
    qp.init(qp_random.H,
            qp_random.g,
            qp_random.A,
            qp_random.b,
            qp_random.C,
            qp_random.l,
            qp_random.u);
    qp.solve();

    Vec<T> rhs_dim(dim);
    Vec<T> rhs_n_eq(n_eq);
    rhs_n_eq.setOnes();
    Vec<T> rhs_n_in(n_in);
    rhs_n_in.setOnes();
    rhs_dim.noalias() =
      qp_random.A.transpose() * rhs_n_eq + qp_random.C.transpose() * rhs_n_in;
    // detail::noalias_gevmmv_add(Adx, rhs_dim, AT_scaled, dx, rhs_n_eq);
    T scaled_eps = (rhs_dim).lpNorm<Eigen::Infinity>() * eps_abs;

    T pri_res =
      (qp_random.A.transpose() * (qp_random.A * qp.results.x - qp_random.b) +
       qp_random.C.transpose() *
         (helpers::positive_part(qp_random.C * qp.results.x - qp_random.u) +
          helpers::negative_part(qp_random.C * qp.results.x - qp_random.l)))
        .lpNorm<Eigen::Infinity>();
    T dua_res = (qp_random.H.selfadjointView<Eigen::Upper>() * qp.results.x +
                 qp_random.g + qp_random.A.transpose() * qp.results.y +
                 qp_random.C.transpose() * qp.results.z)
                  .lpNorm<Eigen::Infinity>();
    DOCTEST_CHECK(pri_res <= scaled_eps);
    DOCTEST_CHECK(dua_res <= eps_abs);
  }
}
// TEST_CASE("ProxQP::sparse: estimate of minimal eigenvalues using Eigen")
// {
//   double sparsity_factor = 0.25;
//   T tol = T(1e-6);
//   utils::rand::set_seed(1);
//   dense::isize dim = 2;
//   dense::isize n_eq(dim);
//   dense::isize n_in(dim);
//   T strong_convexity_factor(1.e-2);
//   dim = 50;
//   n_eq = dim;
//   n_in = dim;
//   for (isize i = 0; i < 20; ++i) {
//     ::proxsuite::proxqp::utils::rand::set_seed(i);
//     proxqp::dense::Model<T> qp_random =
//     proxqp::utils::dense_strongly_convex_qp(
//       dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);
//     // proxqp::sparse::SparseModel<T> qp_random =
//     // utils::sparse_strongly_convex_qp(
//     //   dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

//     qp_random.H.setZero();
//     dense::Vec<T> random_diag = proxqp::utils::rand::vector_rand<T>(dim);
//     qp_random.H.diagonal().array() += random_diag.array();
//     T minimal_eigenvalue = qp_random.H.diagonal().minCoeff();

//     proxqp::sparse::QP<T, I> qp(dim, n_eq, n_in);
//     qp.settings.max_iter = 1;
//     qp.settings.max_iter_in = 1;
//     qp.settings.estimate_method_option =
//       EigenValueEstimateMethodOption::EigenRegularization;
//     qp.settings.initial_guess = InitialGuessStatus::NO_INITIAL_GUESS;
//     SparseMat<T> H_sparse = qp_random.H.sparseView();
//     SparseMat<T> A_sparse = qp_random.A.sparseView();
//     SparseMat<T> C_sparse = qp_random.C.sparseView();
//     qp.init(H_sparse,
//             qp_random.g,
//             A_sparse,
//             qp_random.b,
//             C_sparse,
//             qp_random.l,
//             qp_random.u);
//     DOCTEST_CHECK(std::abs(qp.results.info.minimal_H_eigenvalue_estimate -
//                            minimal_eigenvalue) <= tol);
//   }
//   dim = 50;
//   n_eq = dim;
//   n_in = dim;
//   for (isize i = 0; i < 20; ++i) {
//     ::proxsuite::proxqp::utils::rand::set_seed(i);
//     proxqp::dense::Model<T> qp_random =
//     proxqp::utils::dense_strongly_convex_qp(
//       dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

//     dense::Vec<T> random_diag = proxqp::utils::rand::vector_rand<T>(dim);
//     qp_random.H.diagonal().array() += 100 * random_diag.array();

//     proxqp::sparse::QP<T, I> qp(dim, n_eq, n_in);
//     qp.settings.max_iter = 1;
//     qp.settings.max_iter_in = 1;
//     qp.settings.estimate_method_option =
//       EigenValueEstimateMethodOption::EigenRegularization;
//     qp.settings.initial_guess = InitialGuessStatus::NO_INITIAL_GUESS;
//     SparseMat<T> H_sparse = qp_random.H.sparseView();
//     SparseMat<T> A_sparse = qp_random.A.sparseView();
//     SparseMat<T> C_sparse = qp_random.C.sparseView();
//     Eigen::SelfAdjointEigenSolver<SparseMat<T>> es(H_sparse,
//                                                    Eigen::EigenvaluesOnly);
//     T minimal_eigenvalue = T(es.eigenvalues().minCoeff());
//     qp.init(H_sparse,
//             qp_random.g,
//             A_sparse,
//             qp_random.b,
//             C_sparse,
//             qp_random.l,
//             qp_random.u);
//     DOCTEST_CHECK(std::abs(qp.results.info.minimal_H_eigenvalue_estimate -
//                            minimal_eigenvalue) <= 1.);
//   }
// }
TEST_CASE("ProxQP::sparse: estimate of minimal eigenvalues using manual choice")
{
  double sparsity_factor = 0.25;
  T tol = T(1e-6);
  utils::rand::set_seed(1);
  dense::isize dim = 2;
  dense::isize n_eq(dim);
  dense::isize n_in(dim);
  T strong_convexity_factor(1.e-2);
  dim = 50;
  n_eq = dim;
  n_in = dim;
  for (isize i = 0; i < 20; ++i) {
    ::proxsuite::proxqp::utils::rand::set_seed(i);
    proxqp::dense::Model<T> qp_random = proxqp::utils::dense_strongly_convex_qp(
      dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);
    // proxqp::sparse::SparseModel<T> qp_random =
    // utils::sparse_strongly_convex_qp(
    //   dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    qp_random.H.setZero();
    dense::Vec<T> random_diag = proxqp::utils::rand::vector_rand<T>(dim);
    qp_random.H.diagonal().array() += random_diag.array();
    T minimal_eigenvalue = qp_random.H.diagonal().minCoeff();

    proxqp::sparse::QP<T, I> qp(dim, n_eq, n_in);
    qp.settings.max_iter = 1;
    qp.settings.max_iter_in = 1;
    qp.settings.initial_guess = InitialGuessStatus::NO_INITIAL_GUESS;
    SparseMat<T> H_sparse = qp_random.H.sparseView();
    SparseMat<T> A_sparse = qp_random.A.sparseView();
    SparseMat<T> C_sparse = qp_random.C.sparseView();
    qp.init(H_sparse,
            qp_random.g,
            A_sparse,
            qp_random.b,
            C_sparse,
            qp_random.l,
            qp_random.u,
            true,
            nullopt,
            nullopt,
            nullopt,
            minimal_eigenvalue);
    DOCTEST_CHECK(std::abs(qp.results.info.minimal_H_eigenvalue_estimate -
                           minimal_eigenvalue) <= tol);
  }
  dim = 50;
  n_eq = dim;
  n_in = dim;
  for (isize i = 0; i < 20; ++i) {
    ::proxsuite::proxqp::utils::rand::set_seed(i);
    proxqp::dense::Model<T> qp_random = proxqp::utils::dense_strongly_convex_qp(
      dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    dense::Vec<T> random_diag = proxqp::utils::rand::vector_rand<T>(dim);
    qp_random.H.diagonal().array() += 100 * random_diag.array();

    proxqp::sparse::QP<T, I> qp(dim, n_eq, n_in);
    qp.settings.max_iter = 1;
    qp.settings.max_iter_in = 1;
    qp.settings.initial_guess = InitialGuessStatus::NO_INITIAL_GUESS;
    SparseMat<T> H_sparse = qp_random.H.sparseView();
    SparseMat<T> A_sparse = qp_random.A.sparseView();
    SparseMat<T> C_sparse = qp_random.C.sparseView();
    Eigen::SelfAdjointEigenSolver<dense::Mat<T>> es(qp_random.H,
                                                    Eigen::EigenvaluesOnly);
    T minimal_eigenvalue = T(es.eigenvalues().minCoeff());
    qp.init(H_sparse,
            qp_random.g,
            A_sparse,
            qp_random.b,
            C_sparse,
            qp_random.l,
            qp_random.u,
            true,
            nullopt,
            nullopt,
            nullopt,
            minimal_eigenvalue);
    DOCTEST_CHECK(std::abs(qp.results.info.minimal_H_eigenvalue_estimate -
                           minimal_eigenvalue) <= 1.);
  }
}

TEST_CASE(
  "ProxQP::sparse: estimate of minimal eigenvalues using Power iteration")
{
  double sparsity_factor = 0.25;
  T tol = T(1e-6);
  utils::rand::set_seed(1);
  dense::isize dim = 2;
  dense::isize n_eq(dim);
  dense::isize n_in(dim);
  T strong_convexity_factor(1.e-2);
  dim = 50;
  n_eq = dim;
  n_in = dim;
  for (isize i = 0; i < 20; ++i) {
    ::proxsuite::proxqp::utils::rand::set_seed(i);
    proxqp::dense::Model<T> qp_random = proxqp::utils::dense_strongly_convex_qp(
      dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);
    // proxqp::sparse::SparseModel<T> qp_random =
    // utils::sparse_strongly_convex_qp(
    //   dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    qp_random.H.setZero();
    dense::Vec<T> random_diag = proxqp::utils::rand::vector_rand<T>(dim);
    qp_random.H.diagonal().array() += random_diag.array();
    T minimal_eigenvalue = qp_random.H.diagonal().minCoeff();
    // std::cout << "qp_random.H" << std::endl;
    // std::cout << qp_random.H << std::endl;
    // std::cout << "minimal_eigenvalue " << minimal_eigenvalue << std::endl;
    proxqp::sparse::QP<T, I> qp(dim, n_eq, n_in);
    qp.settings.max_iter = 1;
    qp.settings.max_iter_in = 1;
    qp.settings.initial_guess = InitialGuessStatus::NO_INITIAL_GUESS;
    SparseMat<T> H_sparse = qp_random.H.sparseView();
    SparseMat<T> A_sparse = qp_random.A.sparseView();
    SparseMat<T> C_sparse = qp_random.C.sparseView();
    const T estimate_minimal_eigen_value =
      sparse::estimate_minimal_eigen_value_of_symmetric_matrix<T, I>(
        H_sparse, 1.E-6, 10000);
    qp.init(H_sparse,
            qp_random.g,
            A_sparse,
            qp_random.b,
            C_sparse,
            qp_random.l,
            qp_random.u,
            true,
            nullopt,
            nullopt,
            nullopt,
            estimate_minimal_eigen_value);
    DOCTEST_CHECK(std::abs(qp.results.info.minimal_H_eigenvalue_estimate -
                           minimal_eigenvalue) <= tol);
  }
  dim = 50;
  n_eq = dim;
  n_in = dim;
  for (isize i = 0; i < 20; ++i) {
    ::proxsuite::proxqp::utils::rand::set_seed(i);
    proxqp::dense::Model<T> qp_random = proxqp::utils::dense_strongly_convex_qp(
      dim, n_eq, n_in, sparsity_factor, strong_convexity_factor);

    dense::Vec<T> random_diag = proxqp::utils::rand::vector_rand<T>(dim);
    qp_random.H.diagonal().array() += 100 * random_diag.array();

    proxqp::sparse::QP<T, I> qp(dim, n_eq, n_in);
    qp.settings.max_iter = 1;
    qp.settings.max_iter_in = 1;
    qp.settings.initial_guess = InitialGuessStatus::NO_INITIAL_GUESS;
    SparseMat<T> H_sparse = qp_random.H.sparseView();
    SparseMat<T> A_sparse = qp_random.A.sparseView();
    SparseMat<T> C_sparse = qp_random.C.sparseView();
    T estimate_minimal_eigen_value =
      sparse::estimate_minimal_eigen_value_of_symmetric_matrix<T, I>(
        H_sparse, 1.E-6, 10000);
    Eigen::SelfAdjointEigenSolver<dense::Mat<T>> es(qp_random.H,
                                                    Eigen::EigenvaluesOnly);
    const T minimal_eigenvalue = T(es.eigenvalues().minCoeff());
    qp.init(H_sparse,
            qp_random.g,
            A_sparse,
            qp_random.b,
            C_sparse,
            qp_random.l,
            qp_random.u,
            true,
            nullopt,
            nullopt,
            nullopt,
            estimate_minimal_eigen_value);
    DOCTEST_CHECK(std::abs(qp.results.info.minimal_H_eigenvalue_estimate -
                           minimal_eigenvalue) <= 1.);
  }
}
