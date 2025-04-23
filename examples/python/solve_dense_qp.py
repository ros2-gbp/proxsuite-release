import proxsuite
import numpy as np
import scipy.sparse as spa


def generate_mixed_qp(n, seed=1):
    # A function for generating sparse random convex qps in dense format

    np.random.seed(seed)
    n_eq = int(n / 4)
    n_in = int(n / 4)
    m = n_eq + n_in

    P = spa.random(
        n, n, density=0.075, data_rvs=np.random.randn, format="csc"
    ).toarray()
    P = (P + P.T) / 2.0

    s = max(np.absolute(np.linalg.eigvals(P)))
    P += (abs(s) + 1e-02) * spa.eye(n)
    P = spa.coo_matrix(P)
    q = np.random.randn(n)
    A = spa.random(m, n, density=0.15, data_rvs=np.random.randn, format="csc").toarray()
    v = np.random.randn(n)  # Fictitious solution
    delta = np.random.rand(m)  # To get inequality
    u = A @ v
    l = -1.0e20 * np.ones(m)

    return P.toarray(), q, A[:n_eq, :], u[:n_eq], A[n_in:, :], u[n_in:], l[n_in:]


# load a qp object using qp problem dimensions
n = 10
n_eq = 2
n_in = 2
qp = proxsuite.proxqp.dense.QP(n, n_eq, n_in)
# generate a random QP
H, g, A, b, C, u, l = generate_mixed_qp(n)
# initialize the model of the problem to solve
qp.init(H, g, A, b, C, l, u)
# solve without warm start
qp.solve()
# solve with a warm start, for ex random one
qp.solve(np.random.randn(n), np.random.randn(n_eq), np.random.randn(n_in))
# print an optimal solution
print("optimal x: {}".format(qp.results.x))
print("optimal y: {}".format(qp.results.y))
print("optimal z: {}".format(qp.results.z))
# Another example if you have box constraints (for the dense backend only for the moment)
qp2 = proxsuite.proxqp.dense.QP(n, n_eq, n_in, True)
l_box = -np.ones(n) * 1.0e10
u_box = np.ones(n) * 1.0e10
qp2.init(H, g, A, b, C, l, u, l_box, u_box)
qp2.solve()
# An important note regarding the inequality multipliers
z_ineq = qp.results.z[:n_in]  # contains the multiplier associated to qp_random.C
z_box = qp.results.z[
    n_in:
]  # the last dim elements correspond to multiplier associated to
# the box constraints
