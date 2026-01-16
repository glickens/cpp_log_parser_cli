# cpp_log_parser_cli

A C++ command-line tool that parses log files and prints summary stats (counts, errors, top messages).  

## Features
- Reads a log file (plain text)
- Counts total lines and log levels (INFO/WARN/ERROR)
- Shows **Top N** most frequent messages
- Basic input validation and helpful usage output

## Example log format
This tool supports lines like:

2026-01-15 10:03:21 INFO  AuthService - User login ok
2026-01-15 10:03:22 WARN  Billing     - Slow query detected
2026-01-15 10:03:23 ERROR Billing     - ORA-12541: TNS no listener

## Build (Linux / macOS)
```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -o log_parser src/main.cpp

## Run
./log_parser sample_logs/sample.log

## Optional: show top 10 messages

./log_parser sample_logs/sample.log --top 10

Output (example)

## Real output (from sample_logs/sample.log)

```text
Summary
-------
Total lines: 20

Log levels:
  INFO: 8
  WARN: 4
  ERROR: 5
  DEBUG: 3

Top messages:
  1) Cache miss (3)
  2) ORA-12541: TNS no listener (3)
  3) User login ok (3)
  4) Invalid token (2)
  5) Packet loss detected (2)
