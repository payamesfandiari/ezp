//
// Created by payam on 2/16/16.
//

#ifndef EZPLANE_MAIN_H
#define EZPLANE_MAIN_H
#include "ezp.h"

void exit_with_help();
void parse_command_line(int argc, char **argv, std::string &input_file_name, std::string &model_file_name, ezp_param &param);
problem* read_problem(const std::string &input_file_name);

#endif //EZPLANE_MAIN_H
