#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <iomanip>
#include <cmath>
#include <filesystem>

void summarize(const std::string& input_file, const std::string& output_file) {

    std::map<int, double> data;
    std::map<int, double> errors;     
    std::ifstream input(input_file);    
    std::string line;
    int size;
    double time;
    int iter = 0;
    int algorithm, nprocs;

    while (std::getline(input, line)) {
        
        if (line.find("Algorithm") != std::string::npos) {
            algorithm = std::stoi(line.substr(line.find(" ")));
        }
        else if (line.find("Procs") != std::string::npos) {
            nprocs = std::stoi(line.substr(line.find(" ")));
        }
        else if (line.find("Iter") != std::string::npos) {
            iter++;
        } 
        else if (line.find("#") != std::string::npos) {
            continue;
        }
        else if (!line.empty()) {
            size = std::stoi(line);
            time = std::stod(line.substr(line.find(" ")));
            data[size] += time;
        }
    }
    input.close();    
    
    for (auto& pair : data) {
        pair.second /= iter;
    }

    input.open(input_file); 
    double error;

    while (std::getline(input, line)) {
        if (line.find("Algorithm") != std::string::npos || 
        line.find("Procs") != std::string::npos || 
        line.find("Iter") != std::string::npos || 
        line.find("#") != std::string::npos) {
            continue;
        }
        if (!line.empty()) {
            size = std::stoi(line);
            time = std::stod(line.substr(line.find(" ")));
            error = std::pow((time - data[size]), 2); 
            errors[size] += error;
        }
    }
    input.close();  

    std::fstream output(output_file, std::ios_base::app);
    
    if (std::filesystem::is_empty(output_file)) {
        output << "Algorithm,Numprocs,Size,Avg Latency(us) mean,Avg Latency(us) sd" << std::endl;
    }        
    
    for (auto& pair : errors) {
        pair.second = std::sqrt(pair.second / iter);
        output << algorithm << "," << nprocs << "," << pair.first << "," << data[pair.first] << "," << pair.second << std::endl;
    }
    output.close();
}


int main (int argc, char** argv) {    

    if (argc < 3) {
        printf("Usage: ./summarize.x <input_file> <output_file>.\n");
        exit(-1);
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    summarize(input_file, output_file);

    return 0;
}