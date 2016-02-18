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

problem* create_data(const problem &prob);

void ezp(const ezp_param &ezpParam,const problem &prob);




#endif //EZPLANE_EZP_H
