#ifndef PROF_HPP_JOF2FZL8
#define PROF_HPP_JOF2FZL8

#include <cstring>
#include <string>
#include <type_traits>
#include <typeinfo>

#include <fmt/format.h>

#include "palanteer.h"

#define _GET_10TH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N

#define _FE_0(_call, ...)
#define _FE_1(_call, x) _call(x)
#define _FE_2(_call, x, ...) _call(x) _FE_1(_call, __VA_ARGS__)
#define _FE_3(_call, x, ...) _call(x) _FE_2(_call, __VA_ARGS__)
#define _FE_4(_call, x, ...) _call(x) _FE_3(_call, __VA_ARGS__)
#define _FE_5(_call, x, ...) _call(x) _FE_4(_call, __VA_ARGS__)
#define _FE_6(_call, x, ...) _call(x) _FE_5(_call, __VA_ARGS__)
#define _FE_7(_call, x, ...) _call(x) _FE_6(_call, __VA_ARGS__)
#define _FE_8(_call, x, ...) _call(x) _FE_7(_call, __VA_ARGS__)
#define _FE_9(_call, x, ...) _call(x) _FE_8(_call, __VA_ARGS__)
#define _FE_10(_call, x, ...) _call(x) _FE_9(_call, __VA_ARGS__)
#define _FOR_EACH(x, ...)                                                      \
  _GET_10TH_ARG("", ##__VA_ARGS__, _FE_8, _FE_7, _FE_6, _FE_5, _FE_4, _FE_3,   \
                _FE_2, _FE_1, _FE_0)                                           \
  (x, ##__VA_ARGS__)

#if defined(USE_PL) && USE_PL == 1
#define PVAR(key, val) tpm::prof::fmt(key, val);
#else
#define PVAR(key, val)
#endif
#define PARG(arg) PVAR(#arg, arg)
#define PARGS(...) _FOR_EACH(PARG, ##__VA_ARGS__)
#define PFUNC(...)                                                             \
  plFunction();                                                                \
  PARGS(__VA_ARGS__);
#define PBEGIN(...) plBegin(__VA_ARGS__);
#define PEND(...) plEnd(__VA_ARGS__);
#define PSCOPE(label, ...)                                                     \
  plScope(label);                                                              \
  PARGS(__VA_ARGS__)
#define PINIT(proc, ...)                                                       \
  plInitAndStart(proc, ##__VA_ARGS__);                                         \
  plDeclareThread("Main");                                                     \
  plFunction();                                                                \
  PVAR("argc", argc);                                                          \
  PVAR("argv", std::vector<std::string>(argv + 1, argv + argc));
#define PSTOP(...) plStopAndUninit(__VA_ARGS__);

namespace tpm::prof {

#if defined(USE_PL) && USE_PL == 1
template <typename T>
inline std::enable_if_t<std::is_fundamental<T>::value, void>
fmt(const char *key, T val) {
  if (PL_IS_ENABLED_())
    plPriv::eventLogData(PL_STRINGHASH(PL_BASEFILENAME),
                         plPriv::fnv1a_(key, PL_FNV_HASH_OFFSET_),
                         PL_EXTERNAL_STRINGS ? 0 : PL_BASEFILENAME,
                         PL_EXTERNAL_STRINGS ? 0 : key, __LINE__,
                         PL_STORE_COLLECT_CASE_, val);
}
template <typename T>
inline std::enable_if_t<std::is_pointer<T>::value, void> fmt(const char *key,
                                                             T val) {
  if (PL_IS_ENABLED_()) {
    size_t key_length = strlen(key);
    char *str = static_cast<char *>(malloc(sizeof(char) * (key_length + 6)));
    strncpy(str, key, key_length + 6);
    strcat(str, "##hexa");
    plPriv::eventLogData(PL_STRINGHASH(PL_BASEFILENAME),
                         plPriv::fnv1a_(str, PL_FNV_HASH_OFFSET_),
                         PL_EXTERNAL_STRINGS ? 0 : PL_BASEFILENAME,
                         PL_EXTERNAL_STRINGS ? 0 : str, __LINE__,
                         PL_STORE_COLLECT_CASE_,
                         reinterpret_cast<std::uintptr_t>(val));
  }
}
template <typename T>
inline std::enable_if_t<
    !std::is_pointer<T>::value && !std::is_fundamental<T>::value, void>
fmt(const char *key, T val) {
  if (PL_IS_ENABLED_())
    plPriv::eventLogData(PL_STRINGHASH(PL_BASEFILENAME),
                         plPriv::fnv1a_(key, PL_FNV_HASH_OFFSET_),
                         PL_EXTERNAL_STRINGS ? 0 : PL_BASEFILENAME,
                         PL_EXTERNAL_STRINGS ? 0 : key, __LINE__,
                         PL_STORE_COLLECT_CASE_,
                         fmt::format("{}", val).c_str());
}
#endif
} // namespace tpm::prof

#endif /* end of include guard: PROF_HPP_JOF2FZL8 */
