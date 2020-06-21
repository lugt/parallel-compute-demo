//
// Created by xc5 on 6/21/20.
//

#include "codetimer.h"

MyTTimer::MyTTimer() : m_start{std::chrono::system_clock::now()}, m_end{std::chrono::system_clock::now()}{}

void MyTTimer::start() {
  m_start = std::chrono::system_clock::now();
}

void MyTTimer::stop() {
  m_end = std::chrono::system_clock::now();
}

std::chrono::duration<double> MyTTimer::duration() const {
  return m_end - m_start;
}