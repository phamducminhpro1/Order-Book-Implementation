#include <iostream>
#include <sstream>
#include <vector>

int main() {
    std::string coordinates = "(25, 4) (1, -6)";

    // Remove parentheses and split the string by spaces
    std::istringstream iss(coordinates);
    std::vector<int> numbers;
    std::string coordinate;
    while (std::getline(iss, coordinate, ' ')) {
        // Remove parentheses from the coordinate
        coordinate = coordinate.substr(1, coordinate.length() - 2);
        
        // Split the coordinate by comma
        std::istringstream iss_coordinate(coordinate);
        std::string number_str;
        while (std::getline(iss_coordinate, number_str, ',')) {
            // Convert the number string to int and add it to the vector
            numbers.push_back(std::stoi(number_str));
        }
    }

    // Print the vector
    for (int number : numbers) {
        std::cout << number << " ";
    }
    std::cout << std::endl;

    return 0;
}