#ifndef _STDAFX_
#define _STDAFX_

#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <assert.h>

#include <boost/utility.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <boost/array.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include <boost/optional.hpp>

#include <boost/program_options.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/tuple/tuple.hpp>
using boost::tuple;
using boost::make_tuple;

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <complex>
#define MKL_Complex16 std::complex<double>
#include <mkl_lapacke.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

#ifdef INCLUDE_QT_HEADERS
#include <QApplication>
#include <QColor>
#include <QtOpenGL>
#endif

#endif /*_STDAFX_*/
