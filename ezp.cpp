//
// Created by MohammadReza Esfandiari on 2/17/16.
//

#include <algorithm>
#include <cstring>
#include "ezp.h"

problem* create_data(const problem& prob,double tr_perc,int* plus_lbls,int* minus_lbls){

    int  cd_plus((prob.nr_plus*tr_perc)+1),cd_minus((prob.nr_minus*tr_perc)+1);
    int cd_rows (cd_minus+cd_plus);
    int* cd_lbl = new int[cd_rows];
    double* cd_data = new double[cd_rows*prob.cols];
    std::shuffle(plus_lbls,plus_lbls+cd_plus,std::default_random_engine());
    std::shuffle(minus_lbls,minus_lbls+cd_minus,std::default_random_engine());

    for (int i = 0; i < cd_plus; ++i) {
        for (int j = 0; j < prob.cols; ++j) {
            cd_data[i*prob.cols + j] = prob.data[plus_lbls[i]*prob.cols + j];
        }
        cd_lbl[i] = prob.labels[plus_lbls[i]];
    }
    for (int i = cd_plus,k=0; i < cd_rows; ++i,++k) {
        for (int j = 0; j < prob.cols; ++j) {
            cd_data[i*prob.cols + j] = prob.data[minus_lbls[k]*prob.cols + j];
        }
        cd_lbl[i] = prob.labels[minus_lbls[k]];
    }
    problem* cd_prob = new problem;
    cd_prob->labels = cd_lbl;
    cd_prob->data = cd_data;
    cd_prob->rows = cd_rows;
    cd_prob->cols = prob.cols;
    cd_prob->nr_minus = cd_minus;
    cd_prob->nr_plus = cd_plus;
    return cd_prob;
}

model* ezp(const ezp_param &ezpParam, problem &prob) {

    int rows(prob.rows),itr(0);
    model* out_model = new model[ezpParam.bsp_param->ils_itr*ezpParam.ezp_size];
    int* plus_lbl_ind = new int[prob.nr_plus];
    int* minus_lbl_ind = new int[prob.nr_minus];

    for (int i = 0,j=0,k=0; i < rows; ++i) {
        if(prob.labels[i] == 1) {
            plus_lbl_ind[j] = i;
            ++j;
        }
        else {
            minus_lbl_ind[k] = i;
            ++k;
        }
    }


    while (itr < ezpParam.ezp_size){
        problem* reduced_prob = create_data(prob,ezpParam.ezp_train_percent,plus_lbl_ind,minus_lbl_ind);
        model* m = bsp(ezpParam.bsp_param,reduced_prob);
        std::memcpy(out_model+(itr*ezpParam.bsp_param->ils_itr),m,ezpParam.bsp_param->ils_itr * sizeof(struct model));
        ++itr;
    }

    return out_model;
}


//double* transform(model *m_array, ezp_param &ezpParam, problem &prob) {
//    int total_num_model(ezpParam.bsp_param->ils_itr*ezpParam.ezp_size);
//    double* trans_output = new double[prob.rows*total_num_model];
//    double proj;
//    for (int m = 0; m < total_num_model; ++m) {
//        for (int i = 0; i < prob.rows; ++i) {
//            proj = 0 ;
//            for (int j = 0; j < prob.cols; ++j) {
//                proj += prob.data[i*prob.cols+j] * m_array[m].W[j];
//            }
//            proj += m_array[m].W0;
//            trans_output[i*total_num_model + m] = ezpParam.ezp_lbl_func(proj);
//        }
//    }
//    return trans_output;
//}

double ezp_sign(double projection) {
    return (projection >0)?1.0:-1.0;
}

double ezp_sigmod(double projection) {
    return projection / (1+std::abs(projection));
}
