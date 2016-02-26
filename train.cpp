#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <set>
#include <string>
#include "train.h"

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

std::map<int,std::vector<int>> nr_class_map;
std::set<int> s;

int main(int argc,char* argv[]) {
//    std::ios::sync_with_stdio(false);
    ezp_param ezpParam;
    ezpParam.bsp_ezp = 2;
    ezpParam.ezp_size = 1000;
    ezpParam.ezp_train_percent = 0.1;
    parameter bsp_param = {1,1,1000,100.0,1.0,0.001,1.0,1};
    ezpParam.bsp_param = &bsp_param;

//    std::cout << ezpParam.bsp_param->local_itr_thresh ;
    std::string input_file_name;
    std::string model_file_name;

    if(argc < 3){
        exit_with_help();
    }
    parse_command_line(argc, argv, input_file_name, model_file_name, ezpParam);
    problem* prob = read_problem(input_file_name);
    std::cout << model_file_name <<std::endl;
    print_to_file(ezpParam,*prob,model_file_name);
    if(ezpParam.bsp_ezp == 1){
        if(prob->num_class > 2){
            for(int plus_c : s){
                prob->nr_plus = static_cast<int>(nr_class_map[plus_c].size());
                prob->nr_minus = 0;
                for(int c : s){
                    if(c == plus_c){
                        for (std::vector<int>::iterator it=nr_class_map[c].begin();it!=nr_class_map[c].end();++it) {
                            prob->labels[*it] = 1;
                        }
                    }else{
                        for (std::vector<int>::iterator it=nr_class_map[c].begin();it!=nr_class_map[c].end();++it) {
                            prob->labels[*it] = -1;
                        }
                        prob->nr_minus += nr_class_map[c].size();
                    }
                }
                model* out_mod = bsp(ezpParam.bsp_param,prob);
                save_model(ezpParam,model_file_name,out_mod,plus_c,prob->cols);
                delete(out_mod);
            }
        }else if(prob->num_class == 2){
            prob->nr_minus = static_cast<int>( nr_class_map[-1].size());
            prob->nr_plus = static_cast<int>(nr_class_map[1].size());
            model* out_mod = bsp(ezpParam.bsp_param,prob);
            save_model(ezpParam,model_file_name,out_mod,1,prob->cols);

        }
    }else if(ezpParam.bsp_ezp == 2){
        if(prob->num_class > 2){
            for(int plus_c : s){
                prob->nr_plus = static_cast<int>(nr_class_map[plus_c].size());
                prob->nr_minus = 0;
                for(int c : s){
                    if(c == plus_c){
                        for (std::vector<int>::iterator it=nr_class_map[c].begin();it!=nr_class_map[c].end();++it) {
                            prob->labels[*it] = 1;
                        }
                    }else{
                        for (std::vector<int>::iterator it=nr_class_map[c].begin();it!=nr_class_map[c].end();++it) {
                            prob->labels[*it] = -1;
                        }
                        prob->nr_minus += nr_class_map[c].size();
                    }
                }
                model* out_mod = ezp(ezpParam,*prob);
                save_model(ezpParam,model_file_name,out_mod,plus_c,prob->cols);
                delete(out_mod);
            }
        }else if(prob->num_class == 2){
            prob->nr_minus = static_cast<int>( nr_class_map[-1].size());
            prob->nr_plus = static_cast<int>(nr_class_map[1].size());
            model* out_mod = ezp(ezpParam,*prob);
            save_model(ezpParam,model_file_name,out_mod,1,prob->cols);
        }
    }

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
            s.insert(l);
            while(iss >> temp){
                col.push_back(temp);
            }
            if (cols==0) cols = static_cast<int>(col.size());
        }
        printf("%d rows and %d cols have been read from the file : %s\n",rows,cols,input_file_name.c_str());
        prob->rows = rows;
        prob->cols = cols;
        prob->data = new double[rows*cols];
        prob->labels = new int[rows];
        prob->num_class = static_cast<int>(s.size());
        for(int x : s){
            std::vector<int> ind_map_elem;
            nr_class_map[x] = ind_map_elem;
        }
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                prob->data[i*cols+j] = col[i*cols+j];
            }
            prob->labels[i] = lbls[i];
            nr_class_map[lbls[i]].push_back(i);
        }
    }
    return prob;
}

void print_to_file(ezp_param &ezpParam, problem &prob, std::string model_file_name) {
    FILE* fout = fopen(model_file_name.c_str(),"w");
    fprintf(fout,"rows:%d\ncols:%d\n",prob.rows,prob.cols);
    fprintf(fout,"num_class:%d\n",prob.num_class);
    if(ezpParam.bsp_ezp == 1)
        fprintf(fout,"bsp_ezp:bsp\n");
    else
        fprintf(fout,"bsp_ezp:ezp\n");

    fprintf(fout,"ezp_size:%d\n",ezpParam.ezp_size);
    fprintf(fout,"ezp_train_per:%.3f\n",ezpParam.ezp_train_percent);
    fprintf(fout,"bsp_c:%.5f\n",ezpParam.bsp_param->C);
    fprintf(fout,"bsp_ils_itr:%d\n",ezpParam.bsp_param->ils_itr);
    fprintf(fout,"bsp_ils_perm:%.5f\n",ezpParam.bsp_param->ils_perm);
    fprintf(fout,"bsp_ils_step:%d\n",ezpParam.bsp_param->ils_step);
    fprintf(fout,"bsp_init_w:%.5f\n",ezpParam.bsp_param->initial_w);
    fprintf(fout,"bsp_l_itr_thresh:%d\n",ezpParam.bsp_param->local_itr_thresh);
    fprintf(fout,"bsp_stop_thresh:%f\n",ezpParam.bsp_param->stop_thresh);
    fprintf(fout,"bsp_w_inc:%f\n",ezpParam.bsp_param->w_inc);
    fprintf(fout,"classes:");
    for (int x: s) {
        fprintf(fout,"%d ",x);
    }
    fprintf(fout,"\nmodel\n");
    fclose(fout);
}


void save_model(ezp_param &ezpParam,std::string model_file_name,model* mod,int class1,int cols){
    FILE* fout = fopen(model_file_name.c_str(),"a");
    fprintf(fout,"class 1:%d\n",class1);
    if (ezpParam.bsp_ezp == 1){
        for (int i = 0; i < ezpParam.bsp_param->ils_itr; ++i) {
            fprintf(fout,"err=%f;obj=%f\n",mod[i].error,mod[i].obj);
            fprintf(fout,"%f ",mod[i].W0);
            for (int j = 0; j < cols; ++j) {
                fprintf(fout,"%f ",mod[i].W[j]);
            }
            fprintf(fout,"\n");
        }
    } else{
        for (int i = 0; i < ezpParam.bsp_param->ils_itr*ezpParam.ezp_size; ++i) {
            fprintf(fout,"err=%f;obj=%f\n",mod[i].error,mod[i].obj);
            fprintf(fout,"%f ",mod[i].W0);
            for (int j = 0; j < cols; ++j) {
                fprintf(fout,"%f ",mod[i].W[j]);
            }
            fprintf(fout,"\n");
        }
    }
    fclose(fout);

}