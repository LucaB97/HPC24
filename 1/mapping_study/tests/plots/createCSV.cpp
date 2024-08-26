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

    // Extract operation and algorithm from the input file name
    std::string operation, algorithm;
    std::size_t underscorePos = inputFileName.find("__");
    std::size_t dotPos = inputFileName.find(".txt");

    if (underscorePos != std::string::npos && dotPos != std::string::npos) {
        operation = inputFileName.substr(0, underscorePos);
        algorithm = inputFileName.substr(underscorePos + 2, dotPos - underscorePos - 2);
    }

    // Write the header to the CSV file
    outputFile << "Operation,Algorithm,Mapping,Size,Avg Latency(us)\n";

    std::string line;
    std::string mapping;
    bool parsingData = false;

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);

        // Detect when data starts by looking for lines starting with a digit
        if (std::isdigit(line[0])) {
            parsingData = true;
            int size;
            float avgLatency;

            iss >> size >> avgLatency;

            outputFile << operation << ','
                       << algorithm << ','
                       << mapping << ','
                       << size << ',' << avgLatency << '\n';
        } 
        else if (line.find("Mapping:") != std::string::npos) {
            // Extract the Mapping from the file
            iss.ignore(9); // Skip the "Mapping: " part
            iss >> mapping;
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
