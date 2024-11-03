#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <filesystem>

const int TOP_WORDS = 100;
const std::string STOP_WORDS[] = {"a", "and", "an", "of", "in", "the"};
const std::string BASE_PATH = "F:\\Downloads\\IITG Tri 1 Project\\Book-Txt\\";

struct Book {
    std::string name;
    std::unordered_map<std::string, double> wordFrequency;
    double totalWords = 0.0;
};

std::string normalizeWord(const std::string& word) {
    std::string normalized;
    for (char ch : word) {
        if (isalpha(ch) || isdigit(ch)) {
            normalized += toupper(ch);
        }
    }
    return normalized;
}

void processBook(Book& book) {
    std::ifstream file(book.name);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << book.name << std::endl;
        return;
    }

    std::unordered_map<std::string, int> rawFrequency;
    std::string word;

    while (file >> word) {
        std::string normalized = normalizeWord(word);
        if (normalized.empty()) continue;

        if (std::find(std::begin(STOP_WORDS), std::end(STOP_WORDS), normalized) == std::end(STOP_WORDS)) {
            rawFrequency[normalized]++;
            book.totalWords++;
        }
    }

    for (const auto& pair : rawFrequency) {
        book.wordFrequency[pair.first] = pair.second / book.totalWords;
    }

    file.close();
}

double calculateSimilarity(const Book& bookA, const Book& bookB) {
    double similarity = 0.0;
    for (const auto& word : bookA.wordFrequency) {
        auto it = bookB.wordFrequency.find(word.first);
        if (it != bookB.wordFrequency.end()) {
            similarity += word.second + it->second;
        }
    }
    return similarity;
}

int main() {
    std::vector<Book> books;

    // Use filesystem to read all .txt files in the specified directory
    for (const auto& entry : std::filesystem::directory_iterator(BASE_PATH)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            Book book;
            book.name = entry.path().string();
            processBook(book);
            books.push_back(book);
        }
    }

    // Create similarity matrix
    size_t numBooks = books.size();
    std::vector<std::vector<double>> similarityMatrix(numBooks, std::vector<double>(numBooks, 0.0));

    for (size_t i = 0; i < numBooks; ++i) {
        for (size_t j = 0; j < numBooks; ++j) {
            if (i != j) {
                similarityMatrix[i][j] = calculateSimilarity(books[i], books[j]);
            }
        }
    }

    // Find top 10 similar pairs
    using Pair = std::pair<double, std::pair<int, int>>;
    std::vector<Pair> similarities;

    for (size_t i = 0; i < numBooks; ++i) {
        for (size_t j = i + 1; j < numBooks; ++j) {
            similarities.emplace_back(similarityMatrix[i][j], std::make_pair(i, j));
        }
    }

    std::sort(similarities.rbegin(), similarities.rend());

    std::cout << "Top 10 Similar Pairs of Textbooks:\n";
    for (size_t i = 0; i < 10 && i < similarities.size(); ++i) {
        std::cout << "Books \"" << books[similarities[i].second.first].name << "\" and \"" 
                  << books[similarities[i].second.second].name << "\" with similarity: " 
                  << std::fixed << std::setprecision(4) << similarities[i].first << '\n';
    }

    return 0;
}
