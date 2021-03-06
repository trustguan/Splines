/*--------------------------------------------------------------------------*\
 |                                                                          |
 |  Copyright (C) 2016                                                      |
 |                                                                          |
 |         , __                 , __                                        |
 |        /|/  \               /|/  \                                       |
 |         | __/ _   ,_         | __/ _   ,_                                | 
 |         |   \|/  /  |  |   | |   \|/  /  |  |   |                        |
 |         |(__/|__/   |_/ \_/|/|(__/|__/   |_/ \_/|/                       |
 |                           /|                   /|                        |
 |                           \|                   \|                        |
 |                                                                          |
 |      Enrico Bertolazzi                                                   |
 |      Dipartimento di Ingegneria Industriale                              |
 |      Universita` degli Studi di Trento                                   |
 |      email: enrico.bertolazzi@unitn.it                                   |
 |                                                                          |
\*--------------------------------------------------------------------------*/

#include "Splines.hh"
#include <iomanip>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wpoison-system-directories"
#endif

/**
 * 
 */

namespace Splines {

  using namespace std; // load standard namespace

  // build spline without computation

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void
  CubicSplineBase::build(
    real_type const x[],  integer incx,
    real_type const y[],  integer incy,
    real_type const yp[], integer incyp,
    integer n
  ) {
    this->reserve( n );
    for ( size_t i = 0; i < size_t(n); ++i ) {
      m_X[i]  = x[i*size_t(incx)];
      m_Y[i]  = y[i*size_t(incy)];
      m_Yp[i] = yp[i*size_t(incyp)];
    }
    m_npts = n;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void
  CubicSplineBase::build(
    vector<real_type> const & x,
    vector<real_type> const & y,
    vector<real_type> const & yp
  ) {
    integer N = integer(x.size());
    if ( N > integer(y.size())  ) N = integer(y.size());
    if ( N > integer(yp.size()) ) N = integer(yp.size());
    this->build (
      &x.front(),  1,
      &y.front(),  1,
      &yp.front(), 1,
      N
    );
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void
  CubicSplineBase::clear(void) {
    if ( !m_external_alloc ) m_baseValue.free();
    m_npts = m_npts_reserved = 0;
    m_external_alloc = false;
    m_X = m_Y = m_Yp = nullptr;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void
  CubicSplineBase::reserve( integer n ) {
    if ( m_external_alloc && n <= m_npts_reserved ) {
      // nothing to do!, already allocated
    } else {
      m_npts_reserved = n;
      m_baseValue.allocate( size_t(3*n) );
      m_X  = m_baseValue( size_t(n) );
      m_Y  = m_baseValue( size_t(n) );
      m_Yp = m_baseValue( size_t(n) );
      m_external_alloc = false;
    }
    initLastInterval();
    m_npts = 0;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void
  CubicSplineBase::reserve_external(
    integer      n,
    real_type *& p_x,
    real_type *& p_y,
    real_type *& p_dy
  ) {
    m_npts_reserved  = n;
    m_X              = p_x;
    m_Y              = p_y;
    m_Yp             = p_dy;
    m_external_alloc = true;
    initLastInterval();
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  real_type
  CubicSplineBase::id_eval( integer i, real_type x ) const {
    if ( m_curve_can_extend && m_curve_extended_constant ) {
      if ( x <= m_X[0]        ) return m_Y[0];
      if ( x >= m_X[m_npts-1] ) return m_Y[m_npts-1];
    }
    real_type base[4];
    Hermite3( x-m_X[i], m_X[i+1]-m_X[i], base );
    return base[0] * m_Y[i]   +
           base[1] * m_Y[i+1] +
           base[2] * m_Yp[i]  +
           base[3] * m_Yp[i+1];
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  real_type
  CubicSplineBase::operator () ( real_type x ) const {
    integer idx = this->search( x ); // eval idx can modify x
    return this->id_eval( idx, x );
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  real_type
  CubicSplineBase::id_D( integer i, real_type x ) const {
    if ( m_curve_can_extend && m_curve_extended_constant ) {
      if ( x <= m_X[0] || x >= m_X[m_npts-1] ) return 0;
    }
    real_type base_D[4];
    Hermite3_D( x-m_X[i], m_X[i+1]-m_X[i], base_D );
    return base_D[0] * m_Y[i]   +
           base_D[1] * m_Y[i+1] +
           base_D[2] * m_Yp[i]  +
           base_D[3] * m_Yp[i+1];
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  real_type
  CubicSplineBase::D( real_type x ) const {
    integer idx = this->search( x ); // eval idx can modify x
    return this->id_D( idx, x );
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  real_type
  CubicSplineBase::id_DD( integer i, real_type x ) const {
    if ( m_curve_can_extend && m_curve_extended_constant ) {
      if ( x <= m_X[0] || x >= m_X[m_npts-1] ) return 0;
    }
    real_type base_DD[4];
    Hermite3_DD( x-m_X[i], m_X[i+1]-m_X[i], base_DD );
    return base_DD[0] * m_Y[i]   +
           base_DD[1] * m_Y[i+1] +
           base_DD[2] * m_Yp[i]  +
           base_DD[3] * m_Yp[i+1];
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  real_type
  CubicSplineBase::DD( real_type x ) const {
    integer idx = this->search( x ); // eval idx can modify x
    return this->id_DD( idx, x );
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  real_type
  CubicSplineBase::id_DDD( integer i, real_type x ) const {
    if ( m_curve_can_extend && m_curve_extended_constant ) {
      if ( x <= m_X[0] || x >= m_X[m_npts-1] ) return 0;
    }
    real_type base_DDD[4];
    Hermite3_DDD( x-m_X[i], m_X[i+1]-m_X[i], base_DDD );
    return base_DDD[0] * m_Y[i]   +
           base_DDD[1] * m_Y[i+1] +
           base_DDD[2] * m_Yp[i]  +
           base_DDD[3] * m_Yp[i+1];
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  real_type
  CubicSplineBase::DDD( real_type x ) const {
    integer idx = this->search( x ); // eval idx can modify x
    return this->id_DDD( idx, x );
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  integer // order
  CubicSplineBase::coeffs(
    real_type cfs[],
    real_type nodes[],
    bool      transpose
  ) const {
    size_t n = size_t(m_npts > 0 ? m_npts-1 : 0);
    for ( size_t i = 0; i < n; ++i ) {
      nodes[i] = m_X[i];
      real_type H  = m_X[i+1]-m_X[i];
      real_type DY = (m_Y[i+1]-m_Y[i])/H;
      real_type a = m_Y[i];
      real_type b = m_Yp[i];
      real_type c = (3*DY-2*m_Yp[i]-m_Yp[i+1])/H;
      real_type d = (m_Yp[i+1]+m_Yp[i]-2*DY)/(H*H);
      if ( transpose ) {
        cfs[4*i+3] = a;
        cfs[4*i+2] = b;
        cfs[4*i+1] = c;
        cfs[4*i+0] = d;
      } else {
        cfs[i+3*n] = a;
        cfs[i+2*n] = b;
        cfs[i+1*n] = c;
        cfs[i+0*n] = d;
      }
    }
    return 4;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  integer
  CubicSplineBase::order() const
  { return 4; }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // Implementation
  void
  CubicSplineBase::copySpline( CubicSplineBase const & S ) {
    CubicSplineBase::reserve(S.m_npts);
    m_npts = S.m_npts;
    std::copy_n( S.m_X,  m_npts, m_X  );
    std::copy_n( S.m_Y,  m_npts, m_Y  );
    std::copy_n( S.m_Yp, m_npts, m_Yp );
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  //! change X-range of the spline
  void
  CubicSplineBase::setRange( real_type xmin, real_type xmax ) {
    Spline::setRange( xmin, xmax );
    real_type recS = ( m_X[m_npts-1] - m_X[0] ) / (xmax - xmin);
    real_type * iy = m_Y;
    while ( iy < m_Y + m_npts ) *iy++ *= recS;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void
  CubicSplineBase::writeToStream( ostream_type & s ) const {
    size_t nseg = size_t( m_npts > 0 ? m_npts - 1 : 0 );
    for ( size_t i = 0; i < nseg; ++i )
      fmt::print( s,
        "segment N.{:4} X:[{},{}] Y:[{},{}] Yp:[{},{}] slope: {}\n",
        i, m_X[i], m_X[i+1], m_Y[i], m_Y[i+1], m_Yp[i], m_Yp[i+1],
        (m_Y[i+1]-m_Y[i])/(m_X[i+1]-m_X[i])
      );
  }

}
