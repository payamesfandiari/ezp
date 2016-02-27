//
// Created by payam on 2/21/16.
//
#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
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
                    "	 5 -- use all\n"
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
    predictParam.pred_strategy = 6;
    std::string input_file_name;
    std::string model_file_name;
    std::string out_file_name;
    parse_command_line(argc,argv,input_file_name,model_file_name,out_file_name,predictParam);
    std::cout << input_file_name << " " << model_file_name << " " << out_file_name << std::endl;
    problem* prob = read_problem(input_file_name);
    std::ifstream model_file(model_file_name);
    read_param(model_file,predictParam);
    prob->num_class = predictParam.num_class;
    if(predictParam.num_class == 2) {
        model* m = read_model(model_file,predictParam,prob->cols);
        if (predictParam.bsp_ezp == 1) {
            bsp_one_predict(m, prob, predictParam, out_file_name);
        } else if (predictParam.bsp_ezp == 2) {
            ezp_one_transform(m, prob, predictParam, out_file_name);
        } else {
            std::cerr << "Unrecognized Option ! Please run Train again...";
        }
        delete[] m->W;
        delete[] prob->data;
        delete(m);
        delete(prob);
    }else{
        if (predictParam.bsp_ezp == 1) {
            bsp_multi_predict(model_file, prob, predictParam, out_file_name);
        } else if (predictParam.bsp_ezp == 2) {
            ezp_multi_transform(model_file, prob, predictParam, out_file_name);
        } else {
            std::cerr << "Unrecognized Option ! Please run Train again...";
        }
    }


}

model* read_model(std::ifstream &infile, predict_param &predictParam,int cols){
    model* mod = new model[predictParam.ils_itr*predictParam.ezp_size];
    std::string line;
    std::size_t found ;
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
    return mod;
}
void read_param(std::ifstream &infile, predict_param &predictParam) {
    std::string line;
    std::size_t found ;
    std::string opt;
    if(infile.is_open()) {
        while (std::getline(infile, line)) {
            found = line.find(":");
            if (found != std::string::npos) {
                opt = line.substr(0, found);
                if (opt.compare("num_class") == 0) {
                    predictParam.num_class = std::stoi(line.substr(found + 1));
                } else if (opt.compare("bsp_ezp") == 0) {
                    if (line.substr(found + 1).compare("bsp") == 0)
                        predictParam.bsp_ezp = 1;
                    else
                        predictParam.bsp_ezp = 2;
                } else if (opt.compare("ezp_size") == 0) {
                    predictParam.ezp_size = std::stoi(line.substr(found + 1));
                } else if (opt.compare("bsp_ils_itr") == 0) {
                    predictParam.ils_itr = std::stoi(line.substr(found + 1));
                } else if (opt.compare("classes") == 0) {
                    int l;
                    std::istringstream iss(line.substr(found + 1));

                    for (int i = 0; i < predictParam.num_class; ++i) {
                        iss >> l;
                        predictParam.classes.insert(l);
                    }

                }
            } else {
                break;
            }
        }
        if (predictParam.bsp_ezp == 1)
            predictParam.ezp_size = 1;
    }
//        mod = new model[predictParam.ils_itr*predictParam.ezp_size];
//        std::getline(infile,line);
//        found = line.find(":");
//        predictParam.pos_class = std::stoi(line.substr(found+1));
//        double temp(0);
//        for (int i = 0; i < predictParam.ils_itr*predictParam.ezp_size; ++i) {
//
//            mod[i].W = new double[cols];
//            std::getline(infile,line);
//            found = line.find(";");
//            if (found != std::string::npos){
//                std::string err_str (line.substr(0,found));
//                std::string obj_str (line.substr(found+1));
//                mod[i].error = std::stod(err_str.substr(err_str.find("=")+1));
//                mod[i].obj = std::stod(obj_str.substr(obj_str.find("=")+1));
//            }else{
//
//            }
//            std::getline(infile,line);
//            std::istringstream iss(line);
//            iss >> temp;
//            mod[i].W0 = temp;
//            for (int j = 0; j < cols; ++j) {
//                iss >> temp;
//                mod[i].W[j] = temp;
//            }
//        }
//    }
//    infile.close();
//    return mod;
}
problem* read_problem(std::string &input_file_name) {
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
            if(mod[i*predictParam.ils_itr +j].obj > best_obj){
                best_obj = mod[i*predictParam.ils_itr +j].obj;
                ind = i*predictParam.ils_itr +j ;
            }
        }
        copy_model(&mod[ind],&best_model[i],cols);
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
            if(mod[i*predictParam.ils_itr +j].error > best_err){
                best_err = mod[i*predictParam.ils_itr +j].error;
                ind = i*predictParam.ils_itr +j ;
            }
        }
        copy_model(&mod[ind],&best_model[i],cols);
//        best_model[i].obj = mod[ind].obj;
//        best_model[i].error = mod[ind].error;
//        best_model[i].W0 = mod[ind].W0;
//        best_model[i].W = new double[cols];
//        for (int k = 0; k < cols; ++k) {
//            best_model[i].W[k] = mod[ind].W[k];
//        }
    }
    return best_model;
}
void bsp_one_predict(model *mod, problem *prob, predict_param &predictParam, std::string &output_file) {
    FILE* fout = fopen(output_file.c_str(),"w");
    double proj(0);
    double error(0);
    int pos(0),neg(0);
    model* m ;
    switch (predictParam.pred_strategy){
        case 0:
            m = pick_best_obj(mod,predictParam,prob->cols);
            for (int i = 0; i < prob->rows; ++i) {
                proj = 0 ;
                for (int j = 0; j < prob->cols; ++j) {
                    proj += prob->data[i*prob->cols+j] * m->W[j];
                }
                proj += m->W0;
                if(proj*prob->labels[i] <= 0){
                    error += 1;
                }
                if(proj > 0){
                    fprintf(fout,"1\n");
                }else{
                    fprintf(fout,"-1\n");
                }
            }
            std::cout << "error %" << 100* error / prob->rows << std::endl;
            break;
        case 1:
            m = pick_best_err(mod,predictParam,prob->cols);
            for (int i = 0; i < prob->rows; ++i) {
                proj = 0 ;
                for (int j = 0; j < prob->cols; ++j) {
                    proj += prob->data[i*prob->cols+j] * m->W[j];
                }
                proj += m->W0;
                if(proj*prob->labels[i] < 0){
                    error++;
                }
                if(proj > 0){
                    fprintf(fout,"1\n");
                }else{
                    fprintf(fout,"-1\n");
                }
            }
            std::cout << "error %" <<100* error / prob->rows << std::endl;
            break;
        case 2:

            for (int i = 0; i < prob->rows; ++i) {
                pos = neg = 0;
                proj = 0;
                for (int k = 0; k < predictParam.ils_itr; ++k) {
                    for (int j = 0; j < prob->cols; ++j) {
                        proj += prob->data[i*prob->cols + j] * mod[k].W[j];
                    }
                    proj += mod[k].W0;
                    if(proj > 0)
                        pos++;
                    else
                        neg++;
                }
                if(pos > neg){
                    fprintf(fout,"1\n");
                    if(prob->labels[i] == -1)
                        error ++;
                }else{
                    fprintf(fout,"-1\n");
                    if(prob->labels[i] == 1)
                        error ++;

                }
            }
            std::cout << "error %" <<100* error / prob->rows << std::endl;
            break;
        default:
            std::cerr << "Not a correct option for strategy." << std::endl;
            break;
    }
    fclose(fout);


}
void ezp_one_transform(model *mod, problem *prob, predict_param &predictParam, std::string &output_file) {

    FILE* fout = fopen(output_file.c_str(),"w");
    double proj(0),output(0);
    int pos(0),neg(0);
    model* m ;
    switch (predictParam.pred_strategy){
        case 3:
            m = pick_best_obj(mod,predictParam,prob->cols);
            for (int i = 0; i < prob->rows; ++i) {
                fprintf(fout,"%d ",prob->labels[i]);
                for (int k = 0; k < predictParam.ezp_size; ++k) {
                    proj = 0 ;
                    for (int j = 0; j < prob->cols; ++j) {
                        proj += prob->data[i * prob->cols + j] * m[k].W[j];
                    }
                    proj += m[k].W0;
                    output = predictParam.ezp_lbl_func(proj);
                    fprintf(fout,"%f ",output);
                }
                fprintf(fout,"\n");
            }
            break;
        case 4:
            m = pick_best_err(mod,predictParam,prob->cols);
            for (int i = 0; i < prob->rows; ++i) {
                fprintf(fout,"%d ",prob->labels[i]);
                for (int k = 0; k < predictParam.ezp_size; ++k) {
                    proj = 0 ;
                    for (int j = 0; j < prob->cols; ++j) {
                        proj += prob->data[i * prob->cols + j] * m[k].W[j];
                    }
                    proj += m[k].W0;
                    output = predictParam.ezp_lbl_func(proj);
                    fprintf(fout,"%f ",output);
                }
                fprintf(fout,"\n");
            }
            break;
        case 5:

            for (int i = 0; i < prob->rows; ++i) {
                fprintf(fout,"%d ",prob->labels[i]);
                for (int k = 0; k < predictParam.ezp_size*predictParam.ils_itr; ++k) {
                    proj = 0 ;
                    for (int j = 0; j < prob->cols; ++j) {
                        proj += prob->data[i * prob->cols + j] * mod[k].W[j];
                    }
                    proj += mod[k].W0;
                    output = predictParam.ezp_lbl_func(proj);
                    fprintf(fout,"%f ",output);
                }
                fprintf(fout,"\n");
            }
            break;

        default:
            std::cerr << "Not a correct option for strategy." << std::endl;
            break;
    }
    fclose(fout);


}


void copy_model(model* src, model* des, int cols) {
    des->error = src->error;
    des->obj = src->obj;
    des->W0 = src->W0;
    des->W = new double[cols];
    for (int j = 0; j < cols; ++j) {
        des->W[j] = src->W[j];
    }
}

void bsp_multi_predict(std::ifstream &model_file, problem *prob, predict_param &predictParam, std::string &output_file) {
    FILE* fout = fopen(output_file.c_str(),"w");
    double error(0);
    if(predictParam.pred_strategy < 2) {
        model* best_model = new model[predictParam.num_class];
        int* class_lbls = new int[predictParam.num_class];
        for (int class_num = 0; class_num < prob->num_class; ++class_num) {
            model *raw_model = read_model(model_file, predictParam, prob->cols);
            class_lbls[class_num] = predictParam.pos_class;
            if(predictParam.pred_strategy == 0) {
                model* best_m = pick_best_obj(raw_model,predictParam,prob->cols);
                copy_model(best_m,&best_model[class_num],prob->cols);
            }else{
                model* best_m = pick_best_err(raw_model,predictParam,prob->cols);
                copy_model(best_m,&best_model[class_num],prob->cols);
            }
            delete (raw_model);
        }
        double best_proj(0),temp_proj(0);
        int ind(0) ;
        for (int i = 0; i < prob->rows; ++i) {
            best_proj = temp_proj = 0;
            for (int j = 0; j < prob->cols; ++j) {
                best_proj += prob->data[i*prob->cols+j] * best_model[0].W[j];
            }
            best_proj += best_model[0].W0;
            ind = 0 ;
            for (int cl_num = 1; cl_num < prob->num_class; ++cl_num) {
                for (int j = 0; j < prob->cols; ++j) {
                    temp_proj += prob->data[i*prob->cols+j] * best_model[cl_num].W[j];
                }
                temp_proj += best_model[0].W0;
                if(best_proj < temp_proj){
                    best_proj = temp_proj;
                    ind = cl_num;
                }
            }
            fprintf(fout,"%d\n",class_lbls[ind]);
            if(class_lbls[ind] != prob->labels[i]){
                error += 1;
            }
        }
        std::cout << "error %" <<100* error / prob->rows << std::endl;
    }else if(predictParam.pred_strategy == 3){
        std::cerr << "Unrecognized Option ! Please run Train again...";
    } else{
        std::cerr << "Unrecognized Option ! Please run Train again...";
    }
}

void ezp_multi_transform(std::ifstream &model_file, problem *prob, predict_param &predictParam,
                         std::string &output_file) {

    std::string cl_output;
    double temp_proj(0),output(0);
    int ind(0) ;
    if(predictParam.pred_strategy < 5) {
        for (int cl_num = 0; cl_num < predictParam.num_class; ++cl_num) {
            cl_output = output_file + std::to_string(cl_num);
            FILE* fout = fopen(cl_output.c_str(),"w");
            model *raw_model = read_model(model_file, predictParam, prob->cols);
            model *best_m ;
            if(predictParam.pred_strategy == 3) {
                best_m = pick_best_obj(raw_model, predictParam, prob->cols);
            }else {
                best_m = pick_best_err(raw_model, predictParam, prob->cols);
            }
            for (int i = 0; i < prob->rows; ++i) {
                if(prob->labels[i] == predictParam.pos_class)
                    fprintf(fout,"1 ");
                else
                    fprintf(fout,"-1 ");
                temp_proj = 0;
                for (int k = 0; k < predictParam.ezp_size; ++k) {
                    for (int j = 0; j < prob->cols; ++j) {
                        temp_proj += prob->data[i*prob->cols+j] * best_m[k].W[j];
                    }
                    temp_proj += best_m[k].W0;
                    output = predictParam.ezp_lbl_func(temp_proj);
                    fprintf(fout,"%f ",output);
                }
                fprintf(fout,"\n");
            }
            fclose(fout);
        }
    }else if(predictParam.pred_strategy == 5){
        for (int cl_num = 0; cl_num < predictParam.num_class; ++cl_num) {
            cl_output = output_file + std::to_string(cl_num);
            FILE* fout = fopen(cl_output.c_str(),"w");
            model *raw_model = read_model(model_file, predictParam, prob->cols);
            for (int i = 0; i < prob->rows; ++i) {
                if(prob->labels[i] == predictParam.pos_class)
                    fprintf(fout,"1 ");
                else
                    fprintf(fout,"-1 ");
                temp_proj = 0;
                for (int k = 0; k < predictParam.ezp_size*predictParam.ils_itr; ++k) {
                    for (int j = 0; j < prob->cols; ++j) {
                        temp_proj += prob->data[i*prob->cols+j] * raw_model[k].W[j];
                    }
                    temp_proj += raw_model[k].W0;
                    output = predictParam.ezp_lbl_func(temp_proj);
                    fprintf(fout,"%f ",output);
                }
                fprintf(fout,"\n");
            }
            fclose(fout);
        }
    } else{
        std::cerr << "Unrecognized Option ! Please run Train again...";
    }

}