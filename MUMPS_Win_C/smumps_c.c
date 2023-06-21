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
/* $Id: smumps_c.c,v 1.22 2006/03/27 16:46:59 jylexcel Exp $ */
/* Written by JYL, march 2002 */
#include "smumps_c.h"
#include <stdio.h>

/* Special case of mapping and nullspace -- allocated from MUMPS */
static F_INT * mapping;
static F_DOUBLE * nullspace;
/* as uns_perm and sym_perm */
static F_INT *sym_perm;
static F_INT *uns_perm;

#ifdef return_scaling
/*
 * Those two are static. They are passed inside smumps_f77 but
 * might also be changed on return by smumps_affect_colsca/rowsca
 */
static F_DOUBLE * colsca_static;
static F_DOUBLE * rowsca_static;
#endif

void smumps_c(SMUMPS_STRUC_C * smumps_par)
{
    /*
     * The following local variables will 
     *  be passed to the F77 interface.
     */
    F_INT *icntl;
    F_DOUBLE2 *cntl;
    F_INT *irn; F_INT *jcn; F_DOUBLE *a;
    F_INT *irn_loc; F_INT *jcn_loc; F_DOUBLE *a_loc;
    F_INT *eltptr, *eltvar; F_DOUBLE *a_elt;
    F_INT *perm_in; F_INT perm_in_avail;
    F_INT *listvar_schur; F_INT listvar_schur_avail;
    F_DOUBLE *schur; F_INT schur_avail;
    F_DOUBLE *rhs; F_DOUBLE *colsca; F_DOUBLE *rowsca;
    F_DOUBLE *rhs_sparse, *sol_loc;
    F_INT *irhs_sparse, *irhs_ptr, *isol_loc;

    F_INT irn_avail, jcn_avail, a_avail, rhs_avail; /* These are actually used
                                                     * as booleans, but we stick
                                                     * to simple types for the
                                                     * C-F77 interface */
    F_INT irn_loc_avail, jcn_loc_avail, a_loc_avail;
    F_INT eltptr_avail, eltvar_avail, a_elt_avail;
    F_INT colsca_avail, rowsca_avail;

    F_INT irhs_ptr_avail, rhs_sparse_avail, sol_loc_avail;
    F_INT irhs_sparse_avail, isol_loc_avail;

    F_INT *info; F_INT *infog;
    F_DOUBLE2 *rinfo; F_DOUBLE2 *rinfog;


    /* Other local variables */

    F_INT idummy; F_INT *idummyp;
    F_DOUBLE rdummy; F_DOUBLE *rdummyp;

    const static F_INT no = 0;
    const static F_INT yes = 1;

    idummyp = &idummy;
    rdummyp = &rdummy;

#ifdef return_scaling
    /* Don't forget to initialize those two before
     * each call to mumps as we may copy values from
     * old instances otherwise ! */
    colsca_static=0;
    rowsca_static=0;
#endif

    /* Initialize pointers to zero for job == -1 */
    if ( smumps_par->job == -1 )
      { /* job = -1: we just reset all pointers to 0 */
        smumps_par->irn=0; smumps_par->jcn=0; smumps_par->a=0; smumps_par->rhs=0;
        smumps_par->eltptr=0; smumps_par->eltvar=0; smumps_par->a_elt=0; smumps_par->perm_in=0; smumps_par->sym_perm=0; smumps_par->uns_perm=0; smumps_par->irn_loc=0;smumps_par->jcn_loc=0;smumps_par->a_loc=0; smumps_par->listvar_schur=0;smumps_par->schur=0;smumps_par->mapping=0;smumps_par->nullspace=0;smumps_par->colsca=0;smumps_par->rowsca=0; smumps_par->rhs_sparse=0; smumps_par->irhs_sparse=0; smumps_par->sol_loc=0; smumps_par->irhs_ptr=0; smumps_par->isol_loc=0;

        /* Next line initializes scalars to arbitrary values.
         * Some of those will anyway be overwritten during the
         * call to Fortran routine SMUMPS_163 */
        smumps_par->n=0; smumps_par->nz=0; smumps_par->nz_loc=0; smumps_par->nelt=0;smumps_par->instance_number=0;smumps_par->deficiency=0;smumps_par->size_schur=0;smumps_par->lrhs=0; smumps_par->nrhs=0; smumps_par->nz_rhs=0; smumps_par->lsol_loc=0;
 smumps_par->schur_mloc=0; smumps_par->schur_nloc=0; smumps_par->schur_lld=0; smumps_par->mblock=0; smumps_par->nblock=0; smumps_par->nprow=0; smumps_par->npcol=0;
      }

    /*
     * Extract info from the C structure to call the F77 interface. The
     * following macro avoids repeating the same code with risks of errors.
     */

#define EXTRACT_POINTERS(component,dummypointer) \
    if ( smumps_par-> component == 0) \
      { component = dummypointer; \
        component ## _avail = no; }  \
    else  \
      { component = smumps_par-> component; \
        component ## _avail = yes; }

    /*
     * For example, EXTRACT_POINTERS(irn,idummyp) produces the following line of code:

       if (smumps_par->irn== 0) {irn= idummyp;irn_avail = no; } else {  irn  = smumps_par->irn;irn_avail = yes; } ;

     * which says that irn is set to smumps_par->irn except if
     * smumps_par->irn is 0, which means that it is not available.
     */

    EXTRACT_POINTERS(irn,idummyp);
    EXTRACT_POINTERS(jcn,idummyp);
    EXTRACT_POINTERS(rhs,rdummyp);
    EXTRACT_POINTERS(irn_loc,idummyp);
    EXTRACT_POINTERS(jcn_loc,idummyp);
    EXTRACT_POINTERS(a_loc,rdummyp);
    EXTRACT_POINTERS(a,rdummyp);
    EXTRACT_POINTERS(eltptr,idummyp);
    EXTRACT_POINTERS(eltvar,idummyp);
    EXTRACT_POINTERS(a_elt,rdummyp);
    EXTRACT_POINTERS(perm_in,idummyp);
    EXTRACT_POINTERS(listvar_schur,idummyp);
    EXTRACT_POINTERS(schur,rdummyp);

    EXTRACT_POINTERS(colsca,rdummyp);
    EXTRACT_POINTERS(rowsca,rdummyp);

    EXTRACT_POINTERS(rhs_sparse,rdummyp);
    EXTRACT_POINTERS(sol_loc,rdummyp);
    EXTRACT_POINTERS(irhs_sparse,idummyp);
    EXTRACT_POINTERS(isol_loc,idummyp);
    EXTRACT_POINTERS(irhs_ptr,idummyp);

    /* printf("irn_avail,jcn_avail, rhs_avail, a_avail, eltptr_avail, eltvar_avail,a_elt_avail,perm_in_avail= %d %d %d %d %d %d %d \n", irn_avail,jcn_avail, rhs_avail, a_avail, eltptr_avail, eltvar_avail, a_elt_avail, perm_in_avail);*/

    /*
     * Extract integers (input) or pointers that are
     * always allocated (such as ICNTL, INFO, ...)
     */
    /* size_schur = smumps_par->size_schur; */
    /* instance_number = smumps_par->instance_number; */
    icntl = smumps_par->icntl;
    cntl = smumps_par->cntl;
    info = smumps_par->info;
    infog = smumps_par->infog;
    rinfo = smumps_par->rinfo;
    rinfog = smumps_par->rinfog;

    /* Call F77 interface */
    smumps_f77_(&(smumps_par->job), &(smumps_par->sym), &(smumps_par->par), &(smumps_par->comm_fortran),
          &(smumps_par->n), icntl, cntl,
          &(smumps_par->nz), irn, &irn_avail, jcn, &jcn_avail, a, &a_avail,
          &(smumps_par->nz_loc), irn_loc, &irn_loc_avail, jcn_loc, &jcn_loc_avail,
          a_loc, &a_loc_avail,
          &(smumps_par->nelt), eltptr, &eltptr_avail, eltvar, &eltvar_avail, a_elt, &a_elt_avail,
          perm_in, &perm_in_avail,
          rhs, &rhs_avail, info, rinfo, infog, rinfog,
          &(smumps_par->deficiency), &(smumps_par->size_schur), listvar_schur, &listvar_schur_avail, schur,
          &schur_avail, colsca, &colsca_avail, rowsca, &rowsca_avail,
          &(smumps_par->instance_number), &(smumps_par->nrhs), &(smumps_par->lrhs),
          rhs_sparse, &rhs_sparse_avail, sol_loc, &sol_loc_avail, irhs_sparse,
          &irhs_sparse_avail, irhs_ptr, &irhs_ptr_avail, isol_loc,
          &isol_loc_avail, &(smumps_par->nz_rhs), &(smumps_par->lsol_loc)
          , &(smumps_par->schur_mloc)
          , &(smumps_par->schur_nloc)
          , &(smumps_par->schur_lld)
          , &(smumps_par->mblock)
          , &(smumps_par->nblock)
          , &(smumps_par->nprow)
          , &(smumps_par->npcol)
);

    /*
     * mapping and nullspace are usually 0 except if
     * smumps_affect_mapping/smumps_affect_nullspace was called.
     */
    smumps_par->mapping=mapping;
    smumps_par->nullspace=nullspace;

    /* to get permutations computed during analysis */
    smumps_par->sym_perm=sym_perm;
    smumps_par->uns_perm=uns_perm;

#ifdef return_scaling
    /*
     * colsca/rowsca can either be user data or have been
     * modified within mumps by calls to smumps_affect_colsca/rowsca.
     */
    if (colsca_avail == no) {smumps_par->colsca=colsca_static;}
    if (rowsca_avail == no) {smumps_par->rowsca=rowsca_static;}
#endif
}

void smumps_affect_mapping_(F_INT * f77mapping)
{
  mapping = f77mapping;
}
void smumps_nullify_c_mapping_()
{
  mapping = 0;
}

void smumps_affect_nullspace_(F_DOUBLE * f77nullspace)
{
  nullspace = f77nullspace;
}
void smumps_nullify_c_nullspace_()
{
  nullspace = 0;
}

void smumps_affect_sym_perm_(F_INT * f77sym_perm)
{
  sym_perm = f77sym_perm;
}
void smumps_nullify_c_sym_perm_()
{
  sym_perm = 0;
}

void smumps_affect_uns_perm_(F_INT * f77uns_perm)
{
  uns_perm = f77uns_perm;
}
void smumps_nullify_c_uns_perm_()
{
  uns_perm = 0;
}

#ifdef return_scaling
void smumps_affect_colsca_(F_DOUBLE * f77colsca)
{
  colsca_static = f77colsca;
}
void smumps_nullify_c_colsca_()
{
  colsca_static = 0;
}
void smumps_affect_rowsca_(F_DOUBLE * f77rowsca)
{
  rowsca_static = f77rowsca;
}
void smumps_nullify_c_rowsca_()
{
  rowsca_static = 0;
}

#endif
