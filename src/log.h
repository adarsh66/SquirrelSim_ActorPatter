/* ------------------------------------------------------------
 * References
 * The code below was adapted from the original on the 
 * website LearnCTheHardWay, author: Zed Shaw
 * Original code can be found at
 * http://c.learncodethehardway.org/book/ex20.html
 * ------------------------------------------------------------
 * debug (msg) 			: Only when compiled with Debug flag. if you want to print out from every rank
 * log_info (msg)		: if you want every rank to print out to stderr
 * log_it(msg) 			: if you want to print out to std out
 */

#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef DEBUGLOG
#define debug(M, ...) fprintf(stderr, "[DEBUG] (%s:%d): " M, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif

#ifdef INFOLOG
#define log_info(M, ...) fprintf(stderr, "[INFO] " M, ##__VA_ARGS__)
#else 
#define log_info(M, ...) 
#endif


#define log_it(M, ...) fprintf(stdout, M, ##__VA_ARGS__)

#endif