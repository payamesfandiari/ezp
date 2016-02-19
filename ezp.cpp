//
// Created by MohammadReza Esfandiari on 2/17/16.
//

#include <algorithm>
#include <random>
#include "ezp.h"
#include "bsp.h"

problem* create_data(const problem& prob,int tr_perc,int* lbl_index){

    int cd_rows = prob.rows * tr_perc,cd_plus(0),cd_minus(0);
    int* cd_lbl = new int[cd_rows];
    double* cd_data = new double[cd_rows];
    std::shuffle(lbl_index,lbl_index+prob.rows,std::default_random_engine());
    for (int i = 0; i < cd_rows; ++i) {
        for (int j = 0; j < prob.cols; ++j) {
            cd_data[lbl_index[i]*prob.cols + j] = prob.data[lbl_index[i]*prob.cols + j];
        }
        cd_lbl[i] = prob.labels[lbl_index[i]];
        if(cd_lbl[i] == 1)
            ++cd_plus;
        else
            ++cd_minus;
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

void ezp(const ezp_param &ezpParam, const problem &prob) {

    int rows(prob.rows),cols(prob.cols);
    short* trans_data = new short[rows*ezpParam.ezp_size];
    int itr(0);
    int* lbl_index = new int[rows];
    for (int i = 0; i < rows; ++i) {
        lbl_index[i] = i ;
    }
    while (itr < ezpParam.ezp_size){
        problem* reduced_prob = create_data(prob,ezpParam.ezp_train_percent,lbl_index);




        ++itr;
    }


}
