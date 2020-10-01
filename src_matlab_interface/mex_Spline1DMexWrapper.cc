/****************************************************************************\
Copyright (c) 2015, Enrico Bertolazzi
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
\****************************************************************************/

#include "Splines.hh"
#include "mex_utils.hh"

#define ASSERT(COND,MSG)                          \
  if ( !(COND) ) {                                \
    std::ostringstream ost;                       \
    ost << "Spline1DMexWrapper: " << MSG << '\n'; \
    mexErrMsgTxt(ost.str().c_str());              \
  }

#define MEX_ERROR_MESSAGE \
"%======================================================================%\n" \
"% Spline1DMexWrapper:  Compute spline curve                            %\n" \
"%                                                                      %\n" \
"% USAGE:                                                               %\n" \
"%   obj = Spline1DMexWrapper( 'new', kind );                           %\n" \
"%   Spline1DMexWrapper( 'delete', obj );                               %\n" \
"%   Spline1DMexWrapper( 'build', obj, X, Y, [Yp or subtype] );         %\n" \
"%   P      = Spline1DMexWrapper( 'eval', obj, X );                     %\n" \
"%   DP     = Spline1DMexWrapper( 'eval_D', obj, X );                   %\n" \
"%   DDP    = Spline1DMexWrapper( 'eval_DD', obj, X );                  %\n" \
"%   DDDP   = Spline1DMexWrapper( 'eval_DDD', obj, X );                 %\n" \
"%   DDDDP  = Spline1DMexWrapper( 'eval_DDDD', obj, X );                %\n" \
"%   DDDDDP = Spline1DMexWrapper( 'eval_DDDDD', obj, X );               %\n" \
"%                                                                      %\n" \
"% On input:                                                            %\n" \
"%                                                                      %\n" \
"%  kind = string with the kind of spline, any of:                      %\n" \
"%         'linear', 'cubic', 'akima', 'bessel', 'pchip', 'quintic'     %\n" \
"%  X = vector of X coordinates                                         %\n" \
"%  Y = vector of Y coordinates                                         %\n" \
"%                                                                      %\n" \
"% On output:                                                           %\n" \
"%                                                                      %\n" \
"%  P      = vector of Y values                                         %\n" \
"%  DP     = vector of dimension size(X) with derivative                %\n" \
"%  DDP    = vector of dimension size(X) with second derivative         %\n" \
"%  DDDP   = vector of dimension size(X) with third derivative          %\n" \
"%  DDDDP  = vector of dimension size(X) with 4th derivative            %\n" \
"%  DDDDDP = vector of dimension size(X) with 5th derivative            %\n" \
"%                                                                      %\n" \
"%======================================================================%\n" \
"%                                                                      %\n" \
"%  Autor: Enrico Bertolazzi                                            %\n" \
"%         Department of Industrial Engineering                         %\n" \
"%         University of Trento                                         %\n" \
"%         enrico.bertolazzi@unitn.it                                   %\n" \
"%                                                                      %\n" \
"%======================================================================%\n"

using namespace std;

namespace Splines {

  static
  Spline *
  DATA_NEW( mxArray * & mx_id, Spline * ptr ) {
    mx_id = convertPtr2Mat<Spline>(ptr);
    return ptr;
  }

  static
  inline
  Spline *
  DATA_GET( mxArray const * & mx_id ) {
    return convertMat2Ptr<Spline>(mx_id);
  }

  static
  void
  DATA_DELETE( mxArray const * & mx_id ) {
    destroyObject<Spline>(mx_id);
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_new(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper( 'new', kind ): "
    MEX_ASSERT( nrhs == 2, CMD "expected 2 inputs, nrhs = " << nrhs );
    MEX_ASSERT( nlhs == 1, CMD "expected 1 output, nlhs = " << nlhs );

    MEX_ASSERT( mxIsChar(arg_in_1),
                CMD "second argument must be a string, found ``" <<
                mxGetClassName(arg_in_1) << "''" );

    // the first argument must be a string
    string tname = mxArrayToString(arg_in_1);

    if      ( tname == "linear"  ) DATA_NEW( arg_out_0, new Splines::LinearSpline() );
    else if ( tname == "cubic"   ) DATA_NEW( arg_out_0, new Splines::CubicSpline()  );
    else if ( tname == "akima"   ) DATA_NEW( arg_out_0, new Splines::AkimaSpline()  );
    else if ( tname == "bessel"  ) DATA_NEW( arg_out_0, new Splines::BesselSpline() );
    else if ( tname == "pchip"   ) DATA_NEW( arg_out_0, new Splines::PchipSpline()  );
    else if ( tname == "hermite" ) DATA_NEW( arg_out_0, new Splines::HermiteSpline());
    else if ( tname == "quintic" ) DATA_NEW( arg_out_0, new Splines::QuinticSpline());
    else {
      MEX_ASSERT(
        false,
        "Second argument must be one of the strings:\n" <<
        "'linear', 'cubic', 'akima', 'bessel', 'hermite', 'pchip', 'quintic'"
      );
    }

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_delete(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper( 'delete', OBJ ): "
    MEX_ASSERT( nrhs == 2, CMD "expected 2 inputs, nrhs = " << nrhs );
    MEX_ASSERT( nlhs == 0, CMD "expected 0 output, nlhs = " << nlhs );

    // Destroy the C++ object
    DATA_DELETE(arg_in_1);

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_build(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('build',OBJ,x,y[,yp or subtype]): "

    MEX_ASSERT( nlhs == 0, CMD "expected no output, nlhs = " << nlhs );
    MEX_ASSERT(
      nrhs == 4 || nrhs == 5,
      CMD "expected 4 or 5 input, nrhs = " << nrhs
    );

    mwSize nx, ny;
    real_type const * x  = nullptr;
    real_type const * y  = nullptr;
    real_type const * yp = nullptr;
    x = getVectorPointer( arg_in_2, nx, CMD "error in reading 'x'" );
    y = getVectorPointer( arg_in_3, ny, CMD "error in reading 'y'" );

    MEX_ASSERT( nx == ny, "lenght of 'x' must be the lenght of 'y'" );

    Spline * ptr = DATA_GET( arg_in_1 );

    if ( nrhs == 5 ) {
      if ( mxIsChar(arg_in_4) ) {
        MEX_ASSERT(
          ptr->type() == CUBIC_TYPE || ptr->type() == QUINTIC_TYPE,
          "subtype can be specifiewd only for Cubic or Quintic spline"
        );
        string subtype = mxArrayToString(arg_in_4);
        switch ( ptr->type() ) {
        case CONSTANT_TYPE:
        case LINEAR_TYPE:
        case AKIMA_TYPE:
        case BESSEL_TYPE:
        case PCHIP_TYPE:
        case HERMITE_TYPE:
          break;
        case CUBIC_TYPE:
          if ( subtype == "extrapolate" ) {
            static_cast<CubicSpline*>(ptr)->setInitialBC( EXTRAPOLATE_BC );
            static_cast<CubicSpline*>(ptr)->setFinalBC( EXTRAPOLATE_BC );
          } else if ( subtype == "natural" ) {
            static_cast<CubicSpline*>(ptr)->setInitialBC( NATURAL_BC );
            static_cast<CubicSpline*>(ptr)->setFinalBC( NATURAL_BC );
          } else if ( subtype == "parabolic"  ) {
            static_cast<CubicSpline*>(ptr)->setInitialBC( PARABOLIC_RUNOUT_BC );
            static_cast<CubicSpline*>(ptr)->setFinalBC( PARABOLIC_RUNOUT_BC );
          } else if ( subtype == "not_a_knot" ) {
            static_cast<CubicSpline*>(ptr)->setInitialBC( NOT_A_KNOT );
            static_cast<CubicSpline*>(ptr)->setFinalBC( NOT_A_KNOT );
          } else {
            MEX_ASSERT(
              false,
              CMD "subtype: " <<  subtype <<
              " must be in ['extrapolate','natural','parabolic','not_a_knot']"
            );
          }
          break;
        case QUINTIC_TYPE:
          if ( subtype == "cubic" ) {
            static_cast<QuinticSpline*>(ptr)->setQuinticType( CUBIC_QUINTIC );
          } else if ( subtype == "pchip"  ) {
            static_cast<QuinticSpline*>(ptr)->setQuinticType( PCHIP_QUINTIC );
          } else if ( subtype == "akima" ) {
            static_cast<QuinticSpline*>(ptr)->setQuinticType( AKIMA_QUINTIC );
          } else if ( subtype == "bessel" ) {
            static_cast<QuinticSpline*>(ptr)->setQuinticType( BESSEL_QUINTIC );
          } else {
            MEX_ASSERT(
              false,
              CMD "subtype: " <<  subtype <<
              " must be in ['cubic','pchip','akima','bessel']"
            );
          }
          break;
        }
      } else {
        MEX_ASSERT(
          ptr->type() == HERMITE_TYPE,
          "yp can be specified only for Hermite type Spline"
        );
        mwSize nyp;
        yp = getVectorPointer( arg_in_4, nyp, CMD "error in reading 'yp'" );
        MEX_ASSERT( ny == nyp, "lenght of 'yp' must be the lenght of 'y'" );
      }
    }
    switch ( ptr->type() ) {
    case HERMITE_TYPE:
      MEX_ASSERT(
        x != nullptr && y != nullptr && yp != nullptr,
        CMD "something go wrong in reading x, y, or yp"
      );
      static_cast<HermiteSpline*>(ptr)->build( x, y, yp, nx );
      break;
    default:
      MEX_ASSERT(
        x != nullptr && y != nullptr,
        CMD "something go wrong in reading x or y"
      );
      ptr->build( x, y, nx );
      break;
    }
    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_degree(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('degree',OBJ): "

    MEX_ASSERT( nlhs == 1, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );
    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarInt( arg_out_0, ptr->order()-1 );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_order(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('order',OBJ): "

    MEX_ASSERT( nlhs == 1, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );
    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarInt( arg_out_0, ptr->order() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_eval(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('eval',OBJ): "

    MEX_ASSERT( nlhs == 1, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 3, CMD "expected 3 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );

    mwSize nx;
    real_type const * x = getVectorPointer(
      arg_in_2, nx, CMD "error in reading `x`"
    );
    real_type * y = createMatrixValue( arg_out_0, nx, 1 );

    for ( mwSize i = 0; i < nx; ++i ) *y++ = ptr->eval( *x++ );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_eval_D(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('eval_D',OBJ): "

    MEX_ASSERT( nlhs == 1, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 3, CMD "expected 3 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );

    mwSize nx;
    real_type const * x = getVectorPointer(
      arg_in_2, nx, CMD "error in reading `x`"
    );
    real_type * y = createMatrixValue( arg_out_0, nx, 1 );

    for ( mwSize i = 0; i < nx; ++i ) *y++ = ptr->eval_D( *x++ );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_eval_DD(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('eval_DD',OBJ): "

    MEX_ASSERT( nlhs == 1, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 3, CMD "expected 3 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );

    mwSize nx;
    real_type const * x = getVectorPointer(
      arg_in_2, nx, CMD "error in reading `x`"
    );
    real_type * y = createMatrixValue( arg_out_0, nx, 1 );

    for ( mwSize i = 0; i < nx; ++i ) *y++ = ptr->eval_DD( *x++ );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_eval_DDD(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('eval_DDD',OBJ): "

    MEX_ASSERT( nlhs == 1, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 3, CMD "expected 3 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );

    mwSize nx;
    real_type const * x = getVectorPointer(
      arg_in_2, nx, CMD "error in reading `x`"
    );
    real_type * y = createMatrixValue( arg_out_0, nx, 1 );

    for ( mwSize i = 0; i < nx; ++i ) *y++ = ptr->eval_DDD( *x++ );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_eval_DDDD(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('eval_DDDD',OBJ): "

    MEX_ASSERT( nlhs == 1, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 3, CMD "expected 3 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );

    mwSize nx;
    real_type const * x = getVectorPointer(
      arg_in_2, nx, CMD "error in reading `x`"
    );
    real_type * y = createMatrixValue( arg_out_0, nx, 1 );

    for ( mwSize i = 0; i < nx; ++i ) *y++ = ptr->eval_DDDD( *x++ );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_eval_DDDDD(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('eval_DDDDD',OBJ): "

    MEX_ASSERT( nlhs == 1, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 3, CMD "expected 3 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );

    mwSize nx;
    real_type const * x = getVectorPointer(
      arg_in_2, nx, CMD "error in reading `x`"
    );
    real_type * y = createMatrixValue( arg_out_0, nx, 1 );

    for ( mwSize i = 0; i < nx; ++i ) *y++ = ptr->eval_DDDDD( *x++ );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_make_closed(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('make_closed',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 0 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    ptr->make_closed();

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_make_opened(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('make_opened',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 0 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    ptr->make_opened();

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_is_closed(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('is_closed',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->is_closed() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_make_bounded(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('make_bounded',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 0 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    ptr->make_bounded();

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_make_unbounded(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('make_unbounded',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 0 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    ptr->make_unbounded();

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_is_bounded(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('is_bounded',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->is_bounded() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_make_extended_constant(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('make_extended_constant',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 0 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    ptr->make_extended_constant();

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_make_extended_not_constant(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('make_extended_not_constant',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 0 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    ptr->make_extended_not_constant();

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_is_extended_constant(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('is_extended_constant',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->is_extended_constant() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_xBegin(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('xBegin',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->xBegin() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_yBegin(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('yBegin',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->yBegin() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_xEnd(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('xEnd',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->xEnd() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_yEnd(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('yEnd',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->yEnd() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_xMin(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('xMin',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->xMin() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_yMin(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('yMin',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->yMin() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_xMax(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('xMax',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->xMax() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static
  void
  do_yMax(
    int nlhs, mxArray       *plhs[],
    int nrhs, mxArray const *prhs[]
  ) {

    #define CMD "Spline1DMexWrapper('yMax',OBJ): "

    MEX_ASSERT( nlhs == 0, CMD "expected 1 output, nlhs = " << nlhs );
    MEX_ASSERT( nrhs == 2, CMD "expected 2 input, nrhs = " << nrhs );

    Spline * ptr = DATA_GET( arg_in_1 );
    setScalarBool( arg_out_0, ptr->yMax() );

    #undef CMD
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  typedef enum {
    CMD_NEW,
    CMD_DELETE,
    CMD_BUILD,
    CMD_DEGREE,
    CMD_ORDER,
    CMD_EVAL,
    CMD_EVAL_D,
    CMD_EVAL_DD,
    CMD_EVAL_DDD,
    CMD_EVAL_DDDD,
    CMD_EVAL_DDDDD,
    CMD_MAKE_CLOSED,
    CMD_MAKE_OPENED,
    CMD_IS_CLOSED,
    CMD_MAKE_BOUNDED,
    CMD_MAKE_UNBOUNDED,
    CMD_IS_BOUNDED,
    CMD_MAKE_EXTENDED_CONSTANT,
    CMD_MAKE_EXTENDED_NOT_CONSTANT,
    CMD_IS_EXTENDED_CONSTANT,
    CMD_XBEGIN,
    CMD_YBEGIN,
    CMD_XEND,
    CMD_YEND,
    CMD_XMIN,
    CMD_YMIN,
    CMD_XMAX,
    CMD_YMAX
  } CMD_LIST;

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  static map<string,unsigned> cmd_to_idx = {
    {"new",CMD_NEW},
    {"delete",CMD_DELETE},
    {"build",CMD_BUILD},
    {"degree",CMD_DEGREE},
    {"order",CMD_ORDER},
    {"eval",CMD_EVAL},
    {"eval_D",CMD_EVAL_D},
    {"eval_DD",CMD_EVAL_DD},
    {"eval_DDD",CMD_EVAL_DDD},
    {"eval_DDDD",CMD_EVAL_DDDD},
    {"eval_DDDDD",CMD_EVAL_DDDDD},
    {"make_closed",CMD_MAKE_CLOSED},
    {"make_opened",CMD_MAKE_OPENED},
    {"is_closed",CMD_IS_CLOSED},
    {"make_bounded",CMD_MAKE_BOUNDED},
    {"make_unbounded",CMD_MAKE_UNBOUNDED},
    {"is_bounded",CMD_IS_BOUNDED},
    {"make_extended_constant",CMD_MAKE_EXTENDED_CONSTANT},
    {"make_extended_not_constant",CMD_MAKE_EXTENDED_NOT_CONSTANT},
    {"is_extended_constant",CMD_IS_EXTENDED_CONSTANT},
    {"xBegin",CMD_XBEGIN},
    {"yBegin",CMD_YBEGIN},
    {"xEnd",CMD_XEND},
    {"yEnd",CMD_YEND},
    {"xMin",CMD_XMIN},
    {"yMin",CMD_YMIN},
    {"xMax",CMD_XMAX},
    {"yMax",CMD_YMAX}
  };

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  extern "C"
  void
  mexFunction( int nlhs, mxArray       *plhs[],
               int nrhs, mxArray const *prhs[] ) {
    // the first argument must be a string
    if ( nrhs == 0 ) {
      mexErrMsgTxt(MEX_ERROR_MESSAGE);
      return;
    }

    try {

      MEX_ASSERT( mxIsChar(arg_in_0), "First argument must be a string" );
      string cmd = mxArrayToString(arg_in_0);

      switch ( cmd_to_idx.at(cmd) ) {
      case CMD_NEW:
        do_new( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_DELETE:
        do_delete( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_BUILD:
        do_build( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_EVAL:
        do_eval( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_EVAL_D:
        do_eval_D( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_EVAL_DD:
        do_eval_DD( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_EVAL_DDD:
        do_eval_DDD( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_EVAL_DDDD:
        do_eval_DDDD( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_EVAL_DDDDD:
        do_eval_DDDDD( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_DEGREE:
        do_degree( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_ORDER:
        do_order( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_MAKE_CLOSED:
        do_make_closed( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_MAKE_OPENED:
        do_make_opened( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_IS_CLOSED:
        do_is_closed( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_MAKE_BOUNDED:
        do_make_bounded( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_MAKE_UNBOUNDED:
        do_make_unbounded( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_IS_BOUNDED:
        do_is_bounded( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_MAKE_EXTENDED_CONSTANT:
        do_make_extended_constant( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_MAKE_EXTENDED_NOT_CONSTANT:
        do_make_extended_not_constant( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_IS_EXTENDED_CONSTANT:
        do_is_extended_constant( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_XBEGIN:
        do_xBegin( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_YBEGIN:
        do_yBegin( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_XEND:
        do_xEnd( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_YEND:
        do_yEnd( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_XMIN:
        do_xMin( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_YMIN:
        do_yMin( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_XMAX:
        do_xMax( nlhs, plhs, nrhs, prhs );
        break;
      case CMD_YMAX:
        do_yMax( nlhs, plhs, nrhs, prhs );
        break;
      }
    } catch ( exception const & e ) {
      mexErrMsgTxt(e.what());
    } catch (...) {
      mexErrMsgTxt("SplineSetMexWrapper failed\n");
    }

  }

}
