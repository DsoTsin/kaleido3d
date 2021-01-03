/**
 * MIT License
 *
 * Copyright (c) 2021 Zhou Qin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once
#include "Types.h"

namespace k3d {
class Handle {
public:
  static Handle New();
  static void Free(const Handle &InHandle);
  static Handle InvalidValue;

  Handle() : m_Value(~0ULL) {}

  Handle &operator=(const Handle &InHandle) {
    m_Value = InHandle.m_Value;
    return *this;
  }

  bool operator==(const Handle &InHandle) const {
    return m_Value == InHandle.m_Value;
  }

  bool operator!=(const Handle &InHandle) const {
    return m_Value != InHandle.m_Value;
  }

  bool IsValid() const;
  bool IsFreed() const;

private:
  Handle(const U64 &InValue) : m_Value(InValue) {}

  U64 m_Value;
};
} // namespace k3d