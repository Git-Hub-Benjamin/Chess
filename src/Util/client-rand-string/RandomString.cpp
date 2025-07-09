#include "RandomString.hpp"

// Function to generate a random alphanumeric string
std::string generateRandomString(int length) {
    std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

    // Generate random characters and append them to the result string
    for (int i = 0; i < length; ++i)
        result += charset[distribution(gen)];
    
    return result;
}

