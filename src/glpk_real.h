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
        #define GLP_DBL_FMT_G "Lg"
        #define GLP_DBL_FMT_F "Lf"
        #define GLP_DBL_FMT_E "LE"
        #define GLP_DBL_FMT_e "Le"
    #else
        #define glp_double double
        #define GLP_DBL_EPSILON DBL_EPSILON
        #define GLP_DBL_FMT_G "g"
        #define GLP_DBL_FMT_F "f"
        #define GLP_DBL_FMT_E "E"
        #define GLP_DBL_FMT_e "e"
    #endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* GLPK_REAL_H */

