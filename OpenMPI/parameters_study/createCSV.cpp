#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

void createCSV(const std::string& inputFileName, const std::string& outputFileName) {
    std::ifstream inputFile(inputFileName);
    std::ofstream outputFile(outputFileName);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    // Write the header to the CSV file
    // outputFile << "Operation,Algorithm,Mapping,Size,Avg Latency(us),Min Latency(us),Max Latency(us),Iterations\n";

    std::string line;
    std::string operation, algorithm, mapping;
    int iterations, warmup;
    bool parsingData = false;

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);

        // Read Operation, Algorithm, and Mapping from the file
        if (line.find("Operation:") != std::string::npos) {
            parsingData = false;
            iss.ignore(10);
            iss >> operation;
        }
        if (line.find("Algorithm:") != std::string::npos) {
            iss.ignore(11);
            iss >> algorithm;
        }
        if (line.find("Mapping:") != std::string::npos) {
            iss.ignore(9);
            iss >> mapping;
        }
        // Read Iterations and Warmup (not directly used in the CSV output but to control the loop)
        if (line.find("Iterations:") != std::string::npos) {
            parsingData = true;
            iss.ignore(11);
            iss >> iterations;
        }
        if (line.find("Warmup:") != std::string::npos) {
            iss.ignore(8);
            iss >> warmup;
        }

        // Parse and write latency data to the CSV file
        if (parsingData && std::isdigit(line[0])) {
            int size;
            float avgLatency, minLatency, maxLatency;
            // int iters;

            iss >> size >> avgLatency >> minLatency >> maxLatency;

            outputFile << operation << ',' << algorithm << ',' << mapping << ','
                       << iterations << ',' << warmup << ','
                       << size << ',' << avgLatency << ',' << minLatency << ',' << maxLatency <<'\n';
        }
    }

    inputFile.close();
    outputFile.close();
    // std::cout << "CSV file created successfully!" << std::endl;
}



int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: ./createCSV.x <input_file> <output_file>.\n");
        exit(-1);
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    createCSV(input_file, output_file);

    return 0;
}