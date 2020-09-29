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
        #define glp_long_double long double
        #define GLP_DBL_EPSILON LDBL_EPSILON
        #define GLP_DBL_MAX LDBL_MAX
        #define GLP_DBL_MIN LDBL_MIN
        #define GLP_DBL_DIG LDBL_DIG
        #define GLP_DBL_FMT_G "Lg"
        #define GLP_DBL_FMT_F "Lf"
        #define GLP_DBL_FMT_E "LE"
        #define GLP_DBL_FMT_e "Le"
        #define GLP_LDBL_FMT_G GLP_DBL_FMT_G
        #define GLP_LDBL_FMT_F GLP_DBL_FMT_F
        #define GLP_LDBL_FMT_E GLP_DBL_FMT_E
        #define GLP_LDBL_FMT_e GLP_DBL_FMT_e
        #define LITDBL(n) n
    #elif defined(WITH_FLOAT32)
        #define glp_double float
        #define glp_long_double double
        #define GLP_DBL_EPSILON FLT_EPSILON
        #define GLP_DBL_MAX FLT_MAX
        #define GLP_DBL_MIN FLT_MIN
        #define GLP_DBL_DIG FLT_DIG
        #define GLP_DBL_FMT_G "g"
        #define GLP_DBL_FMT_F "f"
        #define GLP_DBL_FMT_E "E"
        #define GLP_DBL_FMT_e "e"
        #define GLP_LDBL_FMT_G GLP_DBL_FMT_G
        #define GLP_LDBL_FMT_F GLP_DBL_FMT_F
        #define GLP_LDBL_FMT_E GLP_DBL_FMT_E
        #define GLP_LDBL_FMT_e GLP_DBL_FMT_e
        #define LITDBL(n) n##dd
    #elif defined(WITH_DEC64)
        #define glp_double _Decimal64
        #define glp_long_double _Decimal128
        #define GLP_DBL_EPSILON DEC64_EPSILON
        #define GLP_DBL_MAX DEC64_MAX
        #define GLP_DBL_MIN DEC64_MIN
        #define GLP_DBL_DIG DEC64_DIG
        #define GLP_DBL_FMT_G "g"
        #define GLP_DBL_FMT_F "f"
        #define GLP_DBL_FMT_E "E"
        #define GLP_DBL_FMT_e "e"
        #define GLP_LDBL_FMT_G GLP_DBL_FMT_G
        #define GLP_LDBL_FMT_F GLP_DBL_FMT_F
        #define GLP_LDBL_FMT_E GLP_DBL_FMT_E
        #define GLP_LDBL_FMT_e GLP_DBL_FMT_e
        #define LITDBL(n) n
    #else
        #define glp_double double
        #define glp_long_double double
        #define GLP_DBL_EPSILON DBL_EPSILON
        #define GLP_DBL_MAX DBL_MAX
        #define GLP_DBL_MIN DBL_MIN
        #define GLP_DBL_DIG DBL_DIG
        #define GLP_DBL_FMT_G "g"
        #define GLP_DBL_FMT_F "f"
        #define GLP_DBL_FMT_E "E"
        #define GLP_DBL_FMT_e "e"
        #if glp_double == double
            #define GLP_LDBL_FMT_L
        #else
            #define GLP_LDBL_FMT_L "L"
        #endif
        #define GLP_LDBL_FMT_G GLP_LDBL_FMT_L GLP_DBL_FMT_G
        #define GLP_LDBL_FMT_F GLP_LDBL_FMT_L GLP_DBL_FMT_F
        #define GLP_LDBL_FMT_E GLP_LDBL_FMT_L GLP_DBL_FMT_E
        #define GLP_LDBL_FMT_e GLP_LDBL_FMT_L GLP_DBL_FMT_e
        #define LITDBL(n) n
    #endif
    /*common definitions*/
    #define GLP_DBL_EMINUS_3 1e-3
    #define GLP_DBL_EPLUS_3 1e+3
    #define GLP_DBL_EMINUS_4 1e-4
    #define GLP_DBL_EMINUS_5 1e-5
    #define GLP_DBL_EPLUS_5 1e+5
    #define GLP_DBL_EMINUS_6 1e-6
    #define GLP_DBL_EPLUS_6 1e+6
    #define GLP_DBL_EMINUS_7 1e-7
    #define GLP_DBL_EMINUS_8 1e-8
    #define GLP_DBL_EMINUS_9 1e-9
    #define GLP_DBL_EMINUS_10 1e-10
    #define GLP_DBL_EMINUS_12 1e-12
    #define GLP_DBL_EMINUS_20 1e-20

    #define glp_float double
    #define glp_double_t glp_double
    #define GLP_PIVOT_TOL 0.10
    #define GLP_EPS_TOL GLP_DBL_EPSILON
    #define GLP_D_EPS GLP_DBL_EMINUS_9
    #define GLP_IFU_TOL GLP_DBL_EMINUS_5
    #define GLP_SGF_TOL GLP_DBL_EMINUS_20
    #define GLP_VPQ_TOL GLP_DBL_EMINUS_5
    #define GLP_ERR_TOL GLP_DBL_EMINUS_10
    #define GLP_SMCP_TOL_BND  GLP_DBL_EMINUS_7
    #define GLP_SMCP_TOL_DJ   GLP_DBL_EMINUS_7
    #define GLP_IOCP_TOL_INT  GLP_DBL_EMINUS_5
    #define GLP_IOCP_TOL_OBJ   GLP_DBL_EMINUS_7
    #define GLP_BFCP_TOL_EPS  GLP_DBL_EMINUS_6
    #define GLP_BFCP_TOL_DUAL   GLP_DBL_EMINUS_5
    #define GLP_COL_IMPL_BOUNDS_TOL   GLP_DBL_EMINUS_6
    #define GLP_CHK_ROW_COL_BOUNDS_TOL1   GLP_DBL_EMINUS_3
    #define GLP_CHK_ROW_BOUNDS_TOL2   GLP_DBL_EMINUS_12
    #define GLP_CHK_COL_BOUNDS_TOL2   GLP_DBL_EMINUS_10
    #define GLP_CHK_IOS_BOUNDS_TOL   GLP_DBL_EMINUS_4
    #define GLP_CHK_COVER_TOL   GLP_DBL_EMINUS_5
    #define GLP_CHK_COV_GEN_TOL   GLP_DBL_EMINUS_3
    #define GLP_BRNCH_DRTOM_TOL   GLP_DBL_EMINUS_3
    #define GLP_BRNCH_DRTOM_TOL2   GLP_DBL_EMINUS_6
    #define GLP_BEST_NODE_TOL   GLP_DBL_EMINUS_10
    #define GLP_SOLVE_NE_TOL   GLP_DBL_EMINUS_4
    #define GLP_IPM_MAIN_TOL   GLP_DBL_EMINUS_8
    #define GLP_IPM_MAIN_TOL2   GLP_DBL_EPLUS_5
    #define GLP_IPM_MAIN_TOL3   GLP_DBL_EPLUS_6
    #define GLP_FPUMP_TOL   GLP_DBL_EMINUS_6
    #define GLP_GMI_CUT_TOL   GLP_DBL_EPLUS_5
    #define GLP_GMI_CUT_TOL2   GLP_DBL_EMINUS_10
    #define GLP_GMI_CUT_TOL3   GLP_DBL_EMINUS_12
    #define GLP_GMI_GEN_TOL   GLP_DBL_EMINUS_3
    #define GLP_GMI_GEN_TOL2   GLP_DBL_EPLUS_3
    #define GLP_MPS_TOL   GLP_DBL_EMINUS_12
    #define GLP_WCLICK_TOL   GLP_DBL_EMINUS_5
    #define GLP_CPS_RELAX    0.07
    #define GLP_CPS_TOL_BND  GLP_DBL_EMINUS_7
    #define GLP_CPS_TOL_DJ   GLP_DBL_EMINUS_7
    #define GLP_CPS_TOL_PIV  GLP_DBL_EMINUS_9
    #define GLP_CPS_OBJ_LL   -GLP_DBL_MAX
    #define GLP_CPS_OBJ_UL   +GLP_DBL_MAX
    #define GLP_CPS_IT_LIM   -1
    #define GLP_CPS_TM_LIM   -1.0
    #define GLP_CPS_TOL_INT  GLP_DBL_EMINUS_5
    #define GLP_CPS_TOL_OBJ  GLP_DBL_EMINUS_7
    #define GLP_LPX_RE_MAX_9  GLP_DBL_EMINUS_9
    #define GLP_LPX_RE_MAX_6  GLP_DBL_EMINUS_6
    #define GLP_LPX_RE_MAX_3  GLP_DBL_EMINUS_3
    #define GLP_MPL_MIN_9  GLP_DBL_EMINUS_9
    #define GLP_MPL_MIN_8  GLP_DBL_EMINUS_8
    #define GLP_MPL_MIN_5  GLP_DBL_EMINUS_5
    #define GLP_MPL_MIN_6  GLP_DBL_EMINUS_6
    #define GLP_MPL_MIN_7  GLP_DBL_EMINUS_7
    #define GLP_MPL_MIN_3  GLP_DBL_EMINUS_3
    #define GLP_WARMUP_EPS(x)  (GLP_DBL_EMINUS_6 + GLP_DBL_EMINUS_9 * fabs(x))
    #define GLP_MKEQ_EPS(x)  (GLP_DBL_EMINUS_9 + GLP_DBL_EMINUS_12 * fabs(x))
    #define GLP_DBL_THOUSAND  1000.0
    #define GLP_SCALE_RANGE_MIN  0.10
    #define GLP_SCALE_RANGE_MAX  10.0
    #define GLP_NPP_EPS  GLP_DBL_EMINUS_3
    #define GLP_NPP_EPS5  GLP_DBL_EMINUS_5
    #define GLP_NPP_EPS6  GLP_DBL_EMINUS_6
    #define GLP_NPP_EPS7  GLP_DBL_EMINUS_7
    #define GLP_NPP_EPS10  GLP_DBL_EMINUS_10
    #define GLP_NPP_EPS_3_6_fabs(x) (GLP_DBL_EMINUS_3 + GLP_DBL_EMINUS_6 * fabs(x))
    #define GLP_NPP_EPS_5_8_fabs(x) (GLP_DBL_EMINUS_5 + GLP_DBL_EMINUS_8 * fabs(x))
    #define GLP_NPP_EPS_3_6(x) (GLP_DBL_EMINUS_3 + GLP_DBL_EMINUS_6 * (x))
    #define GLP_PROXY_EPS  GLP_DBL_EMINUS_6
    #define GLP_PROXY_RINF  1e38
    #define GLP_PROXY_MAXVAL  1e20
    #define GLP_MIP_GAP  9999999.9
    #define GLP_SPX_PENALTY  0.97
    #define GLP_SPX_ADJ_PENALTY  0.99
    #define GLP_SPY_ART_BOUND  GLP_DBL_EPLUS_3
    #define GLP_SPY_FEASIBILITY  0.97
#endif

#ifdef __cplusplus
}
#endif

#endif /* GLPK_REAL_H */

