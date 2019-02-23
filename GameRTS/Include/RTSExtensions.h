#pragma once

#include <gePrerequisitesUtil.h>

template<typename T, typename UnaryPredicate>
FORCEINLINE void
static PriorityPushBack(List<T>& list, T& val, UnaryPredicate predicate)
{
  for (auto it = list.begin(); it != list.end(); ++it) {
    if (predicate) {
      list.insert(it, val);
      return;
    }
  }

  list.push_back(val);
}