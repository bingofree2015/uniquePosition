#include "uniquePosition.h"
#include "base64.h"
#include <iostream>
#include <node.h>
#include <v8.h>
#include <node_buffer.h>
#include <nan.h>

#include <stdio.h>

#define LOG_ERROR std::cout

#define DCHECK(x) {if(!x) LOG_ERROR << " dcheck error" << endl;}

#define DCHECK_OP(name, op, val1, val2)  {bool test=val1 op val2; if(!test) LOG_ERROR << "failed dcheck: " << #val1 " " #op " " #val2 <<  endl;}

#define DCHECK_EQ(val1, val2) DCHECK_OP(EQ, ==, val1, val2)
#define DCHECK_NE(val1, val2) DCHECK_OP(NE, !=, val1, val2)
#define DCHECK_LE(val1, val2) DCHECK_OP(LE, <=, val1, val2)
#define DCHECK_LT(val1, val2) DCHECK_OP(LT, < , val1, val2)
#define DCHECK_GE(val1, val2) DCHECK_OP(GE, >=, val1, val2)
#define DCHECK_GT(val1, val2) DCHECK_OP(GT, > , val1, val2)

#define CHECK_GE(val1, val2) DCHECK_OP(GE, >=, val1, val2)
#define CHECK_LT(val1, val2) DCHECK_OP(LT, <, val1, val2)

std::string HexEncode(const void* bytes, size_t size) {
  static const char kHexChars[] = "0123456789ABCDEF";

  // Each input byte creates two output hex characters.
  std::string ret(size * 2, '\0');

  for (size_t i = 0; i < size; ++i) {
    char b = reinterpret_cast<const char*>(bytes)[i];
    ret[(i * 2)] = kHexChars[(b >> 4) & 0xf];
    ret[(i * 2) + 1] = kHexChars[b & 0xf];
  }
  return ret;
}

std::string Hexdecode(const string &input)
{
	vector<uint8> output;
	HexStringToBytesT(input, &output);
	string ret;
	ret.assign(output.begin(), output.end());
	return ret;
}

const size_t UniquePosition::kSuffixLength = 28;
const size_t UniquePosition::kCompressBytesThreshold = 128;

Persistent<Function> UniquePosition::constructor;

//
UniquePosition::UniquePosition(
	const std::string& internal_rep,
	const bool bIsBase64) {
//    : compressed_(internal_rep),
//      is_valid_(IsValidBytes(Uncompress(internal_rep)))
    std::string sinternal_rep = "";
    if (bIsBase64) {
    	sinternal_rep = base64_decode(internal_rep.c_str(), internal_rep.length());
    }
    else {
    	sinternal_rep = internal_rep;
    }
    compressed_ = sinternal_rep,
    is_valid_ = IsValidBytes(Uncompress(sinternal_rep));
}

UniquePosition::UniquePosition(
    const std::string& uncompressed,
    const std::string& suffix)
  : compressed_(Compress(uncompressed)),
    is_valid_(IsValidBytes(uncompressed)) {
    DCHECK(uncompressed.rfind(suffix) + kSuffixLength == uncompressed.length());
    DCHECK(IsValidSuffix(suffix));
    DCHECK(IsValid());
}

UniquePosition::UniquePosition() : is_valid_(false) {
}

UniquePosition::~UniquePosition() {
}

void UniquePosition::Init(Local<Object> exports, Local<Object> module) {
    Isolate* isolate = Isolate::GetCurrent();

    // Function模板
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    // 类名
    tpl->SetClassName(String::NewFromUtf8(isolate, "UniquePosition"));
    // InternalField
    tpl->InstanceTemplate()->SetInternalFieldCount(2);
    // 设置Prototype函数
    NODE_SET_METHOD(exports, "IsValidBytes", IsValidBytes_); // static
    NODE_SET_METHOD(exports, "IsValidSuffix", IsValidSuffix_); // static
    NODE_SET_PROTOTYPE_METHOD(tpl, "getCompressValue", getCompressValue_);
    NODE_SET_PROTOTYPE_METHOD(tpl, "ToDebugString", ToDebugString_);

    NODE_SET_METHOD(exports, "CreateInvalid", CreateInvalid); // static
    NODE_SET_METHOD(exports, "InitialPosition", InitialPosition); // static
    NODE_SET_METHOD(exports, "FromInt64", FromInt64); // static
    NODE_SET_METHOD(exports, "Before", Before); // static
    NODE_SET_METHOD(exports, "After", After); // static
    NODE_SET_METHOD(exports, "Between", Between); // static

    NODE_SET_PROTOTYPE_METHOD(tpl, "Equals", Equals_);
    NODE_SET_PROTOTYPE_METHOD(tpl, "GetSuffixForTest", GetSuffixForTest_);
    NODE_SET_PROTOTYPE_METHOD(tpl, "ToInt64", ToInt64_);
    NODE_SET_PROTOTYPE_METHOD(tpl, "IsValid", IsValid_);
    NODE_SET_PROTOTYPE_METHOD(tpl, "LessThan", LessThan_);
    // 设置constructor
    constructor.Reset(isolate, tpl->GetFunction());
    // export `UniquePosition`
    exports->Set(String::NewFromUtf8(isolate, "UniquePosition"), tpl->GetFunction());
}

void UniquePosition::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.IsConstructCall()) {
    // Invoked as constructor: `new UniquePosition(...)`
    UniquePosition* obj;
    if(args.Length() == 0){
        obj = new UniquePosition();
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
        return;
    }
    Local<String> str = Local<String>::Cast(args[0]);
    String::Utf8Value utfValue(str);
    if(args.Length() == 1){
        obj = new UniquePosition(std::string(*utfValue));
    }else{
        if(args[1]->IsBoolean()){
            Local<Boolean> isbase64 = Local<Boolean>::Cast(args[1]);
            obj = new UniquePosition(std::string(*utfValue), isbase64->BooleanValue());
        }else{
            Local<String> str2 = Local<String>::Cast(args[1]);
            String::Utf8Value utfValue2(str2);
            obj = new UniquePosition(std::string(*utfValue), std::string(*utfValue2));
        }
    }
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    // Invoked as plain function `UniquePosition(...)`, turn into construct call.

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> result;

    if(args.Length() >= 2){
        Local<Value> argv[2] = { args[0], args[1] };
        result = cons->NewInstance(context, 2, argv).ToLocalChecked();
    }else if(args.Length() == 1){
        Local<Value> argv[1] = { args[0]};
        result = cons->NewInstance(context, 1, argv).ToLocalChecked();
    }else {
        result = cons->NewInstance(context, 0, NULL).ToLocalChecked();
    }
    args.GetReturnValue().Set(result);
  }
}

std::string parseCString(const FunctionCallbackInfo<Value>& args, int index) {
    if(args.Length() == 0){
        return "";
    }
    Local<String> str = Local<String>::Cast(args[index]);
    String::Utf8Value utfValue(str);
    return std::string(*utfValue);
}

// static.
bool UniquePosition::IsValidSuffix(const std::string& suffix) {
    return suffix.length() == kSuffixLength;
}

// const std::string& suffix
void UniquePosition::IsValidSuffix_(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if(args.Length() < 1) {
        args.GetReturnValue().Set(false);
        return;
    }
    char* buffer = (char*) node::Buffer::Data(args[0]->ToObject());
    bool val = IsValidSuffix(buffer);
    args.GetReturnValue().Set(val);
}

// static.
bool UniquePosition::IsValidBytes(const std::string& bytes) {
  return bytes.length() >= kSuffixLength
      && bytes[bytes.length()-1] != 0;
}

// const std::string& bytes
void UniquePosition::IsValidBytes_(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if(args.Length() < 1) {
        args.GetReturnValue().Set(false);
        return;
    }
    char* buffer = (char*) node::Buffer::Data(args[0]->ToObject());
    bool val = IsValidBytes(buffer);
    args.GetReturnValue().Set(val);
}

// static.
//UniquePosition UniquePosition::CreateInvalid() {
//  UniquePosition pos;
//  DCHECK(!pos.IsValid());
//  return pos;
//}

//
void UniquePosition::CreateInvalid(const v8::FunctionCallbackInfo<v8::Value>& args) {
//    UniquePosition* obj = new UniquePosition();
//    obj->Wrap(args.This());
//    args.GetReturnValue().Set(args.This());

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    // 使用constructor构建Function
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> lp = cons->NewInstance(context, 0, NULL).ToLocalChecked();
    args.GetReturnValue().Set(lp);
}

// static. int64 x, const std::string &suffix
void UniquePosition::FromInt64(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    if(args.Length() != 2){
        Local<Value> null = Null(isolate);
        args.GetReturnValue().Set(null);
        return;
    }
    Local<Number> number = Local<Number>::Cast(args[0]);
    int64 x = number->NumberValue();
    uint64 y = static_cast<uint64>(x);
    y ^= 0x8000000000000000ULL; // Make it non-negative.
    std::string bytes(8, 0);
    for (int i = 7; i >= 0; --i) {
        bytes[i] = static_cast<uint8>(y);
        y >>= 8;
    }

    std::string suffix = parseCString(args, 1);

//    UniquePosition* obj = new UniquePosition(bytes + suffix, suffix);
//    obj->Wrap(args.This());
//    args.GetReturnValue().Set(args.This());

    Local<Value> l1 = String::NewFromUtf8(isolate, (bytes + suffix).c_str());
    Local<Value> l2 = String::NewFromUtf8(isolate, suffix.c_str());
    Local<Value> argv[2] = { l1, l2 };
    // 使用constructor构建Function
    Local<Context> context = isolate->GetCurrentContext();

    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> lp = cons->NewInstance(context, 2, argv).ToLocalChecked();
    args.GetReturnValue().Set(lp);

    // return UniquePosition(bytes + suffix, suffix);
}

// static. const std::string& suffix
void UniquePosition::InitialPosition(const v8::FunctionCallbackInfo<v8::Value>& args) {

    std::string suffix = parseCString(args, 0);
    DCHECK(IsValidSuffix(suffix));

//    UniquePosition* obj = new UniquePosition(suffix);
//    obj->Wrap(args.This());
//    args.GetReturnValue().Set(args.This());

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    Local<Value> lv = String::NewFromUtf8(isolate, suffix.c_str());
    Local<Value> argv[2] = { lv, lv };
    // 使用constructor构建Function
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> lp = cons->NewInstance(context, 2, argv).ToLocalChecked();
    args.GetReturnValue().Set(lp);
}

// static. const UniquePosition& x, const std::string& suffix
void UniquePosition::Before(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Local<Object> localX = args[0]->ToObject();
    UniquePosition* x = ObjectWrap::Unwrap<UniquePosition>(localX);

    std::string suffix = parseCString(args, 1);

    DCHECK(IsValidSuffix(suffix));
    DCHECK(x->IsValid());

    const std::string& before = FindSmallerWithSuffix(Uncompress(x->compressed_), suffix);

//    UniquePosition* obj = new UniquePosition(before + suffix, suffix);
//    obj->Wrap(args.This());
//    args.GetReturnValue().Set(args.This());

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Value> lv1 = String::NewFromUtf8(isolate, (before + suffix).c_str());
    Local<Value> lv2 = String::NewFromUtf8(isolate, suffix.c_str());
    Local<Value> argv[2] = { lv1, lv2 };
    // 使用constructor构建Function
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> lp = cons->NewInstance(context, 2, argv).ToLocalChecked();
    args.GetReturnValue().Set(lp);

    //  return UniquePosition(before + suffix, suffix);
}

// static. const UniquePosition& x, const std::string& suffix
void UniquePosition::After(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Local<Object> localX = args[0]->ToObject();
    UniquePosition* x = ObjectWrap::Unwrap<UniquePosition>(localX);

    std::string suffix = parseCString(args, 1);

    DCHECK(IsValidSuffix(suffix));
    DCHECK(x->IsValid());
    const std::string& after = FindGreaterWithSuffix(Uncompress(x->compressed_), suffix);

//    UniquePosition* obj = new UniquePosition(after + suffix, suffix);
//    obj->Wrap(args.This());
//    args.GetReturnValue().Set(args.This());

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Value> lv1 = String::NewFromUtf8(isolate, (after + suffix).c_str());
    Local<Value> lv2 = String::NewFromUtf8(isolate, suffix.c_str());
    Local<Value> argv[2] = { lv1, lv2 };
    // 使用constructor构建Function
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> lp = cons->NewInstance(context, 2, argv).ToLocalChecked();
    args.GetReturnValue().Set(lp);

    // return UniquePosition(after + suffix, suffix);
}

// static.
//    const UniquePosition& before,
//    const UniquePosition& after,
//    const std::string& suffix
void UniquePosition::Between(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Local<Object> localBefore = args[0]->ToObject();
    UniquePosition* before = ObjectWrap::Unwrap<UniquePosition>(localBefore);
    Local<Object> localAfter = args[1]->ToObject();
    UniquePosition* after = ObjectWrap::Unwrap<UniquePosition>(localAfter);
    std::string suffix = parseCString(args, 2);

    DCHECK(before->IsValid());
    DCHECK(after->IsValid());
    DCHECK(before->LessThan(*after));
    DCHECK(IsValidSuffix(suffix));

    const std::string& mid = FindBetweenWithSuffix(
        Uncompress(before->compressed_),
        Uncompress(after->compressed_),
        suffix);

//    UniquePosition* obj = new UniquePosition(mid + suffix, suffix);
//    obj->Wrap(args.This());
//    args.GetReturnValue().Set(args.This());

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Value> lv1 = String::NewFromUtf8(isolate, (mid + suffix).c_str());
    Local<Value> lv2 = String::NewFromUtf8(isolate, suffix.c_str());
    Local<Value> argv[2] = { lv1, lv2 };
    // 使用constructor构建Function
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> lp = cons->NewInstance(context, 2, argv).ToLocalChecked();
    args.GetReturnValue().Set(lp);

    // return UniquePosition(mid + suffix, suffix);
}

bool UniquePosition::LessThan(const UniquePosition& other) const {
  DCHECK(this->IsValid());
  DCHECK(other.IsValid());

  return compressed_ < other.compressed_;
}

// const UniquePosition& other
void UniquePosition::LessThan_(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Local<Object> localObj = args[0]->ToObject();
    UniquePosition* other = ObjectWrap::Unwrap<UniquePosition>(localObj);

    UniquePosition* p = ObjectWrap::Unwrap<UniquePosition>(args.Holder());
    args.GetReturnValue().Set(p->LessThan(*other));
}

// static
bool UniquePosition::Equals(const UniquePosition& other) const {
  if (!this->IsValid() && !other.IsValid())
    return true;

  return compressed_ == other.compressed_;
}

// const UniquePosition& other
void UniquePosition::Equals_(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if(args.Length() == 0){
        args.GetReturnValue().Set(false);
    }
    Local<Object> localObj = args[0]->ToObject();
    UniquePosition* other = ObjectWrap::Unwrap<UniquePosition>(localObj);
    UniquePosition* p = ObjectWrap::Unwrap<UniquePosition>(args.Holder());
    args.GetReturnValue().Set(p->Equals(*other));
}

// static
std::string UniquePosition::getCompressValue(const bool bIsBase64) const
{
    if(bIsBase64){
        return std::string(base64_encode(compressed_.c_str(), compressed_.length()));
    }
    return compressed_;
}

// const bool bIsBase64
void UniquePosition::getCompressValue_(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    UniquePosition* p = ObjectWrap::Unwrap<UniquePosition>(args.Holder());
    Local<Boolean> lb = args[0].As<Boolean>();
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, p->getCompressValue(lb->BooleanValue()).c_str()));
}

int64 UniquePosition::ToInt64() const {
  uint64 y = 0;
  const std::string& s = Uncompress(compressed_);
  size_t l = sizeof(int64);
  if (s.length() < l) {
    l = s.length();
  }
  for (size_t i = 0; i < l; ++i) {
    const uint8 byte = s[l - i - 1];
    y |= static_cast<uint64>(byte) << (i * 8);
  }
  y ^= 0x8000000000000000ULL;
  // This is technically implementation-defined if y > INT64_MAX, so
  // we're assuming that we're on a twos-complement machine.
  return static_cast<int64>(y);
}

//
void UniquePosition::ToInt64_(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    UniquePosition* p = ObjectWrap::Unwrap<UniquePosition>(args.Holder());
    args.GetReturnValue().Set(Number::New(isolate, p->ToInt64()));
}

bool UniquePosition::IsValid() const {
  return is_valid_;
}

void UniquePosition::IsValid_(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    UniquePosition* p = ObjectWrap::Unwrap<UniquePosition>(args.Holder());
    args.GetReturnValue().Set(p->IsValid());
}

// static
std::string UniquePosition::ToDebugString() const {
  const std::string bytes = Uncompress(compressed_);
  if (bytes.empty())
    return std::string("INVALID[]");

  std::string debug_string = HexEncode(bytes.data(), bytes.length());
  if (!IsValid()) {
    debug_string = "INVALID[" + debug_string + "]";
  }

  std::string compressed_string =
      HexEncode(compressed_.data(), compressed_.length());
  debug_string.append(", compressed: " + compressed_string);
  return debug_string;
}

// ...
void UniquePosition::ToDebugString_(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    UniquePosition* p = ObjectWrap::Unwrap<UniquePosition>(args.Holder());
    std::string debugString = p->ToDebugString();
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, debugString.c_str()));
}

// static
std::string UniquePosition::GetSuffixForTest() const{
  const std::string bytes = Uncompress(compressed_);
  const size_t prefix_len = bytes.length() - kSuffixLength;
  return bytes.substr(prefix_len, std::string::npos);
}

// ...
void UniquePosition::GetSuffixForTest_(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    UniquePosition* p = ObjectWrap::Unwrap<UniquePosition>(args.Holder());
    std::string suffix4Test = p->GetSuffixForTest();
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, suffix4Test.c_str()));
}

//
std::string UniquePosition::FindSmallerWithSuffix(
    const std::string& reference,
    const std::string& suffix) {
  size_t ref_zeroes = reference.find_first_not_of('\0');
  size_t suffix_zeroes = suffix.find_first_not_of('\0');

  // Neither of our inputs are allowed to have trailing zeroes, so the following
  // must be true.
  DCHECK_NE(ref_zeroes, std::string::npos);
  DCHECK_NE(suffix_zeroes, std::string::npos);

  if (suffix_zeroes > ref_zeroes) {
    // Implies suffix < ref.
    return std::string();
  }

  if (suffix.substr(suffix_zeroes) < reference.substr(ref_zeroes)) {
    // Prepend zeroes so the result has as many zero digits as |reference|.
    return std::string(ref_zeroes - suffix_zeroes, '\0');
  } else if (suffix_zeroes > 1) {
    // Prepend zeroes so the result has one more zero digit than |reference|.
    // We could also take the "else" branch below, but taking this branch will
    // give us a smaller result.
    return std::string(ref_zeroes - suffix_zeroes + 1, '\0');
  } else {
    // Prepend zeroes to match those in the |reference|, then something smaller
    // than the first non-zero digit in |reference|.
    char lt_digit = static_cast<uint8>(reference[ref_zeroes])/2;
    return std::string(ref_zeroes, '\0') + lt_digit;
  }
}

// static
std::string UniquePosition::FindGreaterWithSuffix(
    const std::string& reference,
    const std::string& suffix) {
  size_t ref_FFs = reference.find_first_not_of(kuint8max);
  size_t suffix_FFs = suffix.find_first_not_of(kuint8max);

  if (ref_FFs == std::string::npos) {
    ref_FFs = reference.length();
  }
  if (suffix_FFs == std::string::npos) {
    suffix_FFs = suffix.length();
  }

  if (suffix_FFs > ref_FFs) {
    // Implies suffix > reference.
    return std::string();
  }

  if (suffix.substr(suffix_FFs) > reference.substr(ref_FFs)) {
    // Prepend FF digits to match those in |reference|.
    return std::string(ref_FFs - suffix_FFs, kuint8max);
  } else if (suffix_FFs > 1) {
    // Prepend enough leading FF digits so result has one more of them than
    // |reference| does.  We could also take the "else" branch below, but this
    // gives us a smaller result.
    return std::string(ref_FFs - suffix_FFs + 1, kuint8max);
  } else {
    // Prepend FF digits to match those in |reference|, then something larger
    // than the first non-FF digit in |reference|.
    char gt_digit = static_cast<uint8>(reference[ref_FFs]) +
        (kuint8max - static_cast<uint8>(reference[ref_FFs]) + 1) / 2;
    return std::string(ref_FFs, kuint8max) + gt_digit;
  }
}

// static
std::string UniquePosition::FindBetweenWithSuffix(
    const std::string& before,
    const std::string& after,
    const std::string& suffix) {
  DCHECK(IsValidSuffix(suffix));
  DCHECK_NE(before, after);
  DCHECK_LT(before, after);

  std::string mid;

  // Sometimes our suffix puts us where we want to be.
  if (before < suffix && suffix < after) {
    return std::string();
  }

  size_t i = 0;
  for ( ; i < std::min(before.length(), after.length()); ++i) {
    uint8 a_digit = before[i];
    uint8 b_digit = after[i];

    if (b_digit - a_digit >= 2) {
      mid.push_back(a_digit + (b_digit - a_digit)/2);
      return mid;
    } else if (a_digit == b_digit) {
      mid.push_back(a_digit);

      // Both strings are equal so far.  Will appending the suffix at this point
      // give us the comparison we're looking for?
      if (before.substr(i+1) < suffix && suffix < after.substr(i+1)) {
        return mid;
      }
    } else {
      DCHECK_EQ(b_digit - a_digit, 1);  // Implied by above if branches.

      // The two options are off by one digit.  The choice of whether to round
      // up or down here will have consequences on what we do with the remaining
      // digits.  Exploring both options is an optimization and is not required
      // for the correctness of this algorithm.

      // Option A: Round down the current digit.  This makes our |mid| <
      // |after|, no matter what we append afterwards.  We then focus on
      // appending digits until |mid| > |before|.
      std::string mid_a = mid;
      mid_a.push_back(a_digit);
      mid_a.append(FindGreaterWithSuffix(before.substr(i+1), suffix));

      // Option B: Round up the current digit.  This makes our |mid| > |before|,
      // no matter what we append afterwards.  We then focus on appending digits
      // until |mid| < |after|.  Note that this option may not be viable if the
      // current digit is the last one in |after|, so we skip the option in that
      // case.
      if (after.length() > i+1) {
        std::string mid_b = mid;
        mid_b.push_back(b_digit);
        mid_b.append(FindSmallerWithSuffix(after.substr(i+1), suffix));

        // Does this give us a shorter position value?  If so, use it.
        if (mid_b.length() < mid_a.length()) {
          return mid_b;
        }
      }
      return mid_a;
    }
  }

//   If we haven't found a midpoint yet, the following must be true:
  DCHECK_EQ(before.substr(0, i), after.substr(0, i));
  DCHECK_EQ(before, mid);
  DCHECK_LT(before.length(), after.length());

  mid.append(FindSmallerWithSuffix(after.substr(i), suffix));
  return mid;
}

//namespace {

// Appends an encoded run length to |output_str|.
static void WriteEncodedRunLength(uint32 length,
                                  bool high_encoding,
                                  std::string* output_str) {
  CHECK_GE(length, 4U);
  CHECK_LT(length, 0x80000000);

  // Step 1: Invert the count, if necessary, to account for the following digit.
  uint32 encoded_length;
  if (high_encoding) {
    encoded_length = 0xffffffff - length;
  } else {
    encoded_length = length;
  }

  // Step 2: Write it as big-endian so it compares correctly with memcmp(3).
  output_str->append(1, 0xff & (encoded_length >> 24U));
  output_str->append(1, 0xff & (encoded_length >> 16U));
  output_str->append(1, 0xff & (encoded_length >> 8U));
  output_str->append(1, 0xff & (encoded_length >> 0U));
}

// Reads an encoded run length for |str| at position |i|.
static uint32 ReadEncodedRunLength(const std::string& str, size_t i) {
  DCHECK_LE(i + 4, str.length());

  // Step 1: Extract the big-endian count.
  uint32 encoded_length =
      ((uint8)(str[i+3]) << 0)  |
      ((uint8)(str[i+2]) << 8)  |
      ((uint8)(str[i+1]) << 16) |
      ((uint8)(str[i+0]) << 24);

  // Step 2: If this was an inverted count, un-invert it.
  uint32 length;
  if (encoded_length & 0x80000000) {
    length = 0xffffffff - encoded_length;
  } else {
    length = encoded_length;
  }

  return length;
}
//
// A series of four identical chars at the beginning of a block indicates
// the beginning of a repeated character block.
static bool IsRepeatedCharPrefix(const std::string& chars, size_t start_index) {
  return chars[start_index] == chars[start_index+1]
      && chars[start_index] == chars[start_index+2]
      && chars[start_index] == chars[start_index+3];
}

//}  // namespace

// static
// Wraps the CompressImpl function with a bunch of DCHECKs.
std::string UniquePosition::Compress(const std::string& str) {
  DCHECK(IsValidBytes(str));
  std::string compressed = CompressImpl(str);
  DCHECK(IsValidCompressed(compressed));
  DCHECK_EQ(str, Uncompress(compressed));
  return compressed;
}

// static
// Performs the order preserving run length compression of a given input string.
std::string UniquePosition::CompressImpl(const std::string& str) {
  std::string output;

  output.reserve(48);

  // Each loop iteration will consume 8, or N bytes, where N >= 4 and is the
  // length of a string of identical digits starting at i.
  for (size_t i = 0; i < str.length(); ) {
    if (i + 4 <= str.length() && IsRepeatedCharPrefix(str, i)) {
      // Four identical bytes in a row at this position means that we must start
      // a repeated character block.  Begin by outputting those four bytes.
      output.append(str, i, 4);

      // Determine the size of the run.
      const char rep_digit = str[i];
      const size_t runs_until = str.find_first_not_of(rep_digit, i+4);

      // Handle the 'runs until end' special case specially.
      size_t run_length;
      bool encode_high;  // True if the next byte is greater than |rep_digit|.
      if (runs_until == std::string::npos) {
        run_length = str.length() - i;
        encode_high = false;
      } else {
        run_length = runs_until - i;
        encode_high = static_cast<uint8>(str[runs_until]) >
            static_cast<uint8>(rep_digit);
      }

      WriteEncodedRunLength(run_length, encode_high, &output);
      i += run_length;  // Jump forward by the size of the run length.
    } else {
      // Output up to eight bytes without any encoding.
      const size_t len = std::min(static_cast<size_t>(8), str.length() - i);
      output.append(str, i, len);
      i += len;  // Jump forward by the amount of input consumed (usually 8).
    }
  }

  return output;
}

// static
// Uncompresses strings that were compresed with UniquePosition::Compress.
std::string UniquePosition::Uncompress(const std::string& str) {
  std::string output;
  size_t i = 0;
  // Iterate through the compressed string one block at a time.
  for (i = 0; i + 8 <= str.length(); i += 8) {
    if (IsRepeatedCharPrefix(str, i)) {
      // Found a repeated character block.  Expand it.
      const char rep_digit = str[i];
      uint32 length = ReadEncodedRunLength(str, i+4);
      output.append(length, rep_digit);
    } else {
      // Found a regular block.  Copy it.
      output.append(str, i, 8);
    }
  }
  // Copy the remaining bytes that were too small to form a block.
  output.append(str, i, std::string::npos);
  return output;
}
//
bool UniquePosition::IsValidCompressed(const std::string& str) {
  for (size_t i = 0; i + 8 <= str.length(); i += 8) {
    if (IsRepeatedCharPrefix(str, i)) {
      uint32 count = ReadEncodedRunLength(str, i+4);
      if (count < 4) {
        // A repeated character block should at least represent the four
        // characters that started it.
        return false;
      }
      if (str[i] == str[i+4]) {
        // Does the next digit after a count match the repeated character?  Then
        // this is not the highest possible count.
        return false;
      }
    }
  }
  // We don't bother looking for the existence or checking the validity of
  // any partial blocks.  There's no way they could be invalid anyway.
  return true;
}

NODE_MODULE(UniquePosition, UniquePosition::Init);

