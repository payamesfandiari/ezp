//
// Created by payam on 2/21/16.
//
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include "predict.h"

#define Malloc(type,n) (type*)malloc((n)*sizeof(type))



void exit_with_help(){
    printf(
            "Usage: predict [options] data_file model_file [prediction_transformation]\n"
                    "options:\n"
                    "-f transformation func: sign function(1), sigmoid function(2)(default 1)\n"
                    "-s prediction strategy:\n"
                    "  for bsp classification (default 0)\n"
                    "	 0 -- pick plane with best objective on training \n"
                    "	 1 -- pick plane with best error on training\n"
                    "	 2 -- get majority vote\n"
                    "  for ezp transformation (default 6)\n"
                    "	 3 -- pick plane with best objective on training in each round and use it to transform\n"
                    "	 4 -- pick plane with best error on training in each round and use it to transform\n"
                    "	 5 -- get majority vote of each round and use it to transform\n"
                    "	 6 -- use all\n"
    );
    exit(1);
}

void parse_command_line(int argc, char **argv, std::string &input_file_name, std::string &model_file_name,std::string &output_file_name, predict_param &param){
    int i;
    for (i = 1; i < argc; ++i) {
        if(argv[i][0] != '-') break;
        if(++i >= argc)
            exit_with_help();

        switch (argv[i-1][1]){
            case 'f':
                if(std::stoi(argv[i]) == 2)
                    param.ezp_lbl_func = &ezp_sigmod;
                else
                    param.ezp_lbl_func = &ezp_sign;
                break;
            case 's':
                param.pred_strategy = std::stoi(argv[i]);
                break;
            default:
                exit_with_help();
                break;
        }
    }
    if (i >= argc)
        exit_with_help();
    std::string s(argv[i++]);
    input_file_name = s;

    if (i >= argc)
        exit_with_help();

    std::string ss(argv[i]);
    model_file_name = ss;

    if (i < argc-1){
        std::string s2(argv[i+1]);
        output_file_name = s2;
    }else{
        std::size_t found = input_file_name.find_last_of("/\\");
        output_file_name = input_file_name.substr(found+1) ;
        output_file_name.append(".out");
    }
}


int main(int argc,char * argv[]){
    if(argc < 3){
        exit_with_help();
    }
    predict_param predictParam;
    ezp_param ezpParam;
    parameter bspParam;
    ezpParam.bsp_param = &bspParam;
    predictParam.pred_strategy = 6;
    std::string input_file_name;
    std::string model_file_name;
    std::string out_file_name;
    parse_command_line(argc,argv,input_file_name,model_file_name,out_file_name,predictParam);
    std::cout << input_file_name << " " << model_file_name << " " << out_file_name << std::endl;

    model* m = read_model(model_file_name,predictParam);
    problem* prob = read_problem(input_file_name);
    prob->num_class = predictParam.num_class;

}

model* read_model(std::string &model_file, predict_param &predictParam) {
    std::ifstream infile(model_file);
    std::string line;
    std::size_t found ;
    std::string opt;
    int cols(0);
    model* mod;
    if(infile.is_open()) {
        while (std::getline(infile, line)) {
            found = line.find(":");
            if (found != std::string::npos){
                opt = line.substr(0,found);
                if (opt.compare("cols")==0){
                    cols = std::stoi(line.substr(found+1));
                }else if (opt.compare("num_class")==0){
                    predictParam.num_class = std::stoi(line.substr(found+1));
                }else if (opt.compare("bsp_ezp")==0){
                    if(line.substr(found+1).compare("bsp")==0)
                        predictParam.bsp_ezp = 1;
                    else
                        predictParam.bsp_ezp = 2;
                }else if (opt.compare("ezp_size")==0){
                    predictParam.ezp_size = std::stoi(line.substr(found+1));
                }else if (opt.compare("bsp_ils_itr")==0){
                    predictParam.ils_itr = std::stoi(line.substr(found+1));
                }else if (opt.compare("classes")==0){
                    int l;
                    std::istringstream iss(line.substr(found+1));

                    for (int i = 0; i < predictParam.num_class; ++i) {
                        iss >> l;
                        predictParam.classes.insert(l);
                    }

                }
            }else{
                break;
            }
        }
        if(predictParam.bsp_ezp == 1)
            predictParam.ezp_size = 1;
        mod = new model[predictParam.ils_itr*predictParam.ezp_size];
        std::getline(infile,line);
        found = line.find(":");
        predictParam.pos_class = std::stoi(line.substr(found+1));
        double temp(0);
        for (int i = 0; i < predictParam.ils_itr*predictParam.ezp_size; ++i) {

            mod[i].W = new double[cols];
            std::getline(infile,line);
            found = line.find(";");
            if (found != std::string::npos){
                std::string err_str (line.substr(0,found));
                std::string obj_str (line.substr(found+1));
                mod[i].error = std::stod(err_str.substr(err_str.find("=")+1));
                mod[i].obj = std::stod(obj_str.substr(obj_str.find("=")+1));
            }else{

            }
            std::getline(infile,line);
            std::istringstream iss(line);
            iss >> temp;
            mod[i].W0 = temp;
            for (int j = 0; j < cols; ++j) {
                iss >> temp;
                mod[i].W[j] = temp;
            }
        }
    }
    infile.close();
    return mod;
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
        printf("%d rows and %d cols have been read from the file : %s\n",rows,cols,input_file_name.c_str());
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
    return prob;
}


model *pick_best_obj(model *mod, predict_param &predictParam, int cols) {
    model* best_model = new model[predictParam.ezp_size];
    double best_obj(-INF);
    int ind(0);
    for (int i = 0; i < predictParam.ezp_size; ++i) {
        best_obj = -INF;
        for (int j = 0; j < predictParam.ils_itr; ++j) {
            if(mod[i+predictParam.ils_itr +j].obj > best_obj){
                best_obj = mod[i+predictParam.ils_itr +j].obj;
                ind = i+predictParam.ils_itr +j ;
            }
        }
        best_model[i].obj = mod[ind].obj;
        best_model[i].error = mod[ind].error;
        best_model[i].W0 = mod[ind].W0;
        best_model[i].W = new double[cols];
        for (int k = 0; k < cols; ++k) {
            best_model[i].W[k] = mod[ind].W[k];
        }
    }
    return best_model;

}

model *pick_best_err(model *mod, predict_param &predictParam, int cols) {
    model* best_model = new model[predictParam.ezp_size];
    double best_err(-INF);
    int ind(0);
    for (int i = 0; i < predictParam.ezp_size; ++i) {
        best_err = -INF;
        for (int j = 0; j < predictParam.ils_itr; ++j) {
            if(mod[i+predictParam.ils_itr +j].error > best_err){
                best_err = mod[i+predictParam.ils_itr +j].error;
                ind = i+predictParam.ils_itr +j ;
            }
        }
        best_model[i].obj = mod[ind].obj;
        best_model[i].error = mod[ind].error;
        best_model[i].W0 = mod[ind].W0;
        best_model[i].W = new double[cols];
        for (int k = 0; k < cols; ++k) {
            best_model[i].W[k] = mod[ind].W[k];
        }
    }
    return best_model;
}
