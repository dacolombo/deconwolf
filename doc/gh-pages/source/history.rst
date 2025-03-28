Version History
===============

0.4.5
-----

- New: support for reading and writing Numpy npy files in many
  places. If the input file ends with ``.npy`` so will the output file.
- Change: Not setting ``TIFFTAG_SUBFILETYPE`` and ``TIFFTAG_PAGENUMBER`` when
  writing tif files since it does not seem to be needed.
- Fix: ``dw maxproj`` truncates ImageJ metadata to remove the 3D
  information so that Python/tifffile no longer issues warnings.
- Fix: ``--scaling s`` works when tiling is used.
- Fix: ``--float`` work when tiling is used.

0.4.4_rc4 (windows only)
------------------------

- Fixes issue #83
- Fixed crashed with ``--method id``.

0.4.4
-----

- Fixed Windows-only issue #77, #80 (Thanks to `WisdomUfondu
  <https://github.com/WisdomUfondu>`__) and #81. I.e. tiling should work
  and the random freezes when using the GPU should be gone (hopefully!).

- Switched to rst/sphinx documentation from markdown.

- Fixed issues when using ``--psigma`` with low values.

- Fixed: Issue #38: 'use quoting when saving the command line to the log files'
- More verbose using ``--verbose 2`` and above.

- Fixed: memory leak in ``fft_block_of_ones``.

- Reporting physical peak memory usage (VmHWM) besides the virtual
  peak memory usage (VmPeak).

- Parts of the function in dw can be built into a library using the
  ``src/CMAkeLists.txt`` file. This is still experimental and to be
  documented.

- Almost clean under ``-fanalyzer``, at least when ``NDEBUG`` is not defined.

- Clean under ``-fsantize=address`` both with and without
  tiling. When ``--gpu`` is used the results are not clean, partly due to `VkFFT issues 194 <https://github.com/DTolm/VkFFT/issues/194>`__.

Updates to ``dw nuclei``:

- Still very experimental.

- Uses the random forest implementation in `trafo <https://github.com/elgw/trafo>`__

- Supports 3 classes, instead of 2 (use red, green and blue in the annotated images).

- New command line interface.

Updates to ``dw dots``:

- Fixed that ``--dog_ls`` and ``--dog_as`` works for ``dw dots``.

- Fixed: erroneously reported the same sub pixel values for both the x and y component.

Windows specific updates:

- Added 'b' to ``fopen`` calls in many places where that caused problems.


0.4.3
-----

-  Fixed `issue #70 <https://github.com/elgw/deconwolf/issues/70>`__
   which affected windows builds only.

-  Dw will now give a error message if the log file can’t be opened
   instead of quietly crashing.

-  Disabled some debug code that caused dw to write the file
   (``one.tif``) every time that is was run. It has been doing so since
   version 0.4.1.

.. _section-2:

0.4.2
-----

-  VkFFT upgraded from v1.3.3 to v1.3.4.
-  Fixed an issue preventing anything than ``--bq 0`` to be used on
   Windows (one array was not properly initialized to 0).

.. _section-3:

0.4.1
-----

-  Fixes the GPU memory leak reported in `issue
   #62 <https://github.com/elgw/deconwolf/issues/62>`__.

.. _section-4:

0.4.0
-----

**TLDR** New features and bug fixes. Update if you are on version 0.3.7
or below.

-  Added pre-generated header files under ``/src/kernels/`` so that
   ``make kernels`` is not needed during the build process. Solves
   `issue #55 <https://github.com/elgw/deconwolf/issues/55>`__

Updates to ``dw dots``: - Updated LoG filter and maxima finding routine
gives a speedup > 2x. - Setting the filter sizes based on the optical
parameters, assuming diffraction limited dots. - Addition: Can fit dots
using a 3D Gaussian while minimizing the :math:`-\log` likelihood
assuming Poissonian noise. For this purpose the
Broyden-Fletcher-Goldfarb-Shanno algoritm (bfgs2 in GSL) is used.

Older
-----

.. _section-5:

0.3.9
^^^^^

-  Added the **–offset** option. This will add an offset to the image
   before the deconvolution start, which will be removed before saving
   the result to disk. Initial tests suggests that this can reduce the
   effect of detector noise (non-Poisson) by decreasing the relative
   weight of pixels with low values. See some numerical tests below, on
   the microtubules image:

   .. figure:: images/20240422.png
      :alt: iterations vs mse

      iterations vs mse

   The default value is set to 5 which should be safe in most
   situations, but it is suggested that it is set to the standard
   deviation of the detector noise. To revert to the old behaviour, use
   **–offset 0**.

-  Removed the **–relax** option since it made no sense.

-  Reverted the default start guess to flat, like it was before v 0.3.8.
   The **–start_lp** still seems to produce better images but takes a
   little too long to initialize for being a default method. Might still
   be the default in future version when there is a GPU implementation.

.. _section-6:

0.3.8
^^^^^

-  Fixed crashes when trying to read a non tif file.

-  For systems with multiple GPUs or OpenCL compatible devices it is now
   possible to select which to use with **–cldevice**. To figure out
   which that are available it is simplest to use ``clinfo``.
   Alternatively call dw with **–verbose 2** or above.

-  Removed depreciated makefile for freebsd as it is no longer needed.

-  Removed anything related to meson as it is no longer needed.

-  Using the ``PRI*`` macros from ``inttype.h``, especially ``PRIu64``
   for ``uint64_t`` and ``PRId64`` for ``int64_t`` to get rid of some
   warnings under MacOS.

-  Changed the initial guess for all methods. It is now set to be a low
   pass filtered version of the input image. Experiments suggests that
   this was better than the previous approach where a flat image was
   used. To use the previous default, please use **–start_flat**.

-  Added the option to start from the input image with the flag
   **–start_id**. This might be a good option when only a few iterations
   are used since but can lead to more shot noise.

-  Added a noise filter for the input image which can be enabled with
   **–psigma s**, where s controls the filter shape. This is implemented
   as follows: 1) the Anscombe transform is applied 2) the transformed
   image is filtered with a isotropic Gaussian kernel with sigma=s. 3)
   The inverse transform is applied. Using this with moderate values of
   s leads to better results on synthetic images. For example on the
   microtubules image the best results were achieved with **–psigma
   0.7**. A value of 0 is the same as not using the filter.

Results when testing on the microtubules image shown below.

.. figure:: images/20240419.png
   :alt: iterations vs mse

   iterations vs mse

.. _section-7:

0.3.7
^^^^^

-  Deconwolf compiles as a native windows program using clang. So far
   the binaries are only smoke tested since the main target is linux.

-  **dw_bw** use OpenMP and does not rely on pthreads any more (for
   portability reasons).

-  Removed the AVE and EVE methods since they don’t add anything over
   SHB.

-  Added a ``CMakeList.txt`` for building with cmake.

-  Added ``--gpu`` which at the moment is equivalent to
   ``--method shbcl2`` but a little more mnemonic.

-  Added the ``--periodic`` option which turns on periodic boundary
   conditions, i.e. is equivalent to ``--bq 0``.

.. _section-8:

0.3.6
^^^^^

-  The GPU code path uses in-place transformations as much as possible
   to save a little on the memory usage.

-  Switched to `VkFFT <https://github.com/DTolm/VkFFT>`__ (v1.3.3) as
   the default FFT backend on the GPU. Unless a big regression is found,
   the clFFT code path will most likely not be maintained in future
   versions and be removed.

   To build with GPU acceleration use:

   ::

      make kernels
      make -B VKFFT=1

   As before, you need also to choose ``--method shbcl2`` to use it over
   the CPU implementation.

   Initial tests show a speed up of about 10-30% depending on the image
   size. As a bonus VkFFT will process any sizes while clFFT simply
   refuse to process the tricky ones.

-  Identified that ``cl_idiv_kernel.c`` took a substantial amount of the
   iteration time and rewrote it.

-  Removed the “CUDA” backend since it does not make sense any more.

-  Checks that the min value of the image > 0. Aborts if not.

-  Checks that the max value of the image >= 1. Aborts if not.

.. _section-9:

0.3.5
^^^^^

-  **dw maxproj** There were problems reading the output in MATLAB.
   Updated so that the output image will be written as a single strip.

0.3.4
^^^^^

-  Minor bug fixes which gives a clean build with ``-fanalyzer``.

0.3.3
^^^^^

-  Writing pixel size to output file also when tiling is used.

0.3.2
^^^^^

-  Tested on raspberry pi 4 using 64-bit bookworm.
-  Found a bug in ``fft.c`` where ``memcpy`` was used wrongly (replaced
   by ``memmove``). Strangely that bug never manifested under
   Ubuntu/x86_64.
-  Added **fim_realloc** for aligned reallocs. This function could be
   branched depending on the OS since there are platform specific
   aligned reallocation functions.
-  Header files: Using ``#pragma once`` instead of the
   ``#ifndef file_h_`` pattern.

0.3.1
^^^^^

-  Introduced **fim_malloc** for all allocations that might benefit from
   a stricter alignment than malloc provides by default. Tested with
   ``MADV_HUGEPAGE`` for the allocations but the results are
   inconclusive (but it uses more RAM when enabled). Cleared all uses of
   ``fftw_free`` and ``fftw_malloc``.

0.3.0
^^^^^


-  Respects the NO_COLOR environmental variable in accord with
   https://no-color.org/.
-  Fixed correct capping of pixel values when **–scaling** is used.

0.2.9
^^^^^

-  Added the command line option **–scaling** for setting bypassing the
   automatic image scaling in 16-bit output mode.

0.2.8
^^^^^

-  Switched from ``fftw3f_threads`` to ``fftw3f_omp``. This reduced the
   run time by about 10% on a Intel i7-6700K. Can be reverted by
   commenting in/out the corresponding lines in the makefile.
-  Cleaned up the output of ``dw --version``

0.2.7
^^^^^

-  Converted a few minor code paths to execute in parallel by OpenMP
   directives.


0.2.6
^^^^^

-  Using ISO 8601 in log files, e.g., ``2023-02-14T11:14:14``.

0.2.5
^^^^^

-  Added the **–xyz** option to **dw maxproj**, for creating max
   projections along the three axes and collecting them on a single 2D
   image.


0.2.4
^^^^^

-  **dw –help** now shows the additional commands/modules available.
-  Reading 16-bit tif files with **TIFFReadEncodedStrip** instead of
   **TIFFReadRawStrip**. Some programs saves tiff files in other ways :)
-  Added the command psf-STED for 3D STED PSFs. Use at your own risk.
-  Building with meson is temporarily broken and to be fixed.
-  Fixed dw chashing when combining –method rl with –iterdump
-  Setting the background level automatically to min(image) unless
   specified with **–bg**.


0.2.3
^^^^^

-  Fixed some errors introduced in v 0.2.2, especially the **dw
   maxproj** was broken.
-  added the subcommand **dw merge**. To be used to merge single
   z-planes into a 3D volume.


0.2.2
^^^^^

-  Can deconvolve using clFFT, when compiled with **OPENCL=1** two new
   methods appear, **–method shbcl** and **–shbcl2**, the first using
   clFFT only for the Fourier transforms, the latter using OpenCL for
   the whole deconvolution procedure. Uses quite much GPU memory which
   is something to improve upon in future version, possibly by switching
   to vkFFT.


0.1.1
^^^^^

-  Added experimental **dw imshift** for shifting images, also shift
   estimation using normalized cross correlation with **dw imshift –ref
   file.tif**. Might be extended to basic tiling etc.


0.1.0
^^^^^

-  Implements the ‘Scaled Heavy Ball’. More memory efficient than eve
   and about the same speed and image quality. Might become the default
   method.
-  Reorganization of code with one file per deconvolution method, RL is
   now separated to an own file which improves readability.
-  The **–method** argument can be used to switch between several
   methods, see **–help** or the man page.
-  Showing Idivergence after each iteration, switch back to MSE with
   **–mse**
-  Cleaned up the text written to the terminal, notably any warnings
   from libtiff now go to the log file.
-  OMP is set to use as many cores as FFTW.
-  Added OMP directives to a few more loops.
-  Using static OMP schedule.
-  Introduces the **–tsv** argument to save information per iteration to
   a separate tsv file for easier plotting and analysis.
-  Three different stopping criteria: Relative error (default) Fixed
   number of iterations or at an absolute error.

.. _section-23:

0.0.26
^^^^^^

-  **dw maxproj** works with file that are not in the current folder.
-  Fixed **–iterdump** not always working.


0.0.25
^^^^^^

-  Builds with cuFFT on Linux, use ``make CUFFT=1 -B``, requires a CUDA
   compatible GPU and of course the cuFFT library installed.


0.0.24
^^^^^^

-  Tested on CentOS, install both with make and meson.
-  Fixed a memory leak with the **–tilesize** option causing crashed
   sometimes.


0.0.23
^^^^^^

-  Added ‘meson.build’ files in order for deconwolf to be built by `The
   Meson Build system <https://mesonbuild.com/>`__, tested to work on
   both Ubuntu 21.10 and MacOS (on x86_64 hardware).
-  Added a small test image under ``demo/`` together with a **makefile**
   to deconvolve it.
-  Added `pseudo code <PSEUDOCODE.md>`__ for the binaries hoping to
   planning to replace this by a properly typeset and more detailed
   document.
-  Aborting if the number of threads is set < 1.
-  The algorithm is still unchanged since v 0.0.20.


0.0.22
^^^^^^

-  Fixed double free-bug in tiling mode.


0.0.21
^^^^^^

-  Updated documentation and man-pages based on markdown files for
   easier updating.
-  Provides ``makefile-freebsd`` for building on FreeBSD 13.0
-  Changed behavior when too few input arguments are given to only give
   a two-line message.


0.0.20
^^^^^^

-  Changing acceleration technique to use ‘Exponential Vector
   Extrapolation’ (EVE) described in Biggs PhD thesis. Deconvolved
   images get higher MSE but much lower I-div.
-  ‘–xyfactor 0’ does not crash dw anymore.
-  Frees the PSF as soon as not needed to save some memory.
-  Changing the behavior of the progress dots to appear more linear in
   time
-  Changing the non-negative condition to strictly positive in order for
   pixel not to get stuck at 0.
-  Adding the option to turn off Biggs acceleration, i.e. run normal
   Richardson-Lucy with –biggs 0.
-  Will load PSFs that don’t have an odd number of pixels in each
   dimension however that is not recommended.
-  Can be built against Intel MKL (``make MKL=1 ...``), consider that an
   experimental option. 14 percent faster on a small test image, varied
   results on larger images.


0.0.19
^^^^^^

-  Using lanczos5 instead of lanczos3 for the PSF generation. As a
   result GSL_EROUND is not raised for the test cases.
-  Faster PSF generation, using more symmetries.
-  dw_bw can now use more than one thread (wrongly disabled in v
   0.0.18).


0.0.18
^^^^^^

-  Provided install instructions for Windows 10.
-  Fixed some mismatching fftwf_malloc/fftwf_free where they were mixed
   up with malloc/free causing crashes on Windows.
-  Added an experimental src/CMakeLists.txt that can be used when
   building with cmake. It is also possible to cross compile for Windows
   on Linux although it takes some effort to collect the DLL files for
   the dependencies.

0.0.17
^^^^^^

-  Fixed some bugs in the PSF generation code that did affect the
   accuracy of the pixels in the PSF.
-  Stared to use GSL for numerical integration. It remains to change the
   double integral over x-y into something more dynamic.
