#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "main.h"

#define Malloc(type,n) (type*)malloc((n)*sizeof(type))



void exit_with_help(){
    printf(
            "Usage: train [options] training_set_file [model_file]\n"
                    "options:\n"
                    "-z bsp (1) or ezplane (2) (default 2)\n"
                    "-zs for -z = 2, number of EZPlanes (default 1000)\n"
                    "-zb for -z = 2, percent of train to pick (default .1)\n"
                    "-i ils num: set number of ILS (default 1)\n"
                    "-c cost : set the parameter C (default 1)\n"
                    "-s ils_step : set the amount of change for each ILS (default 1)\n"
                    "-e epsilon : set tolerance of termination criterion (default .001)\n"
                    "-l local itr thresh : Set the local iteration for each loop of ILS (default 1000)\n"
                    "-w w_inc weight update: how much should we update each dimension of W in for ILS (default 10)\n"
                    "-p ils perm: percent of W that we should update (default 1) \n"
                    "-u init_w : the upper bound of uniform dist which will generate the initial W (default 1)\n"
    );
    exit(1);
}

void parse_command_line(int argc, char **argv, std::string &input_file_name, std::string &model_file_name, ezp_param &param){
    int i;
    for (i = 1; i < argc; ++i) {
        if(argv[i][0] != '-') break;
        if(++i >= argc)
            exit_with_help();

        switch (argv[i-1][1]){
            case 'z':
                if(std::char_traits<char>::length(argv[i-1])==2)
                    param.bsp_ezp = std::stoi(argv[i]);
                else{
                    switch (argv[i-1][2]){
                        case 's':
                            param.ezp_size = std::stoi(argv[i]);
                            break;
                        case 'b':
                            param.ezp_train_percent = std::stod(argv[i]);
                            break;
                        default:
                            exit_with_help();
                            break;
                    }
                }
                break;
            case 'i':
                param.bsp_param->ils_itr = std::stoi(argv[i]);
                break;
            case 'c':
                param.bsp_param->C = std::stod(argv[i]);
                break;
            case 's':
                param.bsp_param->ils_step = std::stoi(argv[i]);
                break;
            case 'e':
                param.bsp_param->stop_thresh = std::stod(argv[i]);
                break;
            case 'l':
                param.bsp_param->local_itr_thresh = std::stoi(argv[i]);
                break;
            case 'w':
                param.bsp_param->w_inc = std::stod(argv[i]);
                break;
            case 'p':
                param.bsp_param->ils_perm = std::stod(argv[i]);
                break;
            case 'u':
                param.bsp_param->initial_w = std::stod(argv[i]);
                break;
            default:
                exit_with_help();
                break;
        }
    }
    if (i >= argc)
        exit_with_help();
    std::string s(argv[i]);
    input_file_name = s;
    if (i < argc-1){
        std::string s2(argv[i+1]);
        model_file_name = s2;
    }else{
        std::size_t found = input_file_name.find_last_of("/\\");
        model_file_name = input_file_name.substr(found+1) ;
        model_file_name.append(".model");
    }
}




int main(int argc,char* argv[]) {
    ezp_param param;
    param.bsp_ezp = 2;
    param.ezp_size = 1000;
    param.ezp_train_percent = 0.1;
    parameter bsp_param = {1,1,1000,100.0,1.0,0.001,1.0,1};
    param.bsp_param = &bsp_param;

    std::cout << param.bsp_param->local_itr_thresh ;
    std::string input_file_name;
    std::string model_file_name;

    if(argc < 3){
        exit_with_help();
    }
    parse_command_line(argc,argv,input_file_name,model_file_name,param);
    problem* prob = read_problem(input_file_name);




    return 0;
}


problem* read_problem(const std::string &input_file_name) {
    problem* prob = new problem;
    std::ifstream infile(input_file_name);
    std::string line;
    std::vector<double> col;
    std::vector<int> lbls;
    int rows(0),cols(0),l(0);
    if(infile.is_open()) {
        double temp;
        while (std::getline(infile, line)) {
            if(line.compare("\n") == 0) continue;
            rows++;
            std::istringstream iss(line);
            iss >> l;
            lbls.push_back(l);
            while(iss >> temp){
                col.push_back(temp);
            }
            if (cols==0) cols = col.size();
        }
        printf("%d rows and %d cols have been read from the file : %s",rows,cols,input_file_name.c_str());
        prob->rows = rows;
        prob->cols = cols;
        prob->data = new double[rows*cols];
        prob->labels = new int[rows];
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                prob->data[i*cols+j] = col[i*cols+j];
            }
            prob->labels[i] = lbls[i];
        }
    }
    col.empty();
    lbls.empty();
    return prob;
}
