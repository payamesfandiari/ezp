//
// Created by payam on 2/16/16.
//

#ifndef EZPLANE_MAIN_H
#define EZPLANE_MAIN_H
#include "ezp.h"
#include <string>
void exit_with_help();
void parse_command_line(int argc, char **argv, std::string &input_file_name, std::string &model_file_name, ezp_param &param);
problem* read_problem(const std::string &input_file_name);
void print_to_file(ezp_param &ezpParam,problem &prob,std::string model_file_name);
void save_model(ezp_param &ezpParam,std::string model_file_name,model* mod,int class1,int cols);

#endif //EZPLANE_MAIN_H
