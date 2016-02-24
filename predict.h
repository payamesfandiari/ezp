//
// Created by payam on 2/21/16.
//

#ifndef EZPLANE_PREDICT_H
#define EZPLANE_PREDICT_H

#include "ezp.h"
#include <set>
struct predict_param{
    int pred_strategy;
    int num_class;
    int bsp_ezp;
    int ils_itr;
    int ezp_size;
    int pos_class;
    double (*ezp_lbl_func)(double);
    std::set<int> classes;
};

void exit_with_help();
void parse_command_line(int argc, char **argv, std::string &input_file_name, std::string &model_file_name,std::string &output_file_name, predict_param &param);
model* read_model(std::string &model_file,predict_param &predictParam);
problem* read_problem(const std::string &input_file_name);
model * pick_best_obj(model *mod, predict_param &predictParam, int cols);
model * pick_best_err(model *mod, predict_param &predictParam, int cols);
void maj_vote();
void bsp_predict(model* mod,problem* prob,predict_param &predictParam);
void ezp_transform();
#endif //EZPLANE_PREDICT_H
