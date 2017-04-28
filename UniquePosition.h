// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _CLOUD_UNIQUE_POSITION_H_
#define _CLOUD_UNIQUE_POSITION_H_

#include <string>
#include <vector>
#include <node.h>
#include <node_object_wrap.h>

using namespace std;
using namespace v8;
using namespace node;

typedef long long int64;
typedef unsigned long long uint64;
typedef unsigned char uint8;
typedef unsigned int uint32_t;
typedef uint32_t uint32;
typedef int int32;

const uint8 kuint8max = ((uint8)0xFF);
const int32 kint32max = ((int32)0x7FFFFFFF);

// Utility to convert a character to a digit in a given base
template <typename CHAR, int BASE, bool BASE_LTE_10>
class BaseCharToDigit
{
};

// Faster specialization for bases <= 10
template <typename CHAR, int BASE>
class BaseCharToDigit<CHAR, BASE, true>
{
public:
  static bool Convert(CHAR c, uint8 *digit)
  {
    if (c >= '0' && c < '0' + BASE)
    {
      *digit = c - '0';
      return true;
    }
    return false;
  }
};

// Specialization for bases where 10 < base <= 36
template <typename CHAR, int BASE>
class BaseCharToDigit<CHAR, BASE, false>
{
public:
  static bool Convert(CHAR c, uint8 *digit)
  {
    if (c >= '0' && c <= '9')
    {
      *digit = c - '0';
    }
    else if (c >= 'a' && c < 'a' + BASE - 10)
    {
      *digit = c - 'a' + 10;
    }
    else if (c >= 'A' && c < 'A' + BASE - 10)
    {
      *digit = c - 'A' + 10;
    }
    else
    {
      return false;
    }
    return true;
  }
};

template <int BASE, typename CHAR>
bool CharToDigit(CHAR c, uint8 *digit)
{
  return BaseCharToDigit<CHAR, BASE, BASE <= 10>::Convert(c, digit);
}

template <typename STR>
bool HexStringToBytesT(const STR &input, std::vector<uint8> *output)
{
  size_t count = input.size();
  if (count == 0 || (count % 2) != 0)
    return false;
  for (uintptr_t i = 0; i < count / 2; ++i)
  {
    uint8 msb = 0; // most significant 4 bits
    uint8 lsb = 0; // least significant 4 bits
    if (!CharToDigit<16>(input[i * 2], &msb) ||
        !CharToDigit<16>(input[i * 2 + 1], &lsb))
      return false;
    output->push_back((msb << 4) | lsb);
  }
  return true;
}

class UniquePosition : public node::ObjectWrap
{
public:
  static const size_t kSuffixLength;
  static const size_t kCompressBytesThreshold;

  static void Init(Local<Object> exports, Local<Object> module);

  explicit UniquePosition(const std::string &internal_rep);
  ~UniquePosition();

  static bool IsValidSuffix(const std::string &suffix);
  static void IsValidSuffix_(const v8::FunctionCallbackInfo<v8::Value> &args);

  static bool IsValidBytes(const std::string &bytes);
  static void IsValidBytes_(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void New(const v8::FunctionCallbackInfo<v8::Value> &args);

  // Returns an invalid position. ?
  // static UniquePosition CreateInvalid();
  static void CreateInvalid(const v8::FunctionCallbackInfo<v8::Value> &args);

  // This constructor creates an invalid value.
  UniquePosition();

  // Returns a valid position.  Its ordering is not defined.
  //  static void InitialPosition(const std::string& suffix);
  static void InitialPosition(const v8::FunctionCallbackInfo<v8::Value> &args);

  // static UniquePosition Before(const UniquePosition& x, const std::string& suffix);
  static void Before(const v8::FunctionCallbackInfo<v8::Value> &args);

  // static UniquePosition After(const UniquePosition& x, const std::string& suffix);
  static void After(const v8::FunctionCallbackInfo<v8::Value> &args);

  // static UniquePosition Between(const UniquePosition& before, const UniquePosition& after, const std::string& suffix);
  static void Between(const v8::FunctionCallbackInfo<v8::Value> &args);

  bool LessThan(const UniquePosition &other) const;
  static void LessThan_(const v8::FunctionCallbackInfo<v8::Value> &args);

  // equals
  bool Equals(const UniquePosition &other) const;
  static void Equals_(const v8::FunctionCallbackInfo<v8::Value> &args);

  // Returns a human-readable representation of this item's internal state.
  std::string ToDebugString() const;
  static void ToDebugString_(const v8::FunctionCallbackInfo<v8::Value> &args);

  // Returns the suffix.
  std::string GetSuffixForTest() const;
  static void GetSuffixForTest_(const v8::FunctionCallbackInfo<v8::Value> &args);

  std::string getCompressValue(const bool bIsBase64) const;
  static void getCompressValue_(const v8::FunctionCallbackInfo<v8::Value> &args);

  // Performs a lossy conversion to an int64 position.  Positions converted to
  // and from int64s using this and the FromInt64 function should maintain their
  // relative orderings unless the int64 values conflict.

  // UniquePosition FromInt64(int64 x, const std::string &suffix);
  static void FromInt64(const v8::FunctionCallbackInfo<v8::Value> &args);

  int64 ToInt64() const;
  static void ToInt64_(const v8::FunctionCallbackInfo<v8::Value> &args);

  bool IsValid() const;
  static void IsValid_(const v8::FunctionCallbackInfo<v8::Value> &args);

private:
  static v8::Persistent<v8::Function> constructor;

  // Returns a string X such that (X ++ |suffix|) > |str|.
  // |str| must be a trailing substring of a valid ordinal.
  // |suffix| must be a valid unique suffix.
  static std::string FindSmallerWithSuffix(const std::string &str,
                                           const std::string &suffix);
  // Returns a string X such that (X ++ |suffix|) > |str|.
  // |str| must be a trailing substring of a valid ordinal.
  // |suffix| must be a valid unique suffix.
  static std::string FindGreaterWithSuffix(const std::string &str,
                                           const std::string &suffix);
  // Returns a string X such that |before| < (X ++ |suffix|) < |after|.
  // |before| and after must be a trailing substrings of valid ordinals.
  // |suffix| must be a valid unique suffix.
  static std::string FindBetweenWithSuffix(const std::string &before,
                                           const std::string &after,
                                           const std::string &suffix);

  // Expects a run-length compressed string as input.  For internal use only.
  // Expects an uncompressed prefix and suffix as input.  The |suffix| parameter
  // must be a suffix of |uncompressed|.  For internal use only.
  UniquePosition(const std::string &uncompressed, const std::string &suffix);

  // Implementation of an order-preserving run-length compression scheme.
  static std::string Compress(const std::string &input);
  static std::string CompressImpl(const std::string &input);
  static std::string Uncompress(const std::string &compressed);
  static bool IsValidCompressed(const std::string &str);

  std::string compressed_;
  bool is_valid_;
};

#endif //_CLOUD_UNIQUE_POSITION_H_
