#include <cstdint>
#include <filesystem>
#include <fstream>
#include <format>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

struct ListNode {
    ListNode* prev = nullptr;
    ListNode* next = nullptr;
    ListNode* rand = nullptr;
    std::string data;
};

struct List {
    ListNode* head = nullptr;
    ListNode* tail = nullptr;
    std::size_t size = 0;

    List() = default;

    ~List() {
        clear();
    }

    List(const List&) = delete;
    List& operator=(const List&) = delete;

    List(List&& other) noexcept
        : head(other.head), tail(other.tail), size(other.size) {
        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
    }

    List& operator=(List&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        clear();
        head = other.head;
        tail = other.tail;
        size = other.size;

        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
        return *this;
    }

    void clear() noexcept {
        while (head != nullptr) {
            ListNode* next = head->next;
            delete head;
            head = next;
        }

        tail = nullptr;
        size = 0;
    }
};

std::string trim(const std::string& value) {
    const char* whitespace = " \t\r\n\f\v";

    const std::size_t begin = value.find_first_not_of(whitespace);
    if (begin == std::string::npos) {
        return "";
    }

    const std::size_t end = value.find_last_not_of(whitespace);
    return value.substr(begin, end - begin + 1);
}

bool isValidNumber(const std::string& value) {
    if (value.empty()) {
        return false;
    }

    int start = 0;
    if (value[0] == '-') {
        if (value.size() == 1) {
            return false;
        }

        start = 1;
    }

    for (int i = start; i < value.size(); ++i) {
        if (value[i] < '0' || value[i] > '9') {
            return false;
        }
    }

    return true;
}

void appendNode(List& list, ListNode* node) {
    if (list.head == nullptr) {
        list.head = node;
        list.tail = node;
        ++list.size;
        return;
    }

    list.tail->next = node;
    node->prev = list.tail;
    list.tail = node;
    ++list.size;
}

template <typename T>
void write(std::ostream& stream, const T& value) {
    stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    if (!stream) {
        throw std::runtime_error("Failed to write binary data");
    }
}

List loadList(const std::filesystem::path& inputPath) {
    std::ifstream input(inputPath);
    if (!input.is_open()) {
        throw std::runtime_error(std::format("Failed to open input file: {}", inputPath.string()));
    }

    List list;
    std::vector<ListNode*> nodes;
    std::vector<int> randIndices;

    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(input, line)) {
        ++lineNumber;

        const std::size_t delimiterPos = line.rfind(';');
        if (delimiterPos == std::string::npos) {
            throw std::runtime_error(std::format("Line {} does not contain ';'", lineNumber));
        }

        std::string randToken = trim(line.substr(delimiterPos + 1));
        long long randIndex = 0;

        if (!isValidNumber(randToken)) {
            throw std::runtime_error(std::format("Line {} contains invalid rand index", lineNumber));
        }

        try {
            randIndex = std::stoll(randToken);
        } 
        catch (...) {
            throw std::runtime_error(std::format("Line {} contains invalid rand index", lineNumber));
        }

        if (randIndex < -1 || randIndex > std::numeric_limits<std::int32_t>::max()) {
            throw std::runtime_error(std::format("Line {} contains out-of-range rand index", lineNumber));
        }

        ListNode* node = new ListNode;
        node->data = line.substr(0, delimiterPos);

        if (node->data.size() > 1000) {
            delete node;
            throw std::runtime_error(std::format("Line {} contains data longer than 1000 bytes", lineNumber));
        }

        appendNode(list, node);

        nodes.push_back(node);
        randIndices.push_back(static_cast<std::int32_t>(randIndex));
    }

    if (list.size > 1'000'000) {
        throw std::runtime_error("List contains more than 1,000,000 nodes");
    }

    for (std::size_t i = 0; i < nodes.size(); ++i) {
        if (randIndices[i] == -1) {
            continue;
        }

        if (randIndices[i] < 0 || randIndices[i] >= nodes.size()) {
            throw std::runtime_error(std::format("Line {} refers to rand index outside the list", i + 1));
        }

        nodes[i]->rand = nodes[randIndices[i]];
    }

    return list;
}

void serializeList(const List& list, const std::filesystem::path& outputPath) {
    std::ofstream output(outputPath, std::ios::binary);
    if (!output.is_open()) {
        throw std::runtime_error(std::format("Failed to open output file: {}", outputPath.string()));
    }

    std::vector<ListNode*> nodes;
    std::unordered_map<ListNode*, std::uint32_t> indices;
    nodes.reserve(list.size);

    for (ListNode* node = list.head; node != nullptr; node = node->next) {
        const std::uint32_t index = static_cast<std::uint32_t>(nodes.size());
        nodes.push_back(node);
        indices.emplace(node, index);
    }

    write(output, static_cast<std::uint32_t>(nodes.size()));

    for (const ListNode* node : nodes) {
        const auto dataSize = static_cast<std::uint32_t>(node->data.size());
        const std::int32_t randIndex =
            node->rand == nullptr ? -1 : static_cast<std::int32_t>(indices.at(node->rand));

        write(output, dataSize);
        write(output, randIndex);

        output.write(node->data.data(), static_cast<std::streamsize>(node->data.size()));
        if (!output) {
            throw std::runtime_error("Failed to write node payload");
        }
    }
}

int main() {
    try {
        const auto inputPath = std::filesystem::current_path() / "inlet.in";
        const auto outputPath = std::filesystem::current_path() / "outlet.out";

        List inputList = loadList(inputPath);
        const std::size_t listSize = inputList.size;

        serializeList(inputList, outputPath);
        std::cout << std::format("Serialized {} nodes to {}\n", listSize, outputPath.string());
        return 0;
    } 
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}