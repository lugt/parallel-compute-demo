//
// Created by xc5 on 6/21/20.
//

#include "codetimer.h"

CodeTimer::CodeTimer() : m_start{std::chrono::system_clock::now()}, m_end{std::chrono::system_clock::now()}{}

void CodeTimer::start() {
  m_start = std::chrono::system_clock::now();
}

void CodeTimer::stop() {
  m_end = std::chrono::system_clock::now();
}

std::chrono::duration<double> CodeTimer::duration() const {
  return m_end - m_start;
}