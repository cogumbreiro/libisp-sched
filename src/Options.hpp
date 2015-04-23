/*
 * Copyright (c) 2008-2009
 *
 * School of Computing, University of Utah,
 * Salt Lake City, UT 84112, USA
 *
 * and the Gauss Group
 * http://www.cs.utah.edu/formal_verification
 *
 * See LICENSE for licensing information
 */

/*
 * ISP: MPI Dynamic Verification Tool
 *
 * File:        Options.hpp
 * Description: Includes options
 * Contact:     isp-dev@cs.utah.edu
 */

#ifndef _OPTIONS_HPP
#define _OPTIONS_HPP

#include <fstream>

namespace Options {
  static int                      _report_progress;
  static bool                     _quiet;
  static std::ofstream            _logfile;
}

#endif
