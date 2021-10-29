/*    Copyright (C) 2020 Erik L. G. Wernersson
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


/* This programs calculate Points Spread Functions for deconvolution of
 * wide field microscopy images (stacks).
 * Inspiration:
 *  https://github.com/Biomedical-Imaging-Group/PSFGenerator/tree/master/src/psf/bornwolf
 */

#ifndef dw_bwpsf_h
#define dw_bwpsf_h

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <complex.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include "fim_tiff.h"
#include "dw_version.h"

#include <gsl/gsl_integration.h>

/* Mode for calculating the 1D integral */
#define MODE_INT1_GSL 0
#define MODE_INT1_LI 1
#define MODE_INT1_PG 2


typedef struct {
    int verbose;
    int overwrite;

    char * cmd; // Command line
    char * outFile;
    char * logFile;
    FILE * log;

    // Settings for 'calculate'
    size_t K; // stop at this number of successful iterations
    float TOL; // Defines a successful iteration

    // Physical parameters
    float lambda; // Emission maxima
    float NA; // Numerical aperture of lens
    float ni;
    float resLateral; // pixel size in x and y
    float resAxial; // pixel size in z

    // Integration options
    int samples_xy;
    int samples_z; // Number of points in Z
    int mode_int1; /* How to calculate the 1D integral */
    int oversampling_R; // Oversampling in radial direction

    float * V; // output image
    // shape of output image
    int M;
    int N;
    int P;

    // For parallel processing
    int thread;
    int nThreads;

    int testing;
} bw_conf;

void bw_conf_printf(FILE * out, bw_conf * conf);
void BW_slice(float * , float z, bw_conf * conf);
void bw_argparsing(int , char ** , bw_conf * conf);
void * BW_thread(void * data);

/* Generate the Born-Wolf PSF
 * allocates conf->V is already allocated
 */
void BW(bw_conf * conf);

// Get the command line options
void getCmdLine(int argc, char ** argv, bw_conf * s);


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


#endif
