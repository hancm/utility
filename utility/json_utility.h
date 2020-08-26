#ifndef __JSON_UTILITY_H__
#define __JSON_UTILITY_H__

#include "./json/ordered_map.h"
#include "./json/json.hpp"

namespace util
{

using json = nlohmann::json;

template<class Key, class T, class Ignore, class Allocator,
         class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>,
         class AllocatorPair = typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, T>>,
         class ValueTypeContainer = std::vector<std::pair<Key, T>, AllocatorPair>>
using ordered_map = tsl::ordered_map<Key, T, Hash, KeyEqual, AllocatorPair, ValueTypeContainer>;
using ordered_json = nlohmann::basic_json<ordered_map>;

} /* namespace util */
#endif /* __JSON_UTILITY_H__ */