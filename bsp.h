//
// Created by payam on 2/14/16.
//

#ifndef EZPLANE_BSP_H
#define EZPLANE_BSP_H

#include <limits>
const double INF = std::numeric_limits<double>::infinity();
// Structures
// Parameters
struct parameter{
    int ils_itr;
    int ils_step;
    int local_itr_thresh;
    double w_inc;
    double initial_w;
    double stop_thresh;
    double ils_perm;
    double C;
};
// Data
// 'data' which is data and 'label' which is labels, rows and cols are also stored
struct problem{
    int rows;
    int cols;
    double* data;
    int* labels;
    int nr_plus;
    int nr_minus;
};

// Model
// W,W0 are the model,'error' is the 0-1 loss,'smooth_term' is smoothing term
// 'geo_smooth_term' Geometric smoothing term, 'objective' is the Objective
// which is C*error + smooth_term + objective
struct model{
    double obj;
    double error;
    double* W;
    double W0;
};


void fisher_yates(int* array,int l);
struct model* bsp(struct parameter* param,struct problem* prob);

#endif //EZPLANE_BSP_H
