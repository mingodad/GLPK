/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   glpk_real.h
 * Author: Domingo Alvarez Duarte mingodad@gmail.com
 *
 * Created on September 23, 2020, 11:36 AM
 */

#ifndef GLPK_REAL_H
#define GLPK_REAL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef glp_double
    #ifdef WITH_LONG_DOUBLE
        #define glp_double long double
        #define GLP_DBL_EPSILON LDBL_EPSILON
        #define GLP_DBL_MAX LDBL_MAX
        #define GLP_DBL_MIN LDBL_MIN
        #define GLP_DBL_DIG LDBL_DIG
        #define GLP_DBL_FMT_G "Lg"
        #define GLP_DBL_FMT_F "Lf"
        #define GLP_DBL_FMT_E "LE"
        #define GLP_DBL_FMT_e "Le"
    #else
        #define glp_double double
        #define GLP_DBL_EPSILON DBL_EPSILON
        #define GLP_DBL_MAX DBL_MAX
        #define GLP_DBL_MIN DBL_MIN
        #define GLP_DBL_DIG DBL_DIG
        #define GLP_DBL_FMT_G "g"
        #define GLP_DBL_FMT_F "f"
        #define GLP_DBL_FMT_E "E"
        #define GLP_DBL_FMT_e "e"
    #endif
    /*common definitions*/
    #define GLP_PIVOT_TOL 0.10
    #define GLP_EPS_TOL GLP_DBL_EPSILON
    #define GLP_D_EPS 1e-9
    #define GLP_IFU_TOL 1e-5
    #define GLP_SGF_TOL 1e-20
    #define GLP_VPQ_TOL 1e-5
    #define GLP_ERR_TOL 1e-10
    #define GLP_SMCP_TOL_BND  1e-7
    #define GLP_SMCP_TOL_DJ   1e-7
    #define GLP_IOCP_TOL_INT  1e-5
    #define GLP_IOCP_TOL_OBJ   1e-7
    #define GLP_BFCP_TOL_EPS  1e-6
    #define GLP_BFCP_TOL_DUAL   1e-5
    #define GLP_COL_IMPL_BOUNDS_TOL   1e-6
    #define GLP_CHK_ROW_COL_BOUNDS_TOL1   1e-3
    #define GLP_CHK_ROW_BOUNDS_TOL2   1e-12
    #define GLP_CHK_COL_BOUNDS_TOL2   1e-10
    #define GLP_CHK_IOS_BOUNDS_TOL   1e-4
    #define GLP_CHK_COVER_TOL   1e-5
    #define GLP_CHK_COV_GEN_TOL   1e-3
    #define GLP_BRNCH_DRTOM_TOL   1e-3
    #define GLP_BRNCH_DRTOM_TOL2   1e-6
    #define GLP_BEST_NODE_TOL   1e-10
    #define GLP_SOLVE_NE_TOL   1e-4
    #define GLP_IPM_MAIN_TOL   1e-8
    #define GLP_IPM_MAIN_TOL2   1e5
    #define GLP_IPM_MAIN_TOL3   1e6
    #define GLP_FPUMP_TOL   1e-6
    #define GLP_GMI_CUT_TOL   1e+05
    #define GLP_GMI_CUT_TOL2   1e-10
    #define GLP_GMI_CUT_TOL3   1e-12
    #define GLP_GMI_GEN_TOL   1e-03
    #define GLP_GMI_GEN_TOL2   1e+03
    #define GLP_WCLICK_TOL   1e-5
    #define GLP_CPS_RELAX    0.07
    #define GLP_CPS_TOL_BND  1e-7
    #define GLP_CPS_TOL_DJ   1e-7
    #define GLP_CPS_TOL_PIV  1e-9
    #define GLP_CPS_OBJ_LL   -GLP_DBL_MAX
    #define GLP_CPS_OBJ_UL   +GLP_DBL_MAX
    #define GLP_CPS_IT_LIM   -1
    #define GLP_CPS_TM_LIM   -1.0
    #define GLP_CPS_TOL_INT  1e-5
    #define GLP_CPS_TOL_OBJ  1e-7
    #define GLP_LPX_RE_MAX_9  1e-9
    #define GLP_LPX_RE_MAX_6  1e-6
    #define GLP_LPX_RE_MAX_3  1e-3
    #define GLP_MPL_MIN_9  1e-9
    #define GLP_WARMUP_EPS(x)  (1e-6 + 1e-9 * fabs(x))
    #define GLP_MKEQ_EPS(x)  (1e-9 + 1e-12 * fabs(x))
    #define GLP_DBL_THOUSAND  1000.0
    #define GLP_SCALE_RANGE_MIN  0.10
    #define GLP_SCALE_RANGE_MAX  10.0
#endif

#ifdef __cplusplus
}
#endif

#endif /* GLPK_REAL_H */

