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


#include "dw.h"



/* GLOBALS */
/* used by tiffErrHandler */
FILE * logfile = NULL;
/* END GLOBALS */


dw_iterator_t * dw_iterator_new(const dw_opts * s)
{
    dw_iterator_t * it = malloc(sizeof(dw_iterator_t));
    it->error = 1;
    it->lasterror = 1;
    it->itertype = s->iter_type;
    it->abserror = s->err_abs;
    it->relerror = s->err_rel;
    it->iter = -1;
    it->niter = s->nIter;
    if(it->itertype != DW_ITER_FIXED)
    {
        it->niter = s->maxiter;
    }

    return it;
}

int dw_iterator_next(dw_iterator_t * it)
{
    it->iter++;
    switch(it->itertype)
    {
    case DW_ITER_FIXED:
        //printf("DW_ITER_FIXED\n");
        break;
    case DW_ITER_REL:
        //printf("DW_ITER_REL, %e, %e\n", fabs(it->error - it->lasterror)/it->error, it->relerror);
        if(it->iter < 2)
        {
            it->lasterror = 2*it->error*it->relerror;
        }
        if(fabs(it->error - it->lasterror)/it->error < it->relerror)
        {
            return -1;
        }
        break;
    case DW_ITER_ABS:
        //printf("DW_ITER_ABS %f < %f ?\n", it->error, it->abserror);
        if(it->iter > 0 && it->error < it->abserror)
        {
            return -1;
        }
        break;
    }

    if(it->iter >= it->niter)
    {
        return -1;
    }

    return it->iter;
}

void dw_iterator_set_error(dw_iterator_t * it, float err)
{
    it->lasterror = it->error;
    it->error = err;
}

void dw_iterator_show(dw_iterator_t * it, const dw_opts *s)
{
    if(s->verbosity > 0){
        printf("\r                                             ");
        if(s->metric == DW_METRIC_MSE)
        {
            printf("\rIteration %3d/%3d, fMSE=%.3e ",
                   it->iter+1, it->niter, it->error);
        }
        if(s->metric == DW_METRIC_IDIV)
        {
            printf("\rIteration %3d/%3d, Idiv=%.3e ",
                   it->iter+1, it->niter, it->error);
        }
        if(it->itertype == DW_ITER_REL && it->iter > 1)
        {
            double rel = fabs(it->error - it->lasterror)/it->error;
            printf("(%.3e", rel);
            if(rel > it->relerror)
            {
                printf(" > ");
            } else {
                printf(" < ");
            }
            printf("%.3e) ", it->relerror);
        }
        if(it->itertype == DW_ITER_ABS && it->iter > 0)
        {
            printf("(");
            if(it->error > it->abserror)
            {
                printf(" > ");
            } else {
                printf(" < ");
            }
            printf("%.3e) ", it->abserror);
        }
        fflush(stdout);
    }

    if(s->log != NULL && s->log != stdout)
    {
        if(s->metric == DW_METRIC_MSE)
        {
            fprintf(s->log, "Iteration %3d/%3d, fMSE=%e\n",
                    it->iter+1, it->niter, it->error);
        }
        if(s->metric == DW_METRIC_IDIV)
        {
            fprintf(s->log, "Iteration %3d/%3d, Idiv=%e\n",
                    it->iter+1, it->niter, it->error);
        }
        fflush(s->log);
    }
}
void dw_iterator_free(dw_iterator_t * it)
{
    free(it);
}



dw_opts * dw_opts_new(void)
{
    dw_opts * s = malloc(sizeof(dw_opts));
    s->nThreads = dw_get_threads();

    s->nThreads < 1 ? s->nThreads = 1 : 0;
    s->nIter = 1; /* Always overwritten if used */
    s->maxiter = 250;
    s->err_rel = 0.02;
    s->err_abs = 1; /* Always overwritten if used */
    s->nIter_auto = 1;
    s->imFile = NULL;
    s->psfFile = NULL;
    s->outFile = NULL;
    s->logFile = NULL;
    s->refFile = NULL;
    s->tsvFile = NULL;
    s->iter_type = DW_ITER_REL;
    s->tsv = NULL;
    s->ref = NULL;
    s->prefix = malloc(10*sizeof(char));
    sprintf(s->prefix, "dw");
    s->log = NULL;
    s->verbosity = 1;
    s->showTime = 0;
    s->overwrite = 0;
    s->tiling_maxSize = -1;
    s->tiling_padding = 20;
    s->method = DW_METHOD_SHB;
    s->fun = deconvolve_shb;
    s->iterdump = 0;
    s->relax = 0;
    s->xycropfactor = 0.001;
    s->commandline = NULL;
    s->onetile = 0;
    s->borderQuality = 2;
    s->outFormat = 16; // write 16 bit int
    s->experimental1 = 0;
    s->fulldump = 0;
    s->positivity = 1;
    s->bg = 1e-5; /* Should be strictly positive or pixels will be freezed */
    s->flatfieldFile = NULL;
    s->lookahead = 0;
    s->psigma = 0;
    s->biggs = 1;
    s->eve = 1;
    s->metric = DW_METRIC_IDIV;
    clock_gettime(CLOCK_REALTIME, &s->tstart);
    return s;
}

void dw_show_iter(dw_opts * s, int it, int nIter, float err)
{
    if(s->verbosity > 0){
        printf("\r                                             ");
        if(s->metric == DW_METRIC_MSE)
        {
            printf("\rIteration %3d/%3d, fMSE=%e ", it+1, nIter, err);
        }
        if(s->metric == DW_METRIC_IDIV)
        {
            printf("\rIteration %3d/%3d, Idiv=%e ", it+1, nIter, err);
        }
        fflush(stdout);
    }

    if(s->log != NULL && s->log != stdout)
    {
        if(s->metric == DW_METRIC_MSE)
        {
            fprintf(s->log, "Iteration %3d/%3d, fMSE=%e\n", it+1, nIter, err);
        }
        if(s->metric == DW_METRIC_IDIV)
        {
            fprintf(s->log, "Iteration %3d/%3d, Idiv=%e\n", it+1, nIter, err);
        }
        fflush(s->log);
    }
}


char * gen_iterdump_name(
    __attribute__((unused)) const dw_opts * s,
    int it)
{
    // Generate a name for the an iterdump file
    // at iteration it
    char * name = malloc(100*sizeof(char));
    sprintf(name, "itd%05d.tif", it);
    return name;
}

int64_t int64_t_max(int64_t a, int64_t b)
{
    if( a > b)
        return a;
    return b;
}

void dw_opts_free(dw_opts ** sp)
{
    dw_opts * s = sp[0];
    dw_nullfree(s->imFile);
    dw_nullfree(s->psfFile);
    dw_nullfree(s->outFile);
    dw_nullfree(s->logFile);
    dw_nullfree(s->flatfieldFile);
    dw_nullfree(s->prefix);
    dw_nullfree(s->commandline);
    dw_nullfree(s->ref);
    dw_nullfree(s->refFile);
    dw_nullfree(s->tsvFile);
    if(s->tsv != NULL)
    {
        fclose(s->tsv);
    }
    free(s);
}

void dw_opts_fprint(FILE *f, dw_opts * s)
{
    f == NULL ? f = stdout : 0;

    fprintf(f, "> Settings:\n");
    fprintf(f, "image:  %s\n", s->imFile);
    if(s->flatfieldFile != NULL)
    {
        fprintf(f, "flat field: %s\n", s->flatfieldFile);
    }
    fprintf(f, "psf:    %s\n", s->psfFile);
    fprintf(f, "output: %s\n", s->outFile);
    fprintf(f, "log file: %s\n", s->logFile);
    fprintf(f, "nIter:  %d\n", s->nIter);
    fprintf(f, "nThreads: %d\n", s->nThreads);
    fprintf(f, "verbosity: %d\n", s->verbosity);
    fprintf(f, "background level: %f\n", s->bg);

    switch(s->method)
    {
    case DW_METHOD_EVE:
        fprintf(f, "method: Biggs (EVE)\n");
        break;
    case DW_METHOD_AVE:
        fprintf(f, "method: Biggs and Andrews (AVE)\n");
        break;
    case DW_METHOD_RL:
        fprintf(f, "method: Richardson-Lucy (RL)\n");
        break;
    case DW_METHOD_ID:
        fprintf(f, "method: Identity, doing nothing (ID)\n");
        break;
    case DW_METHOD_SHB:
        fprintf(f, "method: Scaled Heavy Ball (SHB)\n");
        break;
#ifdef OPENCL
    case DW_METHOD_SHBCL:
        fprintf(f, "method: Scaled Heavy Ball (SHB)\n");
        break;
#endif
    }
    switch(s->metric)
    {
    case DW_METRIC_MSE:
        fprintf(f, "metric: MSE\n");
        break;
    case DW_METRIC_IDIV:
        fprintf(f, "metric: Idiv\n");
        break;
    }
    switch(s->iter_type)
    {
    case DW_ITER_ABS:
        fprintf(f, "Stopping on absolute error: %e or %d iterations\n",
                s->err_abs, s->maxiter);
        break;
    case DW_ITER_REL:
        fprintf(f, "Stopping on relative error: %e or %d iterations\n",
                s->err_rel, s->maxiter);
        break;
    case DW_ITER_FIXED:
        fprintf(f, "Stopping after %d iterations\n",
                s->nIter);
        break;
    }
    if(s->psigma > 0)
    {
        fprintf(f, "pre-filtering enabled, sigma = %f\n", s->psigma);
    }
    if(s->overwrite == 1)
    { fprintf(f, "overwrite: YES\n"); } else
    { fprintf(f, "overwrite: NO\n"); }

    if(s->tiling_maxSize > 0)
    {
        fprintf(f, "tiling, maxSize: %d\n", s->tiling_maxSize);
        fprintf(f, "tiling, padding: %d\n", s->tiling_padding);
    } else {
        fprintf(f, "tiling: OFF\n");
    }
    fprintf(f, "XY crop factor: %f\n", s->xycropfactor);
    if(s->relax > 0)
    {
        fprintf(f, "PSF relaxation: %f\n", s->relax);
    }
    fprintf(f, "Output Format: ");
    switch(s->outFormat){
    case 16:
        fprintf(f, "16 bit integer\n");
        break;
    case 32:
        fprintf(f, "32 bit float\n");
        break;
    default:
        fprintf(f, "ERROR: Unknown\n");
        break;
    }

    fprintf(f, "Border Quality: ");
    switch(s->borderQuality){
    case 0:
        fprintf(f, "0 No boundary handling\n");
        break;
    case 1:
        fprintf(f, "1 Somewhere between 0 and 2\n");
        break;
    case 2:
        fprintf(f, "2 Minimal boundary artifacts\n");
        break;
    default:
        ;
    }
    fprintf(f, "FFT lookahead: %d", s->lookahead);

    if(s->onetile == 1)
    {
        fprintf(f, "DEBUG OPTION: ONETILE = TRUE (only first tile will be deconvolved)\n");
    }
    fprintf(f, "\n");
}

void warning(FILE * fid)
{
    //fprintf(fid, ANSI_UNDERSCORE " ! " ANSI_COLOR_RESET );
    fprintf(fid, " ! ");
    return;
}


void fulldump(dw_opts * s, float * A, size_t M, size_t N, size_t P, char * name)
{
    /* Write A to disk as fulldump_<name>.tif if s->fulldump = 1 */
    if(s->fulldump != 1)
    {
        return;
    }
    assert(name != NULL);

    if(A != NULL)
    {
        printf("Dumping to %s\n", name);
        fim_tiff_write_float(name, A, NULL, M, N, P);
    }
    return;
}

void dw_fprint_info(FILE * f, dw_opts * s)
{
    f == NULL ? f = stdout : 0;
    fprintf(f, "deconwolf: '%s' PID: %d\n", deconwolf_version, (int) getpid());
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        fprintf(f, "PWD: %s\n", cwd);
    }

    if(s->commandline != NULL)
    {
        fprintf(f, "CMD: %s\n", s->commandline);
    }

#ifdef GIT_VERSION
    fprintf(f, "GIT_VERSION: '%s'\n", GIT_VERSION);
#endif

#ifdef CC_VERSION
    fprintf(f, "COMPILER: '%s'\n", CC_VERSION);
#endif

    fprintf(f, "BUILD_DATE: '%s'\n'", __DATE__);
#ifdef CUDA
    fprintf(f, "FFT Backend: 'cuFFT\n");
#else
    fprintf(f, "FFT Backend: '%s'\n", fftwf_version);
#endif
    fprintf(f, "TIFF: '%s'\n", TIFFGetVersion());

#ifndef WINDOWS
    char * user = getenv("USER");
    if(user != NULL)
    { fprintf(f, "USER: '%s'\n", user); }
#endif

#ifndef WINDOWS
    char * hname = malloc(1024*sizeof(char));
    if(gethostname(hname, 1023) == 0)
    {
        fprintf(f, "HOSTNAME: '%s'\n", hname);
        free(hname);
    }
#endif

#ifdef _OPENMP
    fprintf(f, "OpenMP: YES\n");
#endif

#ifdef OPENCL
    fprintf(f, "OpenCL: YES\n");
#endif

    fprintf(f, "\n");
    fflush(f);
    return;
}

static void getCmdLine(int argc, char ** argv, dw_opts * s)
{
    // Copy the command line to s->commandline
    int lcmd=0;
    for(int kk = 0; kk<argc; kk++)
    {
        lcmd += strlen(argv[kk]);
    }
    lcmd += argc+2;
    s->commandline = malloc(lcmd);
    int pos = 0;
    for(int kk = 0; kk<argc; kk++)
    {
        sprintf(s->commandline+pos, "%s ", argv[kk]);
        pos += strlen(argv[kk])+1;
    }
    s->commandline[pos-1] = '\0';
    //  printf("argc: %d cmd: '%s'\n", argc, s->commandline);
}


void dw_argparsing(int argc, char ** argv, dw_opts * s)
{

    getCmdLine(argc, argv, s);


    struct option longopts[] = {
        { "bg",        required_argument, NULL, 'b' },
        { "threads",   required_argument, NULL, 'c' },
        { "tsv",       required_argument, NULL, 'd' },
        { "abserror",  required_argument, NULL, 'e' },
        { "prefix",    required_argument, NULL, 'f' },
        { "times",     no_argument,       NULL, 'g' },
        { "help",      no_argument,       NULL, 'h' },
        { "iterdump",  no_argument,       NULL, 'i' },
        { "relerror",  required_argument, NULL, 'j' },
        { "verbose",   required_argument, NULL, 'l' },
        { "method",    required_argument, NULL, 'm' },
        { "iter",      required_argument, NULL, 'n' },
        { "out",       required_argument, NULL, 'o' },
        { "tilepad",   required_argument, NULL, 'p' },
        { "relax",     required_argument, NULL, 'r' },
        { "tilesize",  required_argument, NULL, 's' },
        { "test",      no_argument,       NULL, 't' },
        { "version",   no_argument,       NULL, 'v' },
        { "overwrite", no_argument,       NULL, 'w' },
        { "xyfactor",  required_argument, NULL, 'x' },
        { "bq",        required_argument, NULL,  'B' },
        { "flatfield", required_argument, NULL,  'C' },
        { "fulldump",  no_argument,       NULL,  'D' },
        { "float",     no_argument,       NULL,  'F' },
        { "niterdump", required_argument, NULL,  'I' },
        { "lookahead", required_argument, NULL,  'L' },
        { "mse",       no_argument,       NULL,  'M' },
        { "maxiter",   required_argument, NULL,  'N' },
        { "ref",       required_argument, NULL,  'R' },
        { "onetile",   no_argument,       NULL,  'T' },
        { "nopos",     no_argument,       NULL,  'P' },
        { "psigma",    required_argument, NULL,  'Q' },
        { "expe1",     no_argument,       NULL,  'X' },
        { NULL,           0,                 NULL,   0   }
    };


    int known_method = 1;
    int ch;
    int prefix_set = 0;
    while((ch = getopt_long(argc, argv,
                            "a:b:c:f:ghil:m:n:o:p:r:s:tvwx:B:C:DFI:L:MR:TPQ:X:",
                            longopts, NULL)) != -1)
    {
        switch(ch) {
        case 'C':
            s->flatfieldFile = strdup(optarg);
            break;
        case 'b':
            s->bg = atof(optarg);
            break;
        case 'd': /* diagnostics */
            s->tsvFile = strdup(optarg);
            break;
        case 'e': /* --abserror */
            s->err_abs = atof(optarg);
            s->iter_type = DW_ITER_ABS;
            break;
        case 'g':
            s->showTime = 1;
            break;
        case 'j': /* --relerror */
            s->err_rel = atof(optarg);
            s->iter_type = DW_ITER_REL;
            break;
        case 'L':
            s->lookahead = atoi(optarg);
            break;
        case 'M':
            s->metric = DW_METRIC_MSE;
            break;
        case 'N':
            s->maxiter = atoi(optarg);
            break;
        case 'P':
            s->positivity = 0;
            printf("Turning off positivity constraint!\n");
            break;
        case 'Q':
            s->psigma = atof(optarg);
            break;
        case 'D':
            s->fulldump = 1;
            break;
        case 'F':
            s->outFormat = 32;
            break;
        case 'B':
            s->borderQuality = atoi(optarg);
            break;
        case 'v':
            dw_fprint_info(NULL, s);
            exit(0);
            break;
        case 'h':
            dw_usage(argc, argv, s);
            exit(0);
            break;
        case 'o':
            s->outFile = malloc(strlen(optarg)+1);
            strcpy(s->outFile, optarg);
            break;
        case 'n':
            s->nIter = atoi(optarg);
            s->iter_type = DW_ITER_FIXED;
            break;
        case 'c':
            s->nThreads = atoi(optarg);
            break;
        case 'i':
            s->iterdump = 1;
            break;
        case 'I':
            s->iterdump = atoi(optarg);
            break;
        case 'l':
            s->verbosity = atoi(optarg);
            break;
        case 't':
            dw_unittests();
            exit(0);
            break;
        case 's':
            s->tiling_maxSize = atoi(optarg);
            break;
        case 'p':
            s->tiling_padding = atoi(optarg);
            break;
        case 'w':
            s->overwrite = 1;
            break;
        case 'f':
            free(s->prefix);
            s->prefix = malloc(strlen(optarg) + 1);
            strcpy(s->prefix, optarg);
            prefix_set = 1;
            break;
        case 'm':
            known_method = 0;
            if(strcmp(optarg, "eve") == 0)
            {
                s->method = DW_METHOD_EVE;
                if(prefix_set == 0)
                {
                    sprintf(s->prefix, "eve");
                }
                s->fun = &deconvolve_eve;
                known_method = 1;
            }
            if(strcmp(optarg, "ave") == 0)
            {
                s->method = DW_METHOD_AVE;
                if(prefix_set == 0)
                {
                    sprintf(s->prefix, "ave");
                }
                s->fun = &deconvolve_ave;
                known_method = 1;
            }
            if(strcmp(optarg, "rl") == 0)
            {
                s->method = DW_METHOD_RL;
                if(prefix_set == 0)
                {
                    sprintf(s->prefix, "rl");
                }
                s->fun = &deconvolve_rl;
                known_method = 1;
            }
            if(strcmp(optarg, "id") == 0)
            {
                s->method = DW_METHOD_ID;
                if(prefix_set == 0)
                {
                    sprintf(s->prefix, "id");
                }
                s->fun = &deconvolve_identity;
                known_method = 1;
            }
            if(strcmp(optarg, "shb") == 0)
            {
                s->method = DW_METHOD_SHB;
                if(prefix_set == 0)
                {
                    sprintf(s->prefix, "shb");
                }
                s->fun = &deconvolve_shb;
                known_method = 1;
            }
            #ifdef OPENCL
            if(strcmp(optarg, "shbcl") == 0)
            {
                s->method = DW_METHOD_SHBCL;
                if(prefix_set == 0)
                {
                    sprintf(s->prefix, "shbcl");
                }
                s->fun = &deconvolve_shb_cl;
                known_method = 1;
            }
            #endif
            if(known_method == 0)
            {
                fprintf(stderr, "--method %s is unknown. Please specify ave, eve (default), shb, rl or id\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        case 'r':
            s->relax = atof(optarg);
            break;
        case 'x':
            s->xycropfactor = atof(optarg);
            if(s->xycropfactor > 1 || s->xycropfactor < 0)
            {
                fprintf(stderr, "The crop factor in x and y has to be => 0 and < 1\n");
                exit(1);
            }
            break;
        case 'R':
            s->refFile = strdup(optarg);
            break;
        case 'T':
            s->onetile = 1;
            break;
        case 'X':
            s->experimental1 = 1;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }

    if((s->biggs < 0) | (s->biggs > 3))
    {
        printf("Invalid settings to --biggs, please specify 0, 1, 2 or 3\n");
        exit(1);
    }

    /* Take care of the positional arguments */
    if(optind + 2 != argc)
    {
        printf("Deconwolf: To few input arguments.\n");
        printf("See `%s --help` or `man dw`.\n", argv[0]);
        exit(1);
    }

#ifdef WINDOWS
    /* TODO, see GetFullPathNameA in fileapi.h */
    s->imFile = strdup(argv[optind]);
#else
    s->imFile = realpath(argv[optind], 0);
#endif

    if(s->imFile == NULL)
    {
        fprintf(stderr, "ERROR: Can't read %s\n", argv[optind]);
        exit(1);
    }

#ifdef WINDOWS
    s->psfFile = strdup(argv[++optind]);
#else
    s->psfFile = realpath(argv[++optind], 0);
#endif

    if(s->psfFile == NULL)
    {
        fprintf(stderr, "ERROR: Can't read %s\n", argv[optind]);
        exit(1);
    }

    if(s->outFile == NULL)
    {
        char * dirc = strdup(s->imFile);
        char * basec = strdup(s->imFile);
        char * dname = dirname(dirc);
        char * bname = basename(basec);
        s->outFile = malloc(strlen(dname) + strlen(bname) + strlen(s->prefix) + 10);
        sprintf(s->outFile, "%s/%s_%s", dname, s->prefix, bname);
        free(dirc);
        free(basec);
    }

    if(! s->iterdump)
    {
        if( s->overwrite == 0 && dw_file_exist(s->outFile))
        {
            printf("%s already exist. Doing nothing\n", s->outFile);
            exit(0);
        }
    }

    if(s->nThreads < 1)
    {
        printf("Invalid number of threads (%d), "
               "please verify your command line\n", s->nThreads);
        exit(EXIT_FAILURE);
    }

    s->logFile = malloc(strlen(s->outFile) + 10);
    sprintf(s->logFile, "%s.log.txt", s->outFile);

    if(s->tsvFile != NULL)
    {
        s->tsv = fopen(s->tsvFile, "w");
        if(s->tsv == NULL)
        {
            fprintf(stderr, "Failed to open %s for writing\n", s->tsvFile);
            exit(EXIT_FAILURE);
        }
        fprintf(s->tsv, "iteration\ttime\tKL\n");
    }

    //  printf("Options received\n"); fflush(stdout);
}


void fsetzeros(const char * fname, size_t N)
/* Create a new file and fill it with N bytes of zeros
 */
{
    size_t bsize = 1024*1024;
    char * buffer = malloc(bsize);
    memset(buffer, 0, bsize);
    FILE * fid = fopen(fname, "wb");
    size_t written = 0;
    while(written + bsize < N)
    {
        fwrite(buffer, bsize, 1, fid);
        written += bsize;
    }
    //  printf("\r %zu / %zu \n", written, N); fflush(stdout);

    fwrite(buffer, N-written, 1, fid);
    fclose(fid);
    free(buffer);
}


void fprint_peakMemory(FILE * fout)
{
    size_t pm = get_peakMemoryKB();

    if(fout == NULL) fout = stdout;
    fprintf(fout, "peakMemory: %zu kiB\n", pm);

    return;
}

void benchmark_write(dw_opts * s, int iter, double fMSE,
                     const float * x0, // current guess of work size
                     const int64_t M, const int64_t N, const int64_t P,
                     const int64_t wM, const int64_t wN, const int64_t wP)
{
    if(s->tsv == NULL)
    {
        return;
    }

    float * x = fim_subregion(x0, wM, wN, wP, M, N, P);

    size_t MNP = M*N*P;
    double KL = 0;
    if(s->ref != NULL)
    {
        for(size_t kk = 0; kk<MNP; kk++)
        {
            if(s->ref[kk] > 0)
            {
                KL += log( x[kk]/s->ref[kk]) * x[kk];
            }
        }
    }
    fftw_free(x);
    struct timespec tnow;
    clock_gettime(CLOCK_REALTIME, &tnow);
    double time = clockdiff(&tnow, &s->tstart);
    fprintf(s->tsv, "%d\t%f\t%f\t%f\n", iter, time, fMSE, KL);

    return;
}

float getErrorX(const float * restrict y, const float * restrict g, const int64_t M, const int64_t N, const int64_t P, const int64_t wM, const int64_t wN, const int64_t wP)
{
    /* Same as getError with the difference that G is expanded to MxNxP */
    if(M > wM || N > wN || P > wP)
    {
        fprintf(stderr,"Something is funky with the dimensions of the images.\n");
        exit(-1);
    }

    double e = 0;
    for(size_t c = 0; c < (size_t) P; c++)
    {
        for(size_t b = 0; b < (size_t) N; b++)
        {
            for(size_t a = 0; a < (size_t) M; a++)
            {
                double yval = y[a + b*wM + c*wM*wN];
                double gval = g[a + b*wM + c*wM*wN];
                e+=pow(yval-gval, 2);
            }
        }
    }
    e/=(M*N*P);
    return (float) e;
}


float getError_idiv(const float * restrict y, const float * restrict g,
                    const int64_t M, const int64_t N, const int64_t P,
                    const int64_t wM, const int64_t wN, const int64_t wP)
{
    /* Csiszar’s I-Divergence between the input, y, and the forward propagated
     * current guess */

    if(M > wM || N > wN || P > wP)
    {
        fprintf(stderr, "Something is funky with the dimensions of the images.\n");
        exit(-1);
    }
    float idiv = 0;
#pragma omp parallel for reduction(+: idiv)
    for(int64_t c = 0; c<P; c++)
    {
        for(int64_t b = 0; b<N; b++)
        {
            for(int64_t a = 0; a<M; a++)
            {
                float obs =  y[a + b*wM + c*wM*wN];
                float est = g[a + b*M + c * M*N];
                if(est > 0)
                {
                    idiv += obs*log(obs/est) - obs + est;
                }
            }
        }
    }

    float idiv_mean = idiv / (float) (M*N*P);
    return idiv_mean;
}

/* Return the "error" or distance between the input image and the
   current guess convolved with the PSF. Also known as the forward
   error */
float getError(const float * restrict y, const float * restrict g,
               const int64_t M, const int64_t N, const int64_t P,
               const int64_t wM, const int64_t wN, const int64_t wP,
               dw_metric metric)
{
    /* Idiv is a better distance measurement but it takes longer to
       calculate due to the log function.
       These calculations needs to be done with double precision or the results
       will look different depending on the number of threads used (cancellation)
    */

    float error = 0;
    switch(metric)
    {
    case DW_METRIC_MSE:
        error = get_fMSE(y, g, M, N, P, wM, wN, wP);
        break;
    case DW_METRIC_IDIV:
        error = get_fIdiv(y, g, M, N, P, wM, wN, wP);
        break;
    }

    return error;
}

float get_fMSE(const float * restrict y, const float * restrict g,
               const int64_t M, const int64_t N, const int64_t P,
               const int64_t wM, const int64_t wN, const int64_t wP)
{
    /* Mean squared error between the input, y, and the forward propagated
     * current guess */

    if(M > wM || N > wN || P > wP)
    {
        fprintf(stderr, "Something is funky with the dimensions of the images.\n");
        exit(-1);
    }
    double e = 0;
    //#pragma omp parallel for reduction(+: e) shared(y, g)
    for(int64_t c = 0; c<P; c++)
    {
        for(int64_t b = 0; b<N; b++)
        {
            for(int64_t a = 0; a<M; a++)
            {
                double yval = y[a + b*wM + c*wM*wN];
                double gval = g[a + b*M + c * M*N];
                e += pow(yval-gval, 2);
            }
        }
    }

    double mse = e / (double) (M*N*P);
    return (double) mse;
}

float get_fIdiv(const float * restrict y, const float * restrict g,
                const int64_t M, const int64_t N, const int64_t P,
                const int64_t wM, const int64_t wN, const int64_t wP)
{
    /* Mean squared error between the input, y, and the forward propagated
     * current guess */

    if(M > wM || N > wN || P > wP)
    {
        fprintf(stderr, "Something is funky with the dimensions of the images.\n");
        exit(-1);
    }
    double I = 0;
#pragma omp parallel for reduction(+: I) shared(y, g)
    for(int64_t c = 0; c<P; c++)
    {
        for(int64_t b = 0; b<N; b++)
        {
            for(int64_t a = 0; a<M; a++)
            {
                double yval = y[a + b*wM + c*wM*wN];
                double gval = g[a + b*M + c * M*N];
                if(yval > 0 && gval > 0)
                {
                    I += gval*log(gval/yval) - (gval-yval);
                }
            }
        }
    }

    double mI = I / (double) (M*N*P);
    return (float) mI;
}


void putdot(const dw_opts *s)
{
    if(s->verbosity > 0)
    {
        printf(ANSI_COLOR_GREEN "." ANSI_COLOR_RESET);
        fflush(stdout);
    }
    return;
}

float getError_ref(const float * restrict y,
                   const float * restrict g,
                   int64_t M, int64_t N, int64_t P,
                   int64_t wM, int64_t wN, int64_t wP)
{
    /* Note: this is just for comparison with getError, not used
     * in production
     **/
    if(M > wM || N > wN || P > wP)
    {
        fprintf(stderr, "Something is funky with the dimensions of the images.\n");
        exit(-1);
    }


    double e = 0;
    for(int64_t c = 0; c<P; c++)
    {
        for(int64_t b = 0; b<N; b++)
        {
            for(int64_t a = 0; a<M; a++)
            {
                double yval = y[a + b*wM + c*wM*wN];
                double gval = g[a + b*M + c * M*N];
                e+=pow(yval-gval, 2);
            }
        }
    }

    e/=(M*N*P);
    return (float) e;
}



void dw_usage(__attribute__((unused)) const int argc, char ** argv, const dw_opts * s)
{
    printf("usage: %s [<options>] image.tif psf.tif\n", argv[0]);

    printf("\n");
    printf(" Options:\n");
    printf(" --version\n\t Show version info\n");
    printf(" --help\n\t Show this message\n");
    printf(" --out file\n\t Specify output image name\n");
    printf(" --iter N\n\t Specify the number of iterations to use (default: %d)\n", s->nIter);
    printf(" --threads N\n\t Specify the number of threads to use\n");
    printf(" --verbose N\n\t Set verbosity level (default: %d)\n", s->verbosity);
    printf(" --test\n\t Run unit tests\n");
    printf(" --tilesize N\n\t Enables tiling mode and sets the largest tile size to N voxels in x and y.\n");
    printf(" --tilepad N\n\t Sets the tiles to overlap by N voxels in tile mode (default: %d)\n", s->tiling_padding);
    printf(" --prefix str\n\t Set the prefix of the output files (default: '%s')\n", s->prefix);
    printf(" --overwrite\n\t Allows deconwolf to overwrite already existing output files\n");
    printf(" --relax F\n\t Multiply the central pixel of the PSF by F. (F>1 relaxation)\n");
    printf(" --xyfactor F\n\t Discard outer planes of the PSF with sum < F of the central. Use 0 for no cropping.\n");
    printf(" --bq Q\n\t Set border handling to 0 'none', 1 'compromise', or 2 'normal' which is default\n");
    printf(" --float\n\t Set output format to 32-bit float (default is 16-bit int) and disable scaling\n");
    printf(" --bg l\n\t Set background level, l\n");
    printf(" --biggs N\n\t Set how agressive the Biggs acceleration should be. 0=off, 1=low/default, 2=intermediate, 3=max\n");
    printf(" --eve\n\t Use Biggs Exponential Vector Extrapolation (EVE)\n");
    printf(" --flatfield image.tif\n\t"
           " Use a flat field correction image. Deconwolf will divide each plane of the\n\t"
           " input image, pixel by pixel, by this correction image.\n");
    printf(" --lookahead N"
           "\n\t Try to do a speed-for-memory trade off by using a N pixels larger"
           "\n\t job size that is better suited for FFT.\n");
    printf("\n");
    printf("max-projections of tif files can be created with:\n");
    printf("\t%s maxproj image.tif\n", argv[0]);
    printf("\tsee %s maxproj --help\n", argv[0]);

    printf("\n");
    printf("Web page: https://www.github.com/elgw/deconwolf/\n");
}




double clockdiff(struct timespec* end, struct timespec * start)
{
    double elapsed = (end->tv_sec - start->tv_sec);
    elapsed += (end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return elapsed;
}


void testfinite(float * x, size_t N)
{
    for(size_t kk = 0; kk<N; kk++)
    {
        if(!isfinite(x[kk]))
        {
            printf("Not finite\n");
            exit(1);
        }
    }
}





float * psf_autocrop_centerZ(float * psf, int64_t * pM, int64_t * pN, int64_t * pP,  // psf and size
                             dw_opts * s)
{

    const int64_t m = pM[0];
    const int64_t n = pN[0];
    const int64_t p = pP[0];

    const int64_t midm = (m-1)/2;
    const int64_t midn = (n-1)/2;
    const int64_t midp = (p-1)/2;

    //  printf("m: %d, n:%d, p:%d\n", m, n, p);
    //  printf("midm: %d, midn: %d, midp: %d\n", midm, midn, midp);

    float maxvalue = -INFINITY;
    int64_t maxp = -1;

    for(int64_t pp = 0; pp<p; pp++)
    {
        size_t idx = midm + midn*m + pp*m*n;
        if(psf[idx] > maxvalue)
        {
            maxp = pp;
            maxvalue = psf[idx];
        }
    }

    if(maxp == midp)
    {
        if(s->verbosity > 2)
        {
            printf("PSF is Z-centered :)\n");
        }
        return psf;
    }


    int64_t m0 = 0, m1 = m-1;
    int64_t n0 = 0, n1 = n-1;
    int64_t p0 = maxp, p1 = maxp;

    while(p0 > 1 && p1+2 < p)
    {
        p0--; p1++;
    }
    if(s->verbosity > 0)
    {
        printf("PSF has %" PRId64 " slices\n", p);
        printf("brightest at plane %" PRId64 "\n", maxp);
        printf("Selecting Z-planes: %" PRId64 " -- %" PRId64 "\n", p0, p1);
    }

    fprintf(s->log, "Selecting Z-planes %" PRId64 " -- %" PRId64 "\n", p0, p1);

    float * psf_cropped = fim_get_cuboid(psf, m, n, p,
                                         m0, m1, n0, n1, p0, p1);
    fftwf_free(psf);
    pP[0] = p1-p0+1;
    return psf_cropped;

}

float * psf_autocrop_byImage(float * psf,/* psf and size */
                             int64_t * pM, int64_t * pN, int64_t * pP,
                             int64_t M, int64_t N, int64_t P, /* image size */
                             dw_opts * s)
{

    /* Purpose: Crop the PSF if it is more
       than 2x the size of the image in any dimension. */

    const int64_t m = pM[0];
    const int64_t n = pN[0];
    const int64_t p = pP[0];


    // Optimal size
    int64_t mopt = (M-1)*2 + 1;
    int64_t nopt = (N-1)*2 + 1;
    int64_t popt = (P-1)*2 + 1;

    if(p < popt)
    {
        warning(stdout);
        fprintf(stdout, "The PSF has only %" PRId64
                " slices, %" PRId64 " would be better.\n", p, popt);
        fprintf(s->log, "WARNING: The PSF has only %" PRId64 " slices, %" PRId64 " would be better.\n", p, popt);
        return psf;
    }

    if((p % 2) == 0)
    {
        fprintf(stderr, "Error: The PSF should have odd number of slices\n");
        fprintf(stderr, "Possibly it will be auto-cropped wrong\n");
    }


    if(m > mopt || n > nopt || p > popt)
    {
        int64_t m0 = 0, m1 = m-1;
        int64_t n0 = 0, n1 = n-1;
        int64_t p0 = 0, p1 = p-1;
        if(m > mopt)
        {
            m0 = (m-mopt)/2;
            m1 = m1-(m-mopt)/2;
        }
        if(n > nopt)
        {
            n0 = (n-nopt)/2;
            n1 = n1-(n-nopt)/2;
        }
        if(p > popt)
        {
            p0 = (p-popt)/2;
            p1 = p1-(p-popt)/2;
        }
        if(s->verbosity > 2)
        {
            printf("! %" PRId64 " %" PRId64 " : %" PRId64 " %" PRId64 " : %" PRId64 " %" PRId64 "\n", m0, m1, n0, n1, p0, p1);
        }
        float * psf_cropped = fim_get_cuboid(psf, m, n, p,
                                             m0, m1, n0, n1, p0, p1);
        fftwf_free(psf);

        pM[0] = m1-m0+1;
        pN[0] = n1-n0+1;
        pP[0] = p1-p0+1;

        if(s->verbosity > 0)
        {
            fprintf(stdout, "PSF Z-crop [%" PRId64 " x %" PRId64 " x %" PRId64 "] -> [%" PRId64 " x %" PRId64 " x %" PRId64 "]\n",
                    m, n, p, pM[0], pN[0], pP[0]);
        }
        fprintf(s->log, "PSF Z-crop [%" PRId64 " x %" PRId64 " x %" PRId64 "] -> [%" PRId64 " x %" PRId64 " x %" PRId64 "]\n",
                m, n, p, pM[0], pN[0], pP[0]);

        return psf_cropped;
    } else {
        return psf;
    }
}

float * psf_autocrop_XY(float * psf, int64_t * pM, int64_t * pN, int64_t * pP,  // psf and size
                        __attribute__((unused))    int64_t M,
                        __attribute__((unused)) int64_t N,
                        __attribute__((unused)) int64_t P, // image size
                        dw_opts * s)
{
    int64_t m = pM[0];
    int64_t n = pN[0];
    int64_t p = pP[0];

    // Find the y-z plane with the largest sum
    float maxsum = 0;
    for(int64_t xx = 0; xx<pM[0]; xx++)
    {
        float sum = 0;
        for(int64_t yy = 0; yy<pN[0]; yy++)
        {
            for(int64_t zz = 0; zz<pP[0]; zz++)
            {
                sum += psf[xx + yy*pM[0] + zz*pM[0]*pN[0]];
            }
        }
        sum > maxsum ? maxsum = sum : 0;
    }

    //  printf("X maxsum %f\n", maxsum);

    int64_t first=-1;
    float sum = 0;

    while(sum < s->xycropfactor * maxsum)
    {
        first++;
        sum = 0;
        int64_t xx = first;
        for(int64_t yy = 0; yy<pN[0]; yy++)
        {
            for(int64_t zz = 0; zz<pP[0]; zz++)
            {
                sum += psf[xx + yy*pM[0] + zz*pM[0]*pN[0]];
            }
        }
    }

    if(first < 1)
    {
        if(s->verbosity > 1)
        {
            printf("PSF X-crop: Not cropping\n");
        }
        return psf;
    }

    // Benchmark FFTW to figure out a good compromise between cropping
    // and a size that will be fast for fftw.
    if(s->lookahead > 0)
    {
        fprintf(s->log, "> Testing lookahead up to %d\n", s->lookahead);
        printf("Suggested PSF size: %" PRId64 "\n", pM[0] - 2*first);
        int64_t imsize = M + (pM[0]-1) - 2*first;
        int64_t imsize_max = M + (pM[0]-1);
        printf("Gives images size: %" PRId64 " -- %" PRId64 "\n", imsize, imsize_max);
        if(imsize_max - imsize > s->lookahead)
        {
            imsize_max = imsize+s->lookahead;
        }
        printf("lookahead gives: images size: %" PRId64 " -- %" PRId64 "\n", imsize, imsize_max);

        double * times = fft_bench_1d(imsize, imsize_max, 100);
        int bestAdd = 0;
        double bestTime = INFINITY;
        for(int kk = imsize; kk<= imsize_max; kk+=2)
        {
            if(times[kk-imsize] < bestTime)
            {
                bestTime = times[kk-imsize];
                bestAdd = (kk-imsize)/2;
            }
            printf("job x-size: %d: %f\n", kk, times[kk-imsize]);
        }
        fflush(stdout);
        double gain = times[0]/bestTime;
        fprintf(s->log, "Lookahead gain: %f\n", gain);
        if(s->verbosity > 1)
        {
            printf("Lookahead gain: %f\n", gain);
        }
        free(times);
        first = first-bestAdd;
    }

    /* Allocated with fftwf */
    float * crop = fim_get_cuboid(psf, pM[0], pN[0], pP[0],
                                  first, pM[0] - first -1,
                                  first, pN[0] - first -1,
                                  0, pP[0]-1);
    pM[0] -= 2*first;
    pN[0] -= 2*first;

    if(s->verbosity > 0)
    {
        fprintf(stdout, "PSF XY-crop [%" PRId64 " x %" PRId64 " x %" PRId64 "] -> [%" PRId64 " x %" PRId64 " x %" PRId64 "]\n",
                m, n, p, pM[0], pN[0], pP[0]);
    }
    fprintf(s->log, "PSF XY-crop [%" PRId64 " x %" PRId64 " x %" PRId64 "] -> [%" PRId64 " x %" PRId64 " x %" PRId64 "]\n",
            m, n, p, pM[0], pN[0], pP[0]);

    fftwf_free(psf);
    return crop;
}

float * psf_autocrop(float * psf, int64_t * pM, int64_t * pN, int64_t * pP,  // psf and size
                     int64_t M, int64_t N, int64_t P, // image size
                     dw_opts * s)
{
    float * p = psf;
    // p = psf_autocrop_centerZ(p, pM, pN, pP, s);
    assert(pM[0] > 0);
    /* Crop the PSF if it is larger than necessary */
    p = psf_autocrop_byImage(p, pM, pN, pP, M, N, P, s);
    assert(pM[0] > 0);
    // Crop the PSF by removing outer planes that has very little information
    p = psf_autocrop_XY(p, pM, pN, pP, M, N, P, s);
    assert(pM[0] > 0);
    assert(p != NULL);
    return p;
}


int deconvolve_tiles(const int64_t M, const int64_t N, const int64_t P,
                     const float * restrict psf, const int64_t pM, const int64_t pN, const int64_t pP,
                     dw_opts * s)
{

    tiling * T = tiling_create(M,N,P, s->tiling_maxSize, s->tiling_padding);
    if( T == NULL)
    {
        fprintf(stderr, "Tiling failed, please check your settings\n");
        exit(1);
    }

    if( T->nTiles == 1)
    {
        fprintf(stderr, "\n"
                "ERROR: Only one tile! Please omit the `--tilesize` parameter if "
                "that is what you intended to to or decrease the value if you "
                "want to process the image in tiles."
                "\n\n");
        exit(1);
    }

    if(s->verbosity > 0)
    {
        printf("-> Divided the [%" PRId64 " x %" PRId64 " x %" PRId64 "] image into %d tiles\n", M, N, P, T->nTiles);
    }

    /* Output image initialize as zeros
     * will be updated block by block
     */
    char * tfile = malloc(strlen(s->outFile)+10);
    sprintf(tfile, "%s.raw", s->outFile);

    if(s->verbosity > 0)
    {
        printf("Initializing %s to 0\n", tfile); fflush(stdout);
    }
    fsetzeros(tfile, (size_t) M* (size_t) N* (size_t) P*sizeof(float));

    char * imFileRaw = malloc(strlen(s->imFile) + 10);
    sprintf(imFileRaw, "%s.raw", s->imFile);

    if(s->verbosity > 0)
    {
        printf("Dumping %s to %s (for quicker io)\n", s->imFile, imFileRaw);
    }

    fim_tiff_to_raw(s->imFile, imFileRaw);
    if(0){
        printf("Writing to imdump.tif\n");
        fim_tiff_from_raw("imdump.tif", M, N, P, imFileRaw);
    }

    //fim_tiff_write_zeros(s->outFile, M, N, P);
    if(s->verbosity > 0)
    {
        printf("\n"); fflush(stdout);
    }

    int nTiles = T->nTiles;
    if(s->onetile == 1)
    {
        nTiles = 1;
        fprintf(s->log, "DEBUG: only the first tile to be deconvolved\n");
        fprintf(stdout, "DEBUG: only the first tile to be deconvolved\n");
    }

    for(int tt = 0; tt < nTiles; tt++)
    {
        // Temporal copy of the PSF that might be cropped to fit the tile
        float * tpsf = fim_copy(psf, pM*pN*pP);
        int64_t tpM = pM, tpN = pN, tpP = pP;

        if(s->verbosity > 0)
        {
            printf("-> Processing tile %d / %d\n", tt+1, T->nTiles);
            fprintf(s->log, "-> Processing tile %d / %d\n", tt+1, T->nTiles);
        }

        //    tictoc
        //   tic
        //float * im_tile = tiling_get_tile_tiff(T, tt, s->imFile);
        float * im_tile = tiling_get_tile_raw(T, tt, imFileRaw);
        //    toc(tiling_get_tile_tiff)

        int64_t tileM = T->tiles[tt]->xsize[0];
        int64_t tileN = T->tiles[tt]->xsize[1];
        int64_t tileP = T->tiles[tt]->xsize[2];

        if(0)
        {
            printf("writing to tiledump.tif\n");
            fim_tiff_write("tiledump.tif", im_tile, NULL, tileM, tileN, tileP);
            getchar();
        }

        fim_normalize_sum1(tpsf, tpM, tpN, tpP);

        tpsf = psf_autocrop(tpsf, &tpM, &tpN, &tpP,
                            tileM, tileN, tileP, s);

        fim_normalize_sum1(tpsf, tpM, tpN, tpP);

        float * dw_im_tile = s->fun(im_tile, tileM, tileN, tileP, // input image and size
                                    tpsf, tpM, tpN, tpP, // psf and size
                                    s);
        fftwf_free(im_tile);
        tiling_put_tile_raw(T, tt, tfile, dw_im_tile);
        free(dw_im_tile);
        // free(tpsf);
    }
    tiling_free(T);
    free(T);

    if(s->verbosity > 2)
    {
        printf("converting %s to %s\n", tfile, s->outFile);
    }
    fim_tiff_from_raw(s->outFile, M, N, P, tfile);

    if(s->verbosity < 5)
    {
        remove(tfile);
    } else {
        printf("Keeping %s for inspection, remove manually\n", tfile);
    }
    free(tfile);

    remove(imFileRaw);
    free(imFileRaw);
    return 0;
}



void timings()
{
    printf("-> Timings\n");
    tictoc
        int64_t M = 1024, N = 1024, P = 50;
    float temp = 0;

    tic
        usleep(1000);
    toc(usleep_1000)

        tic
        float * V = malloc(M*N*P*sizeof(float));
    toc(malloc)

        float * A = malloc(M*N*P*sizeof(float));

    tic
        memset(V, 0, M*N*P*sizeof(float));
    toc(memset)
        memset(A, 0, M*N*P*sizeof(float));


    tic
        for(size_t kk = 0; kk < (size_t) M*N*P; kk++)
        {
            A[kk] = (float) rand()/(float) RAND_MAX;
        }
    toc(rand)

        // ---
        tic
        fftwf_plan p = fftwf_plan_dft_r2c_3d(P, N, M,
                                             V, NULL,
                                             FFTW_WISDOM_ONLY | FFTW_MEASURE);
    fftwf_destroy_plan(p);
    toc(fftwf_plan_create_and_destroy)


        // ---
        tic
        fim_flipall(V, A, M, N, P);
    toc(fim_flipall)

        // ---
        tic
        temp = alpha_ave(V, A, M*N*P, 1);
    toc(biggs_alpha)
        V[0]+= temp;

    // ---
    tic
        float e1 = getError_ref(V, A, M, N, P, M, N, P);
    toc(getError_ref)
        V[0]+= e1;

    tic
        float e2 = getError(V, A, M, N, P, M, N, P, DW_METRIC_MSE);
    toc(getError)
        V[0]+=e2;

    printf("e1: %f, e2: %f, fabs(e1-e2): %f\n", e1, e1, fabs(e1-e2));

    // ---
    tic
        float * S1 = fim_subregion(V, M, N, P, M-1, N-1, P-1);
    toc(fim_subregion)

        tic
        float * S2 = fim_subregion_ref(V, M, N, P, M-1, N-1, P-1);
    toc(fim_subregion_ref)
        printf("S1 - S2 = %f\n", getError(S1, S1, M-1, N-1, P-1, M-1, N-1, P-1, DW_METRIC_MSE));
    free(S1);
    free(S2);

    // ---
    tic
        fim_insert(V, M, N, P, A, M-1, N-1, P-1);
    toc(fim_subregion)

        tic
        fim_insert_ref(V, M, N, P, A, M-1, N-1, P-1);
    toc(fim_subregion_ref)

        // ---


        ((float volatile *)V)[0] = V[0];
    printf("V[0] = %f\n", V[0]);
    free(A);
    free(V);
}

void dw_unittests()
{
    fprint_peakMemory(NULL);
    timings();

    //fim_ut();
    fim_tiff_ut();
    fft_ut();
    printf("done\n");
}

void show_time(FILE * f)
{
    f == NULL ? f = stdout : 0;
    time_t now = time(NULL);
    char * tstring = ctime(&now);
    fprintf(f, "%s\n", tstring);
}

float * psf_makeOdd(float * psf, int64_t * pM, int64_t * pN, int64_t *pP)
{
    // Expand the psf so that it had odd dimensions it if doesn't already have that
    int64_t m = pM[0];
    int64_t n = pN[0];
    int64_t p = pP[0];
    int reshape = 0;
    if(m % 2 == 0)
    { m++; reshape = 1;}
    if(n % 2 == 0)
    { n++; reshape = 1;}
    if(p % 2 == 0)
    { p++; reshape = 1;}

    if(reshape == 0)
    {  return psf; }
    // printf("%d %d %d -> %d %d %d\n", pM[0], pN[0], pP[0], m, n, p);
    float * psf2 = fim_zeros(m*n*p);
    fim_insert(psf2, m, n, p, psf, pM[0], pN[0], pP[0]);
    free(psf);
    pM[0] = m;
    pN[0] = n;
    pP[0] = p;
    return psf2;
}


void dcw_init_log(dw_opts * s)
{
    s->log = fopen(s->logFile, "w");
    assert(s->log != NULL);
    show_time(s->log);
    dw_opts_fprint(s->log, s);
    dw_fprint_info(s->log, s);
}

void dcw_close_log(dw_opts * s)
{
    fprint_peakMemory(s->log);
    show_time(s->log);
    fclose(s->log);
}

double get_nbg(float * I, size_t N, float bg)
{ // count the number of pixels in the image
    // that are set to the background level
    double nbg = 0;
    for(size_t kk = 0; kk<N; kk++)
    {
        if(I[kk] == bg)
        {
            nbg++;
        }
    }
    return nbg;
}




void flatfieldCorrection(dw_opts * s, float * im, int64_t M, int64_t N, int64_t P)
{
    printf("Experimental: applying flat field correction using %s\n", s->flatfieldFile);
    ttags * T = ttags_new();
    int64_t m = 0, n = 0, p = 0;
    float * C = fim_tiff_read(s->flatfieldFile, T, &m, &n, &p, s->verbosity);
    ttags_free(&T);

    assert(m == M);
    assert(n == N);
    assert(p == 1);

    // TODO:
    // check that it is positive

    for(int64_t zz = 0; zz<P; zz++)
    {
        for(size_t pos = 0; pos < (size_t) M*N; pos++)
        {
            im[M*N*zz + pos] /= C[pos];
        }
    }
    free(C);
}


void prefilter(dw_opts * s,
               float * im, int64_t M, int64_t N, int64_t P,
               float * psf, int64_t pM, int64_t pN, int64_t pP)
{

    if(s->psigma <= 0)
    {
        return;
    }
    fim_gsmooth(im, M, N, P, s->psigma);
    fim_gsmooth(psf, pM, pN, pP, s->psigma);
    return;
}

int dw_run(dw_opts * s)
{
    struct timespec tstart, tend;
    clock_gettime(CLOCK_REALTIME, &tstart);
    dcw_init_log(s);


#ifdef _OPENMP
#ifdef MKL
    mkl_set_num_threads(s->nThreads);
    fprintf(s->log, "Set the number of MKL threads to %d\n", s->nThreads);
#else
    omp_set_num_threads(s->nThreads);
    fprintf(s->log, "Set the number of OMP threads to %d\n", s->nThreads);
    /* Fastest of static, dynamic and guided in limited tests */
    omp_set_dynamic(false);
    omp_set_schedule(omp_sched_static, 0);
    fprintf(s->log, "Using static scheduling for OMP\n");
    /* Better on heavily loaded machine? */
    //omp_set_schedule(omp_sched_guided, 0);
#endif
#endif

    if(s->verbosity > 1)
    {
        dw_opts_fprint(NULL, s);
        printf("\n");
    }

    s->verbosity > 1 ? dw_fprint_info(NULL, s) : 0;

    logfile = stdout;

    fim_tiff_init();
    fim_tiff_set_log(s->log);
    if(s->verbosity > 2)
    {
        fim_tiff_set_log(s->log);
    }


    int64_t M = 0, N = 0, P = 0;
    if(fim_tiff_get_size(s->imFile, &M, &N, &P))
    {
        printf("Failed to open %s\n", s->imFile);
        return -1;
    } else {
        if(s->verbosity > 3)
        {
            printf("Got image info from %s\n", s->imFile);
        }
    }

    if(s->verbosity > 1)
    {
        printf("Image dimensions: %" PRId64 " x %" PRId64 " x %" PRId64 "\n", M, N, P);
    }

    int tiling = 0;
    if(s->tiling_maxSize > 0 && (M > s->tiling_maxSize || N > s->tiling_maxSize))
    {
        tiling = 1;
    }


    float * im = NULL;
    ttags * T = ttags_new();

    if(tiling == 0)
    {
        if(s->verbosity > 0 )
        {
            printf("Reading %s\n", s->imFile);
        }

        im = fim_tiff_read(s->imFile, T, &M, &N, &P, s->verbosity);
        if(s->verbosity > 4)
        {
            printf("Done reading\n"); fflush(stdout);
        }
        if(fim_min(im, M*N*P) < 0)
        {
            printf("min value of the image is %f, shifting to 0\n", fim_min(im, M*N*P));
            fim_set_min_to_zero(im, M*N*P);
            if(fim_max(im, M*N*P) < 1000)
            {
                fim_mult_scalar(im, M*N*P, 1000/fim_max(im, M*N*P));
            }
        }
        if(im == NULL)
        {
            fprintf(stderr, "Failed to open %s\n", s->imFile);
            exit(1);
        }

        if(s->refFile != NULL)
        {
            int64_t rM = 0, rN = 0, rP = 0;
            s->ref = fim_tiff_read(s->refFile, NULL, &rM, &rN, &rP, s->verbosity);
            if(s->ref == NULL)
            {
                fprintf(stderr, "Failed to open %s\n", s->imFile);
                exit(1);
            }
            if( (rM != M) || (rN != N) || (rP != P) )
            {
                fprintf(stderr, "Image and reference image does not have matching size\n");
                exit(1);
            }
        }

    }

    // Set up the string for the TIFFTAG_SOFTWARE
    char * swstring = malloc(1024);
    sprintf(swstring, "deconwolf %s", deconwolf_version);
    ttags_set_software(T, swstring);
    free(swstring);

    // fim_tiff_write("identity.tif", im, M, N, P);

    int64_t pM = 0, pN = 0, pP = 0;
    float * psf = NULL;
    if(1){
        if(s->verbosity > 0)
        {
            printf("Reading %s\n", s->psfFile);
        }
        psf = fim_tiff_read(s->psfFile, NULL, &pM, &pN, &pP, s->verbosity);
        if(psf == NULL)
        {
            fprintf(stderr, "Failed to open %s\n", s->psfFile);
            exit(1);
        }
    } else {
        pM = 3; pN = 3; pP = 3;
        psf = malloc(27*sizeof(float));
        memset(psf, 0, 27*sizeof(float));
        psf[13] = 1;
    }

    //psf = psf_makeOdd(psf, &pM, &pN, &pP);
    //psf = psf_autocrop_centerZ(psf, &pM, &pN, &pP, s);

    if(fim_maxAtOrigo(psf, pM, pN, pP) == 0)
    {
        /* It might still be centered between pixels */
        warning(stdout);
        printf("The PSF is not centered!\n");
    }

    // Possibly the PSF will be cropped even more per tile later on

    fim_normalize_sum1(psf, pM, pN, pP);
    if(1)
    {
        psf = psf_autocrop(psf, &pM, &pN, &pP,
                           M, N, P, s);
    }

    if(s->relax > 0)
    {
        // Note: only works with odd sized PSF
        fprintf(s->log, "Relaxing the PSF by %f\n", s->relax);
        if(s->verbosity > 0)
        {
            printf("Relaxing the PSF\n");
        }

        size_t mid = (pM-1)/2 + (pN-1)/2*pM + (pP-1)/2*pM*pN;
        //printf("mid: %f -> %f\n", psf[mid], psf[mid]+s->relax);
        psf[mid] += s->relax;
        fim_normalize_sum1(psf, pM, pN, pP);
        if(s->verbosity > 2)
        {
            double spsf = 0;
            for(int64_t kk = 0 ; kk<pM*pN*pP; kk++)
            {
                spsf+=psf[kk];
            }
            printf("Sum of PSF: %f\n", spsf);
        }
    }

    if(s->verbosity > 0)
    {
        printf("Output: %s(.log.txt)\n", s->outFile);
    }

    myfftw_start(s->nThreads, s->verbosity, s->log);

    float * out = NULL;

    if(tiling)
    {
        if(s->flatfieldFile != NULL)
        {
            warning(stdout);
            printf("Flat-field correction can't be used in tiled mode\n");
        }
        deconvolve_tiles(M, N, P, psf, pM, pN, pP, // psf and size
                         s);// settings
        fftwf_free(psf);
    } else {
        fim_normalize_sum1(psf, pM, pN, pP);
        if(s->flatfieldFile != NULL)
        {
            flatfieldCorrection(s, im, M, N, P);
        }

        /* Pre filter by psigma */
        prefilter(s, im, M, N, P, psf, pM, pN, pP);

        /* Note: psf is freed bu the deconvolve_* functions*/
        out = s->fun(im, M, N, P, // input image and size
                     psf, pM, pN, pP, // psf and size
                     s);// settings
        psf = NULL;
    }

    if(tiling == 0)
    {
        fftwf_free(im);


        if(out == NULL)
        {
            if(s->verbosity > 0)
            {
                printf("Nothing to write to disk :(\n");
            }
        } else
        {
            double nZeros = get_nbg(out, M*N*P, s->bg);
            fprintf(s->log, "%f%% pixels at bg level in the output image.\n", 100*nZeros/(M*N*P));
            if(s->verbosity > 1)
            {
                printf("%f%% pixels at bg level in the output image.\n", 100*nZeros/(M*N*P));
                printf("Writing to %s\n", s->outFile); fflush(stdout);
            }

            //    floatimage_normalize(out, M*N*P);
            if(s->outFormat == 32)
            {
                if(s->iterdump)
                {
                    char * outFile = gen_iterdump_name(s, s->nIter);
                    fim_tiff_write_float(outFile, out, T, M, N, P);
                    free(outFile);
                } else {
                    fim_tiff_write_float(s->outFile, out, T, M, N, P);
                }
            } else {
                if(s->iterdump)
                {
                    char * outFile = gen_iterdump_name(s, s->nIter);
                    fim_tiff_write(outFile, out, T, M, N, P);
                    free(outFile);
                } else {
                    fim_tiff_write(s->outFile, out, T, M, N, P);
                }
            }
        }
    }

    ttags_free(&T);

    if(s->verbosity > 1)
    {
        printf("Finalizing "); fflush(stdout);
    }

    if(out != NULL) fftwf_free(out);
    myfftw_stop();


    clock_gettime(CLOCK_REALTIME, &tend);
    fprintf(s->log, "Took: %f s\n", timespec_diff(&tend, &tstart));
    dcw_close_log(s);

    if(s->verbosity > 1) fprint_peakMemory(NULL);

    if(s->verbosity > 0)
    {
        printf("Done!\n");
        /*
          stdout = fdopen(0, "w");
          setlocale(LC_CTYPE, "");
          //wchar_t star = 0x2605;
          wchar_t star = 0x2728;
          wprintf(L"%lc Done!\n", star);
          stdout = fdopen(0, "w");
        */
    }
    dw_opts_free(&s);

    return 0;
}

fftwf_complex * initial_guess(const int64_t M, const int64_t N, const int64_t P,
                                 const int64_t wM, const int64_t wN, const int64_t wP)
{
    /* Create initial guess: the fft of an image that is 1 in MNP and 0 outside
     * M, N, P is the dimension of the microscopic image
     *
     * Possibly more stable to use the mean of the input image rather than 1
     */

    assert(wM >= M); assert(wN >= N); assert(wP >= P);

    float * one = fim_zeros(wM*wN*wP);

#pragma omp parallel for shared(one)
    for(int64_t cc = 0; cc < P; cc++) {
        for(int64_t bb = 0; bb < N; bb++) {
            for(int64_t aa = 0; aa < M; aa++) {
                one[aa + wM*bb + wM*wN*cc] = 1;
            }
        }
    }
    //  writetif("one.tif", one, wM, wN, wP);

    fftwf_complex * Fone = fft(one, wM, wN, wP);

    fftwf_free(one);
    return Fone;
}
