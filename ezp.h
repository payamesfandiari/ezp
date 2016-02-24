//
// Created by MohammadReza Esfandiari on 2/17/16.
//

#ifndef EZPLANE_EZP_H
#define EZPLANE_EZP_H

#include "bsp.h"

struct ezp_param{
    parameter* bsp_param;
    int bsp_ezp ;
    int ezp_size;
    double ezp_train_percent;
};

problem* create_data(const problem& prob,double tr_perc,int* plus_lbls,int* minus_lbls);

model* ezp(const ezp_param &ezpParam, problem &prob);

double* transform(model* m_array,ezp_param &ezpParam, problem &prob);

double ezp_sign(double projection);
double ezp_sigmod(double projection);

#endif //EZPLANE_EZP_H
