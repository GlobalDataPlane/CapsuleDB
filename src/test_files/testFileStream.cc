#include <fstream>
#include <iostream>

int main(){
    std::ifstream inputFile("outputFile.txt");
    std::string savedString;
    inputFile >> savedString;
    std::cout << savedString;
    inputFile.close();   
}