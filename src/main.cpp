#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

/*
  cpp_log_parser_cli
  ------------------
  Reads a log file and prints:
   - Total lines
   - Count of INFO/WARN/ERROR (and other levels if present)
   - Top N most frequent messages

  Expected (flexible) log format examples:
    2026-01-15 10:03:21 INFO  AuthService - User login ok
    2026-01-15 10:03:22 WARN  Billing     - Slow query detected
    2026-01-15 10:03:23 ERROR Billing     - ORA-12541: TNS no listener
*/

struct Stats {
    long long total_lines = 0;
    std::unordered_map<std::string, long long> level_counts;
    std::unordered_map<std::string, long long> message_counts;
};

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }
    return s.substr(start, end - start);
}

// Attempts to extract a "level" token (INFO/WARN/ERROR/DEBUG/TRACE/etc.) from a line.
// This is intentionally simple and robust for entry-level practice.
static std::string extract_level(const std::string& line) {
    // Common levels to detect.
    static const std::vector<std::string> levels = {
        "TRACE", "DEBUG", "INFO", "WARN", "WARNING", "ERROR", "FATAL"
    };

    // Scan words and match case-insensitively.
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        // Uppercase the token for comparison.
        std::string upper = token;
        for (char& c : upper) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        for (const auto& lvl : levels) {
            if (upper == lvl) {
                // Normalize WARNING -> WARN
                if (upper == "WARNING") return "WARN";
                return upper;
            }
        }
    }
    return "UNKNOWN";
}

// Extracts message part. If line contains " - " use text after it.
// Otherwise, return the full line trimmed (still useful for counting repeats).
static std::string extract_message(const std::string& line) {
    const std::string marker = " - ";
    size_t pos = line.find(marker);
    if (pos != std::string::npos) {
        return trim(line.substr(pos + marker.size()));
    }
    return trim(line);
}

static void print_usage(const std::string& exe) {
    std::cout << "Usage:\n"
              << "  " << exe << " <log_file_path> [--top N]\n\n"
              << "Examples:\n"
              << "  " << exe << " sample_logs/sample.log\n"
              << "  " << exe << " sample_logs/sample.log --top 10\n";
}

static bool parse_args(int argc, char** argv, std::string& filepath, int& top_n) {
    top_n = 5; // default top messages count

    if (argc < 2) {
        return false;
    }

    filepath = argv[1];

    // Optional: --top N
    if (argc == 4) {
        std::string flag = argv[2];
        if (flag != "--top") return false;

        try {
            top_n = std::stoi(argv[3]);
            if (top_n < 1) top_n = 1;
        } catch (...) {
            return false;
        }
    } else if (argc != 2) {
        // We only support: program <file> OR program <file> --top N
        return false;
    }

    return true;
}

static bool process_file(const std::string& filepath, Stats& stats) {
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(infile, line)) {
        stats.total_lines++;

        std::string level = extract_level(line);
        stats.level_counts[level]++;

        std::string msg = extract_message(line);
        if (!msg.empty()) {
            stats.message_counts[msg]++;
        }
    }

    return true;
}

static void print_results(const Stats& stats, int top_n) {
    std::cout << "\nSummary\n";
    std::cout << "-------\n";
    std::cout << "Total lines: " << stats.total_lines << "\n\n";

    // Print level counts in a stable order (common levels first).
    std::vector<std::string> common = {"INFO", "WARN", "ERROR", "DEBUG", "TRACE", "FATAL", "UNKNOWN"};

    std::cout << "Log levels:\n";
    for (const auto& lvl : common) {
        auto it = stats.level_counts.find(lvl);
        if (it != stats.level_counts.end()) {
            std::cout << "  " << lvl << ": " << it->second << "\n";
        }
    }
    // Print any additional levels not in "common"
    for (const auto& kv : stats.level_counts) {
        if (std::find(common.begin(), common.end(), kv.first) == common.end()) {
            std::cout << "  " << kv.first << ": " << kv.second << "\n";
        }
    }

    // Compute top N messages
    std::vector<std::pair<std::string, long long>> pairs;
    pairs.reserve(stats.message_counts.size());
    for (const auto& kv : stats.message_counts) {
        pairs.push_back(kv);
    }

    std::sort(pairs.begin(), pairs.end(),
              [](const auto& a, const auto& b) {
                  if (a.second != b.second) return a.second > b.second; // higher count first
                  return a.first < b.first; // tie-break alphabetically
              });

    std::cout << "\nTop messages:\n";
    int limit = std::min(top_n, static_cast<int>(pairs.size()));
    for (int i = 0; i < limit; i++) {
        std::cout << "  " << (i + 1) << ") " << pairs[i].first << " (" << pairs[i].second << ")\n";
    }
    if (limit == 0) {
        std::cout << "  (No messages found)\n";
    }

    std::cout << "\n";
}

int main(int argc, char** argv) {
    std::string filepath;
    int top_n = 5;

    if (!parse_args(argc, argv, filepath, top_n)) {
        print_usage(argc > 0 ? argv[0] : "log_parser");
        return 1;
    }

    Stats stats;
    if (!process_file(filepath, stats)) {
        std::cerr << "Error: Could not open file: " << filepath << "\n";
        return 2;
    }

    print_results(stats, top_n);
    return 0;
}
