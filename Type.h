#ifndef TYPE_H
#define TYPE_H

#include <array>
#include <cstdint>
#include <string>
#include <chrono>

using Array = std::array;
using String = std::string;
using InputFileStream = std::ifstream;
using NanoTimePoint = std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long long, std::ratio<1, 1000000000>>>;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = int32_t;
using u64 = int64_t;
using f32 = float;
using f64 = double;

#endif //TYPE_H
