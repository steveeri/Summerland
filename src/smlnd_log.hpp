/*
 * smlnd_log.hpp
 *
 *  Created on: 16 Sept 2018
 *      Author: steve
 */

/*************************************************************************
 * Doxygen documentation
 *************************************************************************/

/*! @file smlnd_log.h
 *  @brief Summerland header for logging macros.
 *
 */
/*! @defgroup smlnd_logging
 *  This is the reference documentation for logging related macros
 */


#ifndef SMLND_LOG_HPP_
#define SMLND_LOG_HPP_

#include <iostream>


#define SMLND_DBG_LOG(X) std::cout << X << std::endl
#define SMLND_INF_LOG(X) std::cout << X << std::endl
#define SMLND_ERR_LOG(X) std::cout << X << std::endl
#define SMLND_DBG_LOG_M(M,X) std::cout << M << X << std::endl
#define SMLND_INF_LOG_M(M,X) std::cout << M << X << std::endl
#define SMLND_ERR_LOG_M(M,X) std::cout << M << X << std::endl


#endif /* SMLND_LOG_HPP_ */
