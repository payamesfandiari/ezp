//
// Created by payam on 2/21/16.
//

#ifndef EZPLANE_PREDICT_H
#define EZPLANE_PREDICT_H

#include "ezp.h"
#include <set>
#include <fstream>
#include <string>

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
void copy_model(model* src, model* des, int cols);
void exit_with_help();
void parse_command_line(int argc, char **argv, std::string &input_file_name, std::string &model_file_name,std::string &output_file_name, predict_param &param);
void read_param(std::ifstream &infile, predict_param &predictParam);
model* read_model(std::ifstream &infile, predict_param &predictParam,int cols);
problem* read_problem(std::string &input_file_name);
model * pick_best_obj(model *mod, predict_param &predictParam, int cols);
model * pick_best_err(model *mod, predict_param &predictParam, int cols);
void bsp_one_predict(model *mod, problem *prob, predict_param &predictParam, std::string &output_file);
void ezp_one_transform(model *mod, problem *prob, predict_param &predictParam, std::string &output_file);
void bsp_multi_predict(std::ifstream &model_file,problem *prob, predict_param &predictParam, std::string &output_file);


#endif //EZPLANE_PREDICT_H
