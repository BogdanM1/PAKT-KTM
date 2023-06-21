/*

   THIS FILE IS PART OF MUMPS VERSION 4.6.2
   This Version was built on Fri Apr 14 14:59:20 2006


  This version of MUMPS is provided to you free of charge. It is public
  domain, based on public domain software developed during the Esprit IV
  European project PARASOL (1996-1999) by CERFACS, ENSEEIHT-IRIT and RAL. 
  Since this first public domain version in 1999, the developments are
  supported by the following institutions: CERFACS, ENSEEIHT-IRIT, and
  INRIA.

  Main contributors are Patrick Amestoy, Iain Duff, Abdou Guermouche,
  Jacko Koster, Jean-Yves L'Excellent, and Stephane Pralet.

  Up-to-date copies of the MUMPS package can be obtained
  from the Web pages http://www.enseeiht.fr/apo/MUMPS/
  or http://graal.ens-lyon.fr/MUMPS


   THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY
   EXPRESSED OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.


  User documentation of any code that uses this software can
  include this complete notice. You can acknowledge (using
  references [1], [2], and [3] the contribution of this package
  in any scientific publication dependent upon the use of the
  package. You shall use reasonable endeavours to notify
  the authors of the package of this publication.

   [1] P. R. Amestoy, I. S. Duff and  J.-Y. L'Excellent (1998),
   Multifrontal parallel distributed symmetric and unsymmetric solvers,
   in Comput. Methods in Appl. Mech. Eng., 184,  501-520 (2000).

   [2] P. R. Amestoy, I. S. Duff, J. Koster and  J.-Y. L'Excellent,
   A fully asynchronous multifrontal solver using distributed dynamic
   scheduling, SIAM Journal of Matrix Analysis and Applications,
   Vol 23, No 1, pp 15-41 (2001).

   [3] P. R. Amestoy and A. Guermouche and J.-Y. L'Excellent and
   S. Pralet, Hybrid scheduling for the parallel solution of linear
   systems. Parallel Computing Vol 32 (2), pp 136-156 (2006).

*/
#include "zmumps_io_basic.h"


#ifndef _WIN32
#ifdef WITH_PFUNC
#include <pthread.h>
extern pthread_mutex_t zmumps_io_pwrite_mutex;
#endif
extern int* zmumps_io_pfile_pointer_array;
extern int* zmumps_io_current_file;
#else /*_WIN32*/
extern FILE** zmumps_io_current_file;
extern FILE** zmumps_io_pfile_pointer_array;
#endif /*_WIN32*/



extern int zmumps_io_current_file_number;
extern char* zmumps_ooc_file_prefix;
extern char** zmumps_io_pfile_name;
extern int zmumps_io_current_file_position;
extern int zmumps_io_write_pos;
extern int zmumps_io_last_file_opened;
extern int zmumps_elementary_data_size;
extern int zmumps_io_is_init_called;
extern int zmumps_io_myid;
extern int zmumps_io_nb_file;
extern int zmumps_io_flag_async;
extern int zmumps_io_k211;
extern int zmumps_flag_open;
extern int directio_flag;
extern int zmumps_directio_flag;
