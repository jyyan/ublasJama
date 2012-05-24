/** Eigenvalues and eigenvectors of a real matrix. 
<P>
    If A is symmetric, then A = V*D*V' where the eigenvalue matrix D is
    diagonal and the eigenvector matrix V is orthogonal. That is,
    the diagonal values of D are the eigenvalues, and
    V*V' = I, where I is the identity matrix.  The columns of V 
    represent the eigenvectors in the sense that A*V = V*D.
<P>
    If A is not symmetric, then the eigenvalue matrix D is block diagonal
    with the real eigenvalues in 1-by-1 blocks and any complex eigenvalues,
    a + i*b, in 2-by-2 blocks, [a, b; -b, a].  That is, if the complex
    eigenvalues look like
<pre>

          u + iv     .        .          .      .    .
            .      u - iv     .          .      .    .
            .        .      a + ib       .      .    .
            .        .        .        a - ib   .    .
            .        .        .          .      x    .
            .        .        .          .      .    y
</pre>
        then D looks like
<pre>

            u        v        .          .      .    .
           -v        u        .          .      .    . 
            .        .        a          b      .    .
            .        .       -b          a      .    .
            .        .        .          .      x    .
            .        .        .          .      .    y
</pre>
    This keeps V a real matrix in both symmetric and non-symmetric
    cases, and A*V = V*D.
    
    
    
    <p>
    The matrix V may be badly
    conditioned, or even singular, so the validity of the equation
    A = V*D*inverse(V) depends upon the condition number of V.
**/

#ifndef _BOOST_UBLAS_EIGENVALUEDECOMPOSITION_
#define _BOOST_UBLAS_EIGENVALUEDECOMPOSITION_

#include <algorithm>
#include <cmath>
#include <limits>
#include <complex>
#include <boost/math/special_functions/hypot.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/symmetric.hpp>
#include <boost/numeric/ublas/exception.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

namespace boost { namespace numeric { namespace ublas {
// T: type, TRI: type of triangular matrix (lower/upper), L: layout (row_major/column_major)
template<class T, class TRI = lower, class L = row_major>
class EigenvalueDecomposition {

    typedef vector<T> Vector;
    typedef matrix<T,L> Matrix;
    typedef symmetric_matrix<T,TRI,L> SymmetricMatrix;

/* ------------------------
   Class variables
 * ------------------------ */

   /** Row and column dimension (square matrix).
   @serial matrix dimension.
   */
   int n;

   /** Symmetry flag.
   @serial internal symmetry flag.
   */
   bool issymmetric;

   /** Arrays for internal storage of eigenvalues.
   @serial internal storage of eigenvalues.
   */
   Vector d, e;

   /** Array for internal storage of eigenvectors.
   @serial internal storage of eigenvectors.
   */
   Matrix V;

   /** Array for internal storage of nonsymmetric Hessenberg form.
   @serial internal storage of nonsymmetric Hessenberg form.
   */
   Matrix H;

   /** Working storage for nonsymmetric algorithm.
   @serial working storage for nonsymmetric algorithm.
   */
   Vector ort;

/* ------------------------
   Private Methods
 * ------------------------ */

   // Symmetric Householder reduction to tridiagonal form.

   void tred2 ();

   // Symmetric tridiagonal QL algorithm.
   
   void tql2 ();

   // Nonsymmetric reduction to Hessenberg form.

   void orthes ();

   // Nonsymmetric reduction from Hessenberg to real Schur form.

   void hqr2 ();

public:
/* ------------------------
   Constructor
 * ------------------------ */

   /** Check for symmetry, then construct the eigenvalue decomposition
   @param A    Square matrix
   @return     Structure to access D and V.
   */

   EigenvalueDecomposition (const Matrix& A);
   EigenvalueDecomposition (const SymmetricMatrix& A);

/* ------------------------
   Public Methods
 * ------------------------ */

   /** Is the matrix symmetric?
   @return     true if A is symmetric.
   */
   bool isSymmetric() const {
       return issymmetric;
   }
    
   /** Return the eigenvector matrix
   @return     V
   */

   const Matrix& getV () const {
      return V;
   }

   /** Return the real parts of the eigenvalues
       If A is symmetric, the eigenvalues are in ascending order.
       If A is not symmetric, the eigenvalues are unordered except that
       complex conjugate pairs of values appear consecutively with the
       eigenvalue having the positive imaginary part first.
   @return     real(diag(D))
   */

   const Vector& getRealEigenvalues () const {
      return d;
   }

   /** Return the imaginary parts of the eigenvalues
       If A is not symmetric, the eigenvalues are unordered except that
       complex conjugate pairs of values appear consecutively with the
       eigenvalue having the positive imaginary part first.
   @return     imag(diag(D))
   */

   const Vector& getImagEigenvalues () const {
      return e;
   }

   /** Return the block diagonal eigenvalue matrix
   @param     D
   */

   void getD (Matrix &D) const;
   const Matrix getD () const {
      Matrix D;
      getD(D);
      return D;
   }
};

   /** sqrt(a^2 + b^2) without under/overflow. **/
   /*
   static inline T hypot(T a, T b) {
      T r;
      if (std::abs(a) > std::abs(b)) {
         r = b/a;
         r = std::abs(a)*std::sqrt(1+r*r);
      } else if (b != 0) {
         r = a/b;
         r = std::abs(b)*std::sqrt(1+r*r);
      } else {
         r = 0.0;
      }
      return r;
   }
   */
    
    
/* ------------------------
   Private Methods
 * ------------------------ */

   // Symmetric Householder reduction to tridiagonal form.

template<class T, class TRI, class L>
void EigenvalueDecomposition<T,TRI,L>::tred2 () {

   //  This is derived from the Algol procedures tred2 by
   //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
   //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
   //  Fortran subroutine in EISPACK.

      for (int j = 0; j < n; ++j) {
         d(j) = V(n-1,j);
      }

      // Householder reduction to tridiagonal form.
   
      for (int i = n-1; i > 0; --i) {
   
         // Scale to avoid under/overflow.
   
         T scale = 0.0;
         T h = 0.0;
         for (int k = 0; k < i; ++k) {
            scale = scale + std::abs(d(k));
         }
         if (scale == 0.0) {
            e(i) = d(i-1);
            for (int j = 0; j < i; ++j) {
               d(j) = V(i-1,j);
               V(i,j) = 0.0;
               V(j,i) = 0.0;
            }
         } else {
   
            // Generate Householder vector.
   
            for (int k = 0; k < i; ++k) {
               d(k) /= scale;
               h += d(k) * d(k);
            }
            T f = d(i-1);
            T g = std::sqrt(h);
            if (f > 0) {
               g = -g;
            }
            e(i) = scale * g;
            h -= f * g;
            d(i-1) = f - g;
            for (int j = 0; j < i; ++j) {
               e(j) = 0.0;
            }
   
            // Apply similarity transformation to remaining columns.
   
            for (int j = 0; j < i; ++j) {
               f = d(j);
               V(j,i) = f;
               g = e(j) + V(j,j) * f;
               for (int k = j+1; k <= i-1; ++k) {
                  g += V(k,j) * d(k);
                  e(k) += V(k,j) * f;
               }
               e(j) = g;
            }
            f = 0.0;
            for (int j = 0; j < i; ++j) {
               e(j) /= h;
               f += e(j) * d(j);
            }
            T hh = f / (h + h);
            for (int j = 0; j < i; ++j) {
               e(j) -= hh * d(j);
            }
            for (int j = 0; j < i; ++j) {
               f = d(j);
               g = e(j);
               for (int k = j; k <= i-1; ++k) {
                  V(k,j) -= (f * e(k) + g * d(k));
               }
               d(j) = V(i-1,j);
               V(i,j) = 0.0;
            }
         }
         d(i) = h;
      }
   
      // Accumulate transformations.
   
      for (int i = 0; i < n-1; ++i) {
         V(n-1,i) = V(i,i);
         V(i,i) = 1.0;
         T h = d(i+1);
         if (h != 0.0) {
            for (int k = 0; k <= i; ++k) {
               d(k) = V(k,i+1) / h;
            }
            for (int j = 0; j <= i; ++j) {
               T g = 0.0;
               for (int k = 0; k <= i; ++k) {
                  g += V(k,i+1) * V(k,j);
               }
               for (int k = 0; k <= i; ++k) {
                  V(k,j) -= g * d(k);
               }
            }
         }
         for (int k = 0; k <= i; ++k) {
            V(k,i+1) = 0.0;
         }
      }
      for (int j = 0; j < n; ++j) {
         d(j) = V(n-1,j);
         V(n-1,j) = 0.0;
      }
      V(n-1,n-1) = 1.0;
      e(0) = 0.0;
   } 

   // Symmetric tridiagonal QL algorithm.
   
template<class T, class TRI, class L>
void EigenvalueDecomposition<T,TRI,L>::tql2 () {

   //  This is derived from the Algol procedures tql2, by
   //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
   //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
   //  Fortran subroutine in EISPACK.
   
      for (int i = 1; i < n; ++i) {
         e(i-1) = e(i);
      }
      e(n-1) = 0.0;
   
      T f = 0.0;
      T tst1 = 0.0;
      T eps = std::numeric_limits<T>::epsilon();
      for (int l = 0; l < n; ++l) {

         // Find small subdiagonal element
   
         tst1 = std::max(tst1,std::abs(d(l)) + std::abs(e(l)));
         int m = l;
         while (m < n) {
            if (std::abs(e(m)) <= eps*tst1) {
               break;
            }
            ++m;
         }
   
         // If m == l, d(l) is an eigenvalue,
         // otherwise, iterate.
   
         if (m > l) {
            int iter = 0;
            do {
               ++iter;  // (Could check iteration count here.)
   
               // Compute implicit shift
   
               T g = d(l);
               T p = (d(l+1) - g) / (2.0 * e(l));
                T r = boost::math::hypot(p,1.0);
               if (p < 0) {
                  r = -r;
               }
               d(l) = e(l) / (p + r);
               d(l+1) = e(l) * (p + r);
               T dl1 = d(l+1);
               T h = g - d(l);
               for (int i = l+2; i < n; ++i) {
                  d(i) -= h;
               }
               f += h;
   
               // Implicit QL transformation.
   
               p = d(m);
               T c = 1.0;
               T c2 = c;
               T c3 = c;
               T el1 = e(l+1);
               T s = 0.0;
               T s2 = 0.0;
               for (int i = m-1; i >= l; --i) {
                  c3 = c2;
                  c2 = c;
                  s2 = s;
                  g = c * e(i);
                  h = c * p;
                   r = boost::math::hypot(p,e(i));
                  e(i+1) = s * r;
                  s = e(i) / r;
                  c = p / r;
                  p = c * d(i) - s * g;
                  d(i+1) = h + s * (c * g + s * d(i));
   
                  // Accumulate transformation.
   
                  for (int k = 0; k < n; ++k) {
                     h = V(k,i+1);
                     V(k,i+1) = s * V(k,i) + c * h;
                     V(k,i) = c * V(k,i) - s * h;
                  }
               }
               p = -s * s2 * c3 * el1 * e(l) / dl1;
               e(l) = s * p;
               d(l) = c * p;
   
               // Check for convergence.
   
            } while (std::abs(e(l)) > eps*tst1);
         }
         d(l) += f;
         e(l) = 0.0;
      }
     
      // Sort eigenvalues and corresponding vectors.
   
      for (int i = 0; i < n-1; ++i) {
         int k = i;
         T p = d(i);
         for (int j = i+1; j < n; ++j) {
            if (d(j) < p) {
               k = j;
               p = d(j);
            }
         }
         if (k != i) {
            d(k) = d(i);
            d(i) = p;
            for (int j = 0; j < n; ++j) {
               p = V(j,i);
               V(j,i) = V(j,k);
               V(j,k) = p;
            }
         }
      }
   }

   // Nonsymmetric reduction to Hessenberg form.

template<class T, class TRI, class L>
void EigenvalueDecomposition<T,TRI,L>::orthes () {
   
      //  This is derived from the Algol procedures orthes and ortran,
      //  by Martin and Wilkinson, Handbook for Auto. Comp.,
      //  Vol.ii-Linear Algebra, and the corresponding
      //  Fortran subroutines in EISPACK.
   
      int low = 0;
      int high = n-1;
   
      for (int m = low+1; m <= high-1; ++m) {
   
         // Scale column.
   
         T scale = 0.0;
         for (int i = m; i <= high; ++i) {
            scale += std::abs(H(i,m-1));
         }
         if (scale != 0.0) {
   
            // Compute Householder transformation.
   
            T h = 0.0;
            for (int i = high; i >= m; --i) {
               ort(i) = H(i,m-1)/scale;
               h += ort(i) * ort(i);
            }
            T g = std::sqrt(h);
            if (ort(m) > 0) {
               g = -g;
            }
            h = h - ort(m) * g;
            ort(m) = ort(m) - g;
   
            // Apply Householder similarity transformation
            // H = (I-u*u'/h)*H*(I-u*u')/h)
   
            for (int j = m; j < n; ++j) {
               T f = 0.0;
               for (int i = high; i >= m; --i) {
                  f += ort(i)*H(i,j);
               }
               f = f/h;
               for (int i = m; i <= high; ++i) {
                  H(i,j) -= f*ort(i);
               }
           }
   
           for (int i = 0; i <= high; ++i) {
               T f = 0.0;
               for (int j = high; j >= m; --j) {
                  f += ort(j)*H(i,j);
               }
               f = f/h;
               for (int j = m; j <= high; ++j) {
                  H(i,j) -= f*ort(j);
               }
            }
            ort(m) = scale*ort(m);
            H(m,m-1) = scale*g;
         }
      }
   
      // Accumulate transformations (Algol's ortran).

      //for (int i = 0; i < n; ++i) {
      //   for (int j = 0; j < n; ++j) {
      //      V(i,j) = (i == j ? 1.0 : 0.0);
      //   }
      //}
      V = identity_matrix<T>(n);

      for (int m = high-1; m >= low+1; --m) {
         if (H(m,m-1) != 0.0) {
            for (int i = m+1; i <= high; ++i) {
               ort(i) = H(i,m-1);
            }
            for (int j = m; j <= high; ++j) {
               T g = 0.0;
               for (int i = m; i <= high; ++i) {
                  g += ort(i) * V(i,j);
               }
               // T division avoids possible underflow
               g = (g / ort(m)) / H(m,m-1);
               for (int i = m; i <= high; ++i) {
                  V(i,j) += g * ort(i);
               }
            }
         }
      }
   }


   // Nonsymmetric reduction from Hessenberg to real Schur form.

template<class T, class TRI, class L>
void EigenvalueDecomposition<T,TRI,L>::hqr2 () {
   
      //  This is derived from the Algol procedure hqr2,
      //  by Martin and Wilkinson, Handbook for Auto. Comp.,
      //  Vol.ii-Linear Algebra, and the corresponding
      //  Fortran subroutine in EISPACK.
   
      // Initialize
   
      int nn = this->n;
      int n = nn-1;
      int low = 0;
      int high = nn-1;
      T eps = std::numeric_limits<T>::epsilon();
      T exshift = 0.0;
      T p=0,q=0,r=0,s=0,z=0,t,w,x,y;
   
      // Store roots isolated by balanc and compute matrix norm
   
      T norm = 0.0;
      for (int i = 0; i < nn; ++i) {
         if (i < low || i > high) {
            d(i) = H(i,i);
            e(i) = 0.0;
         }
         for (int j = std::max(i-1,0); j < nn; ++j) {
            norm += std::abs(H(i,j));
         }
      }
   
      // Outer loop over eigenvalue index
   
      int iter = 0;
      while (n >= low) {
   
         // Look for single small sub-diagonal element
   
         int l = n;
         while (l > low) {
            s = std::abs(H(l-1,l-1)) + std::abs(H(l,l));
            if (s == 0.0) {
               s = norm;
            }
            if (std::abs(H(l,l-1)) < eps * s) {
               break;
            }
            --l;
         }
       
         // Check for convergence
         // One root found
   
         if (l == n) {
            H(n,n) = H(n,n) + exshift;
            d(n) = H(n,n);
            e(n) = 0.0;
            --n;
            iter = 0;
   
         // Two roots found
   
         } else if (l == n-1) {
            w = H(n,n-1) * H(n-1,n);
            p = (H(n-1,n-1) - H(n,n)) / 2.0;
            q = p * p + w;
            z = std::sqrt(std::abs(q));
            H(n,n) += exshift;
            H(n-1,n-1) += exshift;
            x = H(n,n);
   
            // Real pair
   
            if (q >= 0) {
               if (p >= 0) {
                  z = p + z;
               } else {
                  z = p - z;
               }
               d(n-1) = x + z;
               d(n) = d(n-1);
               if (z != 0.0) {
                  d(n) = x - w / z;
               }
               e(n-1) = 0.0;
               e(n) = 0.0;
               x = H(n,n-1);
               s = std::abs(x) + std::abs(z);
               p = x / s;
               q = z / s;
               r = std::sqrt(p * p+q * q);
               p /= r;
               q /= r;
   
               // Row modification
   
               for (int j = n-1; j < nn; ++j) {
                  z = H(n-1,j);
                  H(n-1,j) = q * z + p * H(n,j);
                  H(n,j) *= q;
                  H(n,j) -= p * z;
               }
   
               // Column modification
   
               for (int i = 0; i <= n; ++i) {
                  z = H(i,n-1);
                  H(i,n-1) = q * z + p * H(i,n);
                  H(i,n) *= q;
                  H(i,n) -= p * z;
               }
   
               // Accumulate transformations
   
               for (int i = low; i <= high; ++i) {
                  z = V(i,n-1);
                  V(i,n-1) = q * z + p * V(i,n);
                  V(i,n) *= q;
                  V(i,n) -= p * z;
               }
   
            // Complex pair
   
            } else {
               d(n-1) = x + p;
               d(n) = x + p;
               e(n-1) = z;
               e(n) = -z;
            }
            n = n - 2;
            iter = 0;
   
         // No convergence yet
   
         } else {
   
            // Form shift
   
            x = H(n,n);
            y = 0.0;
            w = 0.0;
            if (l < n) {
               y = H(n-1,n-1);
               w = H(n,n-1) * H(n-1,n);
            }
   
            // Wilkinson's original ad hoc shift
   
            if (iter == 10) {
               exshift += x;
               for (int i = low; i <= n; ++i) {
                  H(i,i) -= x;
               }
               s = std::abs(H(n,n-1)) + std::abs(H(n-1,n-2));
               x = y = 0.75 * s;
               w = -0.4375 * s * s;
            }

            // MATLAB's new ad hoc shift

            if (iter == 30) {
                s = (y - x) / 2.0;
                s *= s;
                s += w;
                if (s > 0) {
                    s = std::sqrt(s);
                    if (y < x) {
                       s = -s;
                    }
                    s = x - w / ((y - x) / 2.0 + s);
                    for (int i = low; i <= n; ++i) {
                       H(i,i) -= s;
                    }
                    exshift += s;
                    x = y = w = 0.964;
                }
            }
   
            ++iter;   // (Could check iteration count here.)
   
            // Look for two consecutive small sub-diagonal elements
   
            int m = n-2;
            while (m >= l) {
               z = H(m,m);
               r = x - z;
               s = y - z;
               p = (r * s - w) / H(m+1,m) + H(m,m+1);
               q = H(m+1,m+1) - z - r - s;
               r = H(m+2,m+1);
               s = std::abs(p) + std::abs(q) + std::abs(r);
               p /= s;
               q /= s;
               r /= s;
               if (m == l) {
                  break;
               }
               if (std::abs(H(m,m-1)) * (std::abs(q) + std::abs(r)) <
                  eps * (std::abs(p) * (std::abs(H(m-1,m-1)) + std::abs(z) +
                  std::abs(H(m+1,m+1))))) {
                     break;
               }
               --m;
            }
   
            for (int i = m+2; i <= n; ++i) {
               H(i,i-2) = 0.0;
               if (i > m+2) {
                  H(i,i-3) = 0.0;
               }
            }
   
            // Double QR step involving rows l:n and columns m:n
   
            for (int k = m; k <= n-1; ++k) {
               bool notlast = (k != n-1);
               if (k != m) {
                  p = H(k,k-1);
                  q = H(k+1,k-1);
                  r = (notlast ? H(k+2,k-1) : 0.0);
                  x = std::abs(p) + std::abs(q) + std::abs(r);
                  if (x == 0.0) {
                       continue;
                  }
                  p /= x;
                  q /= x;
                  r /= x;
               }
               s = std::sqrt(p * p + q * q + r * r);
               if (p < 0) {
                  s = -s;
               }
               if (s != 0) {
                  if (k != m) {
                     H(k,k-1) = -s * x;
                  } else if (l != m) {
                     H(k,k-1) = -H(k,k-1);
                  }
                  p += s;
                  x = p / s;
                  y = q / s;
                  z = r / s;
                  q /= p;
                  r /= p;
   
                  // Row modification
   
                  for (int j = k; j < nn; ++j) {
                     p = H(k,j) + q * H(k+1,j);
                     if (notlast) {
                        p += r * H(k+2,j);
                        H(k+2,j) -= p * z;
                     }
                     H(k,j) -= p * x;
                     H(k+1,j) -= p * y;
                  }
   
                  // Column modification
   
                  for (int i = 0; i <= std::min(n,k+3); ++i) {
                     p = x * H(i,k) + y * H(i,k+1);
                     if (notlast) {
                        p += z * H(i,k+2);
                        H(i,k+2) -= p * r;
                     }
                     H(i,k) -= p;
                     H(i,k+1) -= p * q;
                  }
   
                  // Accumulate transformations
   
                  for (int i = low; i <= high; ++i) {
                     p = x * V(i,k) + y * V(i,k+1);
                     if (notlast) {
                        p += z * V(i,k+2);
                        V(i,k+2) -= p * r;
                     }
                     V(i,k) -= p;
                     V(i,k+1) -= p * q;
                  }
               }  // (s != 0)
            }  // k loop
         }  // check convergence
      }  // while (n >= low)
      
      // Backsubstitute to find vectors of upper triangular form

      if (norm == 0.0) {
         return;
      }
   
      for (n = nn-1; n >= 0; --n) {
         p = d(n);
         q = e(n);
   
         // Real vector
   
         if (q == 0) {
            int l = n;
            H(n,n) = 1.0;
            for (int i = n-1; i >= 0; --i) {
               w = H(i,i) - p;
               r = 0.0;
               for (int j = l; j <= n; ++j) {
                  r = r + H(i,j) * H(j,n);
               }
               if (e(i) < 0.0) {
                  z = w;
                  s = r;
               } else {
                  l = i;
                  if (e(i) == 0.0) {
                     if (w != 0.0) {
                        H(i,n) = -r / w;
                     } else {
                        H(i,n) = -r / (eps * norm);
                     }
   
                  // Solve real equations
   
                  } else {
                     x = H(i,i+1);
                     y = H(i+1,i);
                     q = (d(i) - p) * (d(i) - p) + e(i) * e(i);
                     t = (x * s - z * r) / q;
                     H(i,n) = t;
                     if (std::abs(x) > std::abs(z)) {
                        H(i+1,n) = (-r - w * t) / x;
                     } else {
                        H(i+1,n) = (-s - y * t) / z;
                     }
                  }
   
                  // Overflow control
   
                  t = std::abs(H(i,n));
                  if ((eps * t) * t > 1) {
                     for (int j = i; j <= n; ++j) {
                        H(j,n) /= t;
                     }
                  }
               }
            }
   
         // Complex vector
   
         } else if (q < 0) {
            int l = n-1;

            // Last vector component imaginary so matrix is triangular
   
            if (std::abs(H(n,n-1)) > std::abs(H(n-1,n))) {
               H(n-1,n-1) = q / H(n,n-1);
               H(n-1,n) = -(H(n,n) - p) / H(n,n-1);
            } else {
               std::complex<T> ctmp(0.0,-H(n-1,n));
               std::complex<T> ctmp2(H(n-1,n-1)-p,q);
               ctmp /= ctmp2;
               H(n-1,n-1) = ctmp.real();
               H(n-1,n) = ctmp.imag();
            }
            H(n,n-1) = 0.0;
            H(n,n) = 1.0;
            for (int i = n-2; i >= 0; --i) {
               T ra,sa,vr,vi;
               ra = 0.0;
               sa = 0.0;
               for (int j = l; j <= n; ++j) {
                  ra += H(i,j) * H(j,n-1);
                  sa += H(i,j) * H(j,n);
               }
               w = H(i,i) - p;
   
               if (e(i) < 0.0) {
                  z = w;
                  r = ra;
                  s = sa;
               } else {
                  l = i;
                  if (e(i) == 0) {
                     std::complex<T> ctmp(-ra,-sa);
                     std::complex<T> ctmp2(w,q);
                     ctmp /= ctmp2;
                     H(i,n-1) = ctmp.real();
                     H(i,n) = ctmp.imag();
                  } else {
   
                     // Solve complex equations
   
                     x = H(i,i+1);
                     y = H(i+1,i);
                     vr = (d(i) - p) * (d(i) - p) + e(i) * e(i) - q * q;
                     vi = (d(i) - p) * 2.0 * q;
                     if (vr == 0.0 && vi == 0.0) {
                        vr = eps * norm * (std::abs(w) + std::abs(q) +
                        std::abs(x) + std::abs(y) + std::abs(z));
                     }
                     std::complex<T> ctmp(x*r-z*ra+q*sa,x*s-z*sa-q*ra);
                     std::complex<T> ctmp2(vr,vi);
                     ctmp /= ctmp2;
                     H(i,n-1) = ctmp.real();
                     H(i,n) = ctmp.imag();
                     if (std::abs(x) > (std::abs(z) + std::abs(q))) {
                        H(i+1,n-1) = (-ra - w * H(i,n-1) + q * H(i,n)) / x;
                        H(i+1,n) = (-sa - w * H(i,n) - q * H(i,n-1)) / x;
                     } else {
                        std::complex<T> ctmp(-r-y*H(i,n-1),-s-y*H(i,n));
                        std::complex<T> ctmp2(z,q);
                        ctmp /= ctmp2;
                        H(i+1,n-1) = ctmp.real();
                        H(i+1,n) = ctmp.imag();
                     }
                  }
   
                  // Overflow control

                  t = std::max(std::abs(H(i,n-1)),std::abs(H(i,n)));
                  if ((eps * t) * t > 1) {
                     for (int j = i; j <= n; ++j) {
                        H(j,n-1) /= t;
                        H(j,n) /= t;
                     }
                  }
               }
            }
         }
      }
   
      // Vectors of isolated roots
   
      for (int i = 0; i < nn; ++i) {
         if (i < low || i > high) {
            for (int j = i; j < nn; ++j) {
               V(i,j) = H(i,j);
            }
         }
      }
   
      // Back transformation to get eigenvectors of original matrix
   
      for (int j = nn-1; j >= low; --j) {
         for (int i = low; i <= high; ++i) {
            z = 0.0;
            for (int k = low; k <= std::min(j,high); ++k) {
               z += V(i,k) * H(k,j);
            }
            V(i,j) = z;
         }
      }
   }

/* ------------------------
   Constructor
 * ------------------------ */

   /** Check for symmetry, then construct the eigenvalue decomposition
   @param A    Square matrix
   @return     Structure to access D and V.
   */

template<class T, class TRI, class L>
EigenvalueDecomposition<T,TRI,L>::EigenvalueDecomposition (const Matrix& A) {
      BOOST_UBLAS_CHECK(A.size1() == A.size2(), bad_size());
      n = A.size2();
      V.resize(n,n,false);
      d.resize(n,false);
      e.resize(n,false);

      //issymmetric = true;
      //for (int j = 0; (j < n) && issymmetric; ++j) {
      //   for (int i = 0; (i < n) && issymmetric; ++i) {
      //      issymmetric = (A(i,j) == A(j,i));
      //   }
      //}
      issymmetric = boost::numeric::ublas::is_symmetric(A);

      if (issymmetric) {
         V = A;
   
         // Tridiagonalize.
         tred2();
   
         // Diagonalize.
         tql2();

      } else {
         H.resize(n,n,false);
         ort.resize(n,false);
         
         for (int j = 0; j < n; ++j) {
            for (int i = 0; i < n; ++i) {
               H(i,j) = A(i,j);
            }
         }
   
         // Reduce to Hessenberg form.
         orthes();
   
         // Reduce Hessenberg to real Schur form.
         hqr2();
      }
   }

template<class T, class TRI, class L>
EigenvalueDecomposition<T,TRI,L>::EigenvalueDecomposition (const SymmetricMatrix& A) {
      BOOST_UBLAS_CHECK(A.size1() == A.size2(), bad_size());
      n = A.size2();
      V.resize(n,n,false);
      d.resize(n,false);
      e.resize(n,false);

      issymmetric = true;
      V = A;
   
      // Tridiagonalize.
      tred2();
   
      // Diagonalize.
      tql2();
  }

/* ------------------------
   Public Methods
 * ------------------------ */

   /** Return the block diagonal eigenvalue matrix
    If the original matrix A is not symmetric, then the eigenvalue 
	matrix D is block diagonal with the real eigenvalues in 1-by-1 
	blocks and any complex eigenvalues,
    a + i*b, in 2-by-2 blocks, [a, b; -b, a].  That is, if the complex
    eigenvalues look like
<pre>

          u + iv     .        .          .      .    .
            .      u - iv     .          .      .    .
            .        .      a + ib       .      .    .
            .        .        .        a - ib   .    .
            .        .        .          .      x    .
            .        .        .          .      .    y
</pre>
        then D looks like
<pre>

            u        v        .          .      .    .
           -v        u        .          .      .    . 
            .        .        a          b      .    .
            .        .       -b          a      .    .
            .        .        .          .      x    .
            .        .        .          .      .    y
</pre>
    This keeps V a real matrix in both symmetric and non-symmetric
    cases, and A*V = V*D.

	@param D: upon return, the matrix is filled with the block diagonal 
	eigenvalue matrix.
	
*/

template<class T, class TRI, class L>
void EigenvalueDecomposition<T,TRI,L>::getD (Matrix &D) const {
      D.resize(n,n,false);
      D.clear();
      for (int i = 0; i < n; ++i) {
         D(i,i) = d(i);
         if (e(i) > 0) {
            D(i,i+1) = e(i);
         } else if (e(i) < 0) {
            D(i,i-1) = e(i);
         }
      }

   }

}}}
#endif
