/** TestMatrix tests the functionality of the Jama Matrix class and associated decompositions.
<P>
Run the test from the command line using
<BLOCKQUOTE><PRE><CODE>
 java Jama.test.TestMatrix 
</CODE></PRE></BLOCKQUOTE>
Detailed output is provided indicating the functionality being tested
and whether the functionality is correctly implemented.   Exception handling
is also tested.  
<P>
The test is designed to run to completion and give a summary of any implementation errors
encountered. The final output should be:
<BLOCKQUOTE><PRE><CODE>
      TestMatrix completed.
      Total errors reported: n1
      Total warning reported: n2
</CODE></PRE></BLOCKQUOTE>
If the test does not run to completion, this indicates that there is a 
substantial problem within the implementation that was not anticipated in the test design.  
The stopping point should give an indication of where the problem exists.
**/
#include <iostream>
#include <iomanip>
#include <limits>
#include <boost/numeric/ublas/io.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/lagged_fibonacci.hpp>
#include "QRDecomposition.hpp"
#include "LUDecomposition.hpp"
#include "SingularValueDecomposition.hpp"
#include "CholeskyDecomposition.hpp"
#include "EigenvalueDecomposition.hpp"

using namespace boost::numeric::ublas;
using std::cout;
using std::endl;
using std::string;
using std::setw;
using std::setprecision;
using std::fixed;

typedef matrix<double> Matrix;
typedef symmetric_matrix<double> SymmetricMatrix;
typedef identity_matrix<double> IdentityMatrix;
typedef scalar_matrix<double> ScalarMatrix;
typedef vector<double> Vector;
typedef vector<int> PivotVector;

/** private utility routines **/

/** Check magnitude of difference of scalars. **/

static void check(double x, double y) {
    double eps = std::numeric_limits<double>::epsilon();
    if (x == 0 && std::abs(y) < 10*eps) return;
    if (y == 0 && std::abs(x) < 10*eps) return;
    if (std::abs(x-y) > 10*eps*std::max(std::abs(x),std::abs(y))) {
        std::ostringstream oss;
        oss << "The difference x-y is too large: x = " << x << "  y = " << y;
        throw internal_logic(oss.str().c_str());
    }
}

/** Check scalars. **/

static void check_lessthan(double x, double y) {
    if (x < y) return;
    std::ostringstream oss;
    oss << "x is more than or equal to y: x = " << x << "  y = " << y;
    throw internal_logic(oss.str().c_str());
}

/** Check norm of difference of "vectors". **/

#if 0
static void check(const Vector& x, const Vector& y) {
    BOOST_UBLAS_CHECK(x.size() == y.size(), bad_size("Attempt to compare vectors of different lengths"));
        for (unsigned i=0;i<x.size();i++) {
            check(x[i],y[i]);
        } 
}
#endif

/** Check norm of difference of Matrices. **/

static void check(const Matrix& X, const Matrix& Y) {
    double eps = std::numeric_limits<double>::epsilon();
    if (norm_1(X) == 0. && norm_1(Y) < 10*eps) return;
    if (norm_1(Y) == 0. && norm_1(X) < 10*eps) return;
    if (norm_1(X-Y) > 1000*eps*std::max(norm_1(X),norm_1(Y))) {
        std::ostringstream oss;
        oss << "The norm of (X-Y) is too large: " << norm_1(X-Y);
        throw internal_logic(oss.str().c_str());
    }
}

/** Print appropriate messages for successful outcome try **/

static void try_success (string s,string e) {
    cout << ">    " << s << "success\n";
    if ( e != "" ) {
        cout << ">      Message: " << e << "\n";
    }
}
/** Print appropriate messages for unsuccessful outcome try **/

static int try_failure (int count, string s,string e) {
    cout << ">    " << s << "*** failure ***\n>      Message: " << e << "\n";
    return ++count;
}

/** Print appropriate messages for unsuccessful outcome try **/

#if 0
static int try_warning (int count, string s,string e) {
    cout << ">    " << s << "*** warning ***\n>      Message: " << e << "\n";
    return ++count;
}
#endif

/** Print a row vector. **/

#if 0
static void print(Vector x, int w, int d) {
    // Use format Fw.d for all elements.
    cout << "\n";
    for(unsigned i=0; i<x.size(); i++) {
        cout << setw(w) << fixed << setprecision(d) << x(i);
    }
    cout << "\n";
}

/** Print a matrix. **/

static void print(Matrix m, int w, int d) {
    // Use format Fw.d for all elements.
    cout << "[\n";
    for(unsigned i=0; i<m.size1(); i++) {
        print(row(m,i),w,d);
    }
    cout << "]\n";
}
#endif

int main (int argc, char **argv) {
      Matrix A,B,C,Z,O,I,R,S,X,SUB,M,T,SQ,DEF,SOL;
      int errorCount=0;
      int warningCount=0;
    
      double columnwise[12] = {1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,12.};
      double rankdef[3][4] = {{1.,4.,7.,10.},{2.,5.,8.,11.},{3.,6.,9.,12.}};
      double subavals[2][3] = {{5.,8.,11.},{6.,9.,12.}};
      double pvals[3][3] = {{4.,1.,1.},{1.,2.,3.},{1.,3.,6.}};
      double evals[4][4] = 
         {{0.,1.,0.,0.},{1.,0.,2.e-7,0.},{0.,-2.e-7,0.,1.},{0.,0.,1.,0.}};
      double sqSolution[2][1] = {{13.},{15.}};
      double condmat[2][2] = {{1.,3.},{7.,9.}};

/**
      LA methods:
         transpose
         times
         cond
         rank
         det
         trace
         norm1
         norm2
         normF
         normInf
         solve
         solveTranspose
         inverse
         chol
         eig
         lu
         qr
         svd 
**/

      cout << "\nTesting linear algebra methods...\n";

      A = Matrix(4,3);
      for(unsigned i=0; i<A.size1(); i++) {
         for(unsigned j=0; j<A.size2(); j++) {
            A(i,j) = columnwise[i+j*4];
         }
      }
      QRDecomposition QR(A);
      R = QR.getR();
      try {
         check(A,prod(QR.getQ(),R));
         try_success("QRDecomposition...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"QRDecomposition...","incorrect QR decomposition calculation");
      }
      SingularValueDecomposition<double> SVD(A,false,true,true); // non-lazy SVD
      try {
         Matrix US = prod(SVD.getU(),SVD.getS());
         check(A,prod(US,trans(SVD.getV())));
         try_success("SingularValueDecomposition...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"SingularValueDecomposition...","incorrect singular value decomposition calculation");
      }
      try {
         Matrix UtU = prod(trans(SVD.getU()),SVD.getU()); // U is 4x4 because of non-lazy SVD
         check(UtU,IdentityMatrix(A.size1(),A.size1()));
         try_success("SingularValueDecomposition(U)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"SingularValueDecomposition(U)...","U is not orthonormal");
      }
      try {
         Matrix VtV = prod(trans(SVD.getV()),SVD.getV());
         check(VtV,IdentityMatrix(A.size2(),A.size2()));
         try_success("SingularValueDecomposition(V)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"SingularValueDecomposition(V)...","V is not orthonormal");
      }
      SingularValueDecomposition<double> SVDl(A); // lazy SVD
      try {
         Matrix US = prod(SVDl.getU(),SVDl.getS());
         check(A,prod(US,trans(SVDl.getV())));
         try_success("SingularValueDecomposition(lazy)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"SingularValueDecomposition(lazy)...","incorrect singular value decomposition calculation");
      }
      SingularValueDecomposition<double> SVDID(IdentityMatrix(3,3));
      try {
          /*
            cout << "U=";
            print(SVDID.getU(),6,2);
            cout << "S=";
            print(SVDID.getS(),6,2);
            cout << "V=";
            print(SVDID.getV(),6,2);
          */
          Matrix US = prod(SVDID.getU(),SVDID.getS());
         check(IdentityMatrix(3,3),prod(US,trans(SVDID.getV())));
         try_success("SingularValueDecomposition(Identity33)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"SingularValueDecomposition(Identity33)...","incorrect singular value decomposition calculation");
      }
      try {
           const double mean = 0.0;
           const double sigma = 1.0;
           boost::normal_distribution<double> norm_dist(mean, sigma);
           boost::lagged_fibonacci19937 engine;
           for(unsigned k=20; k<=40; k++) { // 21 tries should be OK
              Matrix AR(k,30);
              for(unsigned i=0; i<AR.size1(); i++) {
                  for(unsigned j=0; j<AR.size2(); j++) {
                      const double value = norm_dist.operator () <boost::lagged_fibonacci19937>((engine));
                      AR(i,j) = value;
                  }
              }
              SingularValueDecomposition<double> SVD(AR,false,true,true); // non-lazy SVD
              Matrix US = prod(SVD.getU(),SVD.getS());
              check(AR,prod(US,trans(SVD.getV())));
              Matrix UtU = prod(trans(SVD.getU()),SVD.getU()); // U is 4x4 because of non-lazy SVD
              check(UtU,IdentityMatrix(AR.size1(),AR.size1()));
              Matrix VtV = prod(trans(SVD.getV()),SVD.getV());
              check(VtV,IdentityMatrix(AR.size2(),AR.size2()));
          }
          try_success("SingularValueDecomposition(random)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"SingularValueDecomposition(random)...","incorrect singular value decomposition calculation");
      }

      DEF = Matrix(3,4);
      for(unsigned i=0; i<DEF.size1(); i++) {
         for(unsigned j=0; j<DEF.size2(); j++) {
            DEF(i,j) = rankdef[i][j];
         }
      }
      SVD = SingularValueDecomposition<double>(DEF);
      try {
         check(SVD.rank(),std::min(DEF.size1(),DEF.size2())-1);
         try_success("rank()...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"rank()...","incorrect rank calculation");
      }
      B = Matrix(2,2);
      for(unsigned i=0; i<B.size1(); i++) {
         for(unsigned j=0; j<B.size2(); j++) {
            B(i,j) = condmat[i][j];
         }
      }
      SVD = SingularValueDecomposition<double>(B); 
      Vector singularvalues = SVD.getSingularValues();
      try {
         check(SVD.cond(),singularvalues(0)/singularvalues(std::min(B.size1(),B.size2())-1));
         try_success("cond()...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"cond()...","incorrect condition number calculation");
      }
      int n = A.size2();
      A.resize(n,n,true);
      A(0,0) = 0.;
      LUDecomposition LU(A);
      try {
         // Compute the pivoted A
         B = Matrix(A.size1(),A.size2());
         PivotVector piv = LU.getPivot();
         for(unsigned i=0; i<piv.size(); i++) {
            row(B,i) = row(A,piv(i));
         }
         check(B,prod(LU.getL(),LU.getU()));
         try_success("LUDecomposition...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"LUDecomposition...","incorrect LU decomposition calculation");
      }
      
      QR = QRDecomposition(A);
      X = QR.inverse();
      try {
         check(prod(A,X),IdentityMatrix(3,3));
         try_success("inverse()...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"inverse()...","incorrect inverse calculation");
      }
      SUB = Matrix(2,3);
      for(unsigned i=0; i<SUB.size1(); i++) {
         for(unsigned j=0; j<SUB.size2(); j++) {
            SUB(i,j) = subavals[i][j];
         }
      }
      O = ScalarMatrix(SUB.size1(),1,1.0);
      SOL = Matrix(2,1);
      for(unsigned i=0; i<SOL.size1(); i++) {
        for(unsigned j=0; j<SOL.size2(); j++) {
           SOL(i,j) = sqSolution[i][j];
        }
      }
      SQ = subrange(SUB,0,SUB.size1(),0,SUB.size1());
      try {
         check(QRDecomposition(SQ).solve(SOL),O); 
         try_success("solve()...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"solve()...",e.what());
      }
      A = Matrix(3,3);
      for(unsigned i=0; i<A.size1(); i++) {
         for(unsigned j=0; j<A.size2(); j++) {
            A(i,j) = pvals[i][j];
         }
      }
      CholeskyDecomposition Chol(A); 
      Matrix L = Chol.getL();
      try {
         check(A,prod(L,trans(L)));
         try_success("CholeskyDecomposition...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"CholeskyDecomposition...","incorrect Cholesky decomposition calculation");
      }
      X = Chol.solve(IdentityMatrix(3,3));
      try {
         check(prod(A,X),IdentityMatrix(3,3));
         try_success("CholeskyDecomposition solve()...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"CholeskyDecomposition solve()...","incorrect Choleskydecomposition solve calculation");
      }
      EigenvalueDecomposition<double> Eig(A);
      Matrix D = Eig.getD();
      Matrix V = Eig.getV();
      Vector Re = Eig.getRealEigenvalues();
      try {
         if (!Eig.isSymmetric()) {
            throw internal_logic("A is not symmetric");
         }
         check(prod(A,V),prod(V,D));
         for(unsigned i=0; i<Re.size()-1; i++) {
            if (Re(i) > Re(i+1)) {
               throw internal_logic("Eigenvalues are not in ascending order");
            }
         }
         try_success("EigenvalueDecomposition (symmetric)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"EigenvalueDecomposition (symmetric)...","incorrect symmetric Eigenvalue decomposition calculation");
      }
      A = Matrix(4,4);
      for(unsigned i=0; i<A.size1(); i++) {
         for(unsigned j=0; j<A.size2(); j++) {
            A(i,j) = evals[i][j];
         }
      }
      Eig = EigenvalueDecomposition<double>(A);
      D = Eig.getD();
      V = Eig.getV();
      Re = Eig.getRealEigenvalues();
      Vector Im = Eig.getImagEigenvalues();
      try {
         if (Eig.isSymmetric()) {
            throw internal_logic("A is symmetric");
         }
         check(prod(A,V),prod(V,D));
         for(unsigned i=0; i<Im.size(); i++) {
            if (Im(i) != 0.) {
                if (i == Im.size() -1 || Re(i) != Re(i+1) || Im(i) < 0. || Im(i) != -Im(i+1)) {         
                    throw internal_logic("Conjugate eigenvalues are not in the right order");
                }
               ++i; // skip next value
            }
         }
         try_success("EigenvalueDecomposition (nonsymmetric)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"EigenvalueDecomposition (nonsymmetric)...","incorrect nonsymmetric Eigenvalue decomposition calculation");
      }

      try {
           const double mean = 0.0;
           const double sigma = 1.0;
           boost::normal_distribution<double> norm_dist(mean, sigma);
           boost::lagged_fibonacci19937 engine;
           for(unsigned k=20; k<=40; k++) { // 21 tries should be OK
              Matrix AR(k,k);
              for(unsigned i=0; i<AR.size1(); i++) {
                  for(unsigned j=0; j<=i; j++) {
                      const double value = norm_dist.operator () <boost::lagged_fibonacci19937>((engine));
                      AR(i,j) = value;
                      AR(j,i) = value;
                  }
              }
              Eig = EigenvalueDecomposition<double>(AR);
              if (!Eig.isSymmetric()) {
                  throw internal_logic("AR is not symmetric");
              }
              D = Eig.getD();
              V = Eig.getV();
              check(prod(AR,V),prod(V,D));
              Re = Eig.getRealEigenvalues();
              for(unsigned i=0; i<Re.size()-1; i++) {
                 if (Re(i) > Re(i+1)) {
                    throw internal_logic("Eigenvalues are not in ascending order");
                 }
              }
              Eig = EigenvalueDecomposition<double>(SymmetricMatrix(AR));
              D = Eig.getD();
              V = Eig.getV();
              check(prod(AR,V),prod(V,D));
           }
          try_success("EigenvalueDecomposition(symmetric,random)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"EigenvalueDecomposition(symmetric,random)...","incorrect symmetric Eigenvalue decomposition calculation");
      }

      try {
           const double mean = 0.0;
           const double sigma = 1.0;
           boost::normal_distribution<double> norm_dist(mean, sigma);
           boost::lagged_fibonacci19937 engine;
           for(unsigned k=20; k<=40; k++) { // 21 tries should be OK
              Matrix AR(k,k);
              for(unsigned i=0; i<AR.size1(); i++) {
                  for(unsigned j=0; j<AR.size2(); j++) {
                      const double value = norm_dist.operator () <boost::lagged_fibonacci19937>((engine));
                      AR(i,j) = value;
                  }
              }
              Eig = EigenvalueDecomposition<double>(AR);
              if (Eig.isSymmetric()) {
                 throw internal_logic("AR is symmetric");
              }
              D = Eig.getD();
              V = Eig.getV();
              Re = Eig.getRealEigenvalues();
              Im = Eig.getImagEigenvalues();
              check(prod(AR,V),prod(V,D));
              for(unsigned i=0; i<Im.size(); i++) {
                 if (Im(i) != 0.) {
                    if (i == Im.size() -1 || Re(i) != Re(i+1) || Im(i) < 0. || Im(i) != -Im(i+1)) {         
                       throw internal_logic("Conjugate eigenvalues are not in the right order");
                    }
                    ++i; // skip next value
                 }
              }
           }
          try_success("EigenvalueDecomposition(nonsymmetric,random)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"EigenvalueDecomposition(nonsymmetric,random)...","incorrect nonsymmetric Eigenvalue decomposition calculation");
      }


      try {
          // Bug from http://cio.nist.gov/esd/emaildir/lists/jama/msg01528.html :
          // all eigenvalues should be zero.
          // then Jama comes up with a largest Eigenvalue of 0.5 or something instead of a
          // 0.0 what is the correct value.
          // Interesting is, that Jama gives the correct 0.0 solution if you transpose this
          // matrix, e.g. put the "1.0"s above the main diagonal.
          // http://cio.nist.gov/esd/emaildir/lists/jama/msg01527.html says that the order of magnitude
          // should be .0032
          double eigenbug1[6][6] = {{0., 0., 0., 0., 0., 0.},
                                    {1., 0., 0., 0., 0., 0.},
                                    {0., 1., 0., 0., 0., 0.},
                                    {0., 0., 1., 0., 0., 0.},
                                    {0., 0., 0., 1., 0., 0.},
                                    {0., 0., 0., 0., 1., 0.}};
          A.resize(6,6);
          for(unsigned i=0; i<A.size1(); i++) {
              for(unsigned j=0; j<A.size2(); j++) {
                  A(i,j) = eigenbug1[i][j];
              }
          }
          Eig = EigenvalueDecomposition<double>(A);
          Vector d = Eig.getRealEigenvalues();
          Vector e = Eig.getImagEigenvalues();
          double eps = 0.0032;
          for(unsigned i=0; i<d.size(); i++) {
              check_lessthan(fabs(d(i)), eps);
          }
          for(unsigned i=0; i<e.size(); i++) {
              check_lessthan(fabs(e(i)), eps);
          }
          try_success("EigenvalueDecomposition(special1)...","");
      } catch ( std::exception e ) {
          errorCount = try_failure(errorCount,"EigenvalueDecomposition(special1)...","incorrect nonsymmetric Eigenvalue decomposition calculation");
      }
      try {
          // Bug report by Creag Winacott
          // from http://cio.nist.gov/esd/emaildir/lists/jama/msg00551.html
          //  (or http://cio.nist.gov/esd/emaildir/lists/jama/msg01525.html):
          // eigenvalue decomposition gets stuck! fixed in ublasJama 1.0.2.3.
          // see explanation in README-Eigenbug.txt
          double eigenbug2[5][5] = {{0., 0., 0., 0., 0.},
                                    {0., 0., 0., 0., 1.},
                                    {0., 0., 0., 1., 0.},
                                    {1., 1., 0., 0., 1.},
                                    {1., 0., 1., 0., 1.}};
          A.resize(5,5);
          for(unsigned i=0; i<A.size1(); i++) {
              for(unsigned j=0; j<A.size2(); j++) {
                  A(i,j) = eigenbug2[i][j];
              }
          }
          Eig = EigenvalueDecomposition<double>(A);
          D = Eig.getD();
          V = Eig.getV();
          check(prod(A,V),prod(V,D));
          try_success("EigenvalueDecomposition(special2)...","");
      } catch ( std::exception e ) {
         errorCount = try_failure(errorCount,"EigenvalueDecomposition(special2)...","incorrect nonsymmetric Eigenvalue decomposition calculation");
      }
    try {
        // this matrix is almost non-symmetric, so force it to be non-symmetric
        double eigenbug3[4][4] = {{1,0,-7.49881e-33,-1},
                                  {3.74939e-33,1,1.2326e-32,-3.74939e-33},
                                  {-7.49881e-33,1.2326e-32,1,7.49881e-33},
                                  {-1,-3.74939e-33,1.2326e-32,1}};
        A.resize(4,4);
        for(unsigned i=0; i<A.size1(); i++) {
            for(unsigned j=0; j<A.size2(); j++) {
                A(i,j) = eigenbug3[i][j];
            }
        }
        Eig = EigenvalueDecomposition<double>(A,true);
        Vector d = Eig.getRealEigenvalues();
        Vector e = Eig.getImagEigenvalues();
        double eps = 1e-15;
        check_lessthan(fabs(d(0)), eps);
        check_lessthan(fabs(e(0)), eps);
        try_success("EigenvalueDecomposition(special3)...","");
    } catch ( std::exception e ) {
        errorCount = try_failure(errorCount,"EigenvalueDecomposition(special3)...","incorrect nonsymmetric Eigenvalue decomposition calculation");
    }
      cout << "\nTestMatrix completed.\n";
      cout << "Total errors reported: " << errorCount << "\n";
      cout << "Total warnings reported: " << warningCount << "\n";
   }

