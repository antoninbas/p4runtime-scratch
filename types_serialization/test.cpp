#include "test.pb.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define NUM_MSGS (1 << 24)

using Clock = std::chrono::steady_clock;

size_t allocated_bytes = 0;

void* operator new(std::size_t sz) {
  allocated_bytes += sz;
  return std::malloc(sz);
}

void operator delete(void* ptr) noexcept {
  std::free(ptr);
}

enum class Format { BITSTRING, WRAPPER };

template <typename T>
size_t computeSerializedSize(const std::vector<T> &msgs) {
  size_t serialized_size = 0;
  std::string s;
  for (int i = 0; i < NUM_MSGS; i++) {
    auto &msg = msgs.at(i);
    msg.SerializeToString(&s);
    serialized_size += s.size();
  }
  return serialized_size;
}

void printStats(size_t serialized_size,
                const Clock::time_point &start,
                const Clock::time_point &end) {
  std::cout << "Test run with " << NUM_MSGS << " msgs\n";
  std::cout << "Heap-allocated bytes: " << allocated_bytes << "\n";
  std::cout << "Heap-allocated bytes per message: "
            << static_cast<double>(allocated_bytes) / NUM_MSGS << "\n";
  std::cout << "Serialized size: " << serialized_size << "\n";
  std::cout << "Serialized size per message: "
            << static_cast<double>(serialized_size) / NUM_MSGS  << "\n";
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end - start);
  std::cout << "Test duration: " << duration.count() << " ms\n";
  std::cout << "Test duration per message: " <<
      static_cast<double>(duration.count()) / NUM_MSGS << " ms\n";
}

template <typename T, typename Builder>
void run(const Builder &fn) {
  auto start = Clock::now();
  std::vector<T> msgs(NUM_MSGS);
  for (int i = 0; i < NUM_MSGS; i++) {
    auto &msg = msgs.at(i);
    fn(msg, i);
  }
  auto serialized_size = computeSerializedSize(msgs);
  auto end = Clock::now();
  printStats(serialized_size, start, end);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <bitstring|wrapper>\n";
    return 1;
  }
  Format format = Format::BITSTRING;
  if (std::string(argv[1]) == "wrapper") {
    format = Format::WRAPPER;
  } else if (std::string(argv[1]) != "bitstring") {
    std::cout << "Usage: " << argv[0] << " <bitstring|wrapper>\n";
    return 1;
  }

  if (format == Format::BITSTRING) {
    run<p4::Bitstring>([](p4::Bitstring &msg, int i){
        msg.set_bitstring(std::to_string(i)); });
  } else if (format == Format::WRAPPER) {
    run<p4::DataWrapper>([](p4::DataWrapper &msg, int i){
        auto *data = msg.mutable_data();
        data->set_bitstring(std::to_string(i)); });
  }

  return 0;
}
