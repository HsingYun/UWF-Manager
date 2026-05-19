#pragma once

// 进程级"可投递"门闸。
//
// 后台 worker 线程把结果投回 GUI 线程（QMetaObject::invokeMethod(qApp, ...)）
// 之前，必须经此门闸——它与应用关停互斥：main 在 QApplication 析构之前调一次
// close() 永久关闭门闸；此后 runIfOpen 一律直接跳过，杜绝向已销毁的
// QApplication 投递（detached worker 可能比 QApplication 活得久）。
//
// runIfOpen 在持锁状态下检查门闸并执行回调——这把"门闸仍开"与"投递"绑成原子
// 操作：要么 close() 先拿到锁（worker 随后跳过），要么 worker 先拿到锁并完成
// 投递（此时 close() 尚未返回，QApplication 必然尚未开始析构）。
//
// 门闸状态进程级、永不析构——detached worker 可能在静态析构期仍调 runIfOpen。

#include <mutex>

namespace uwf::postgate {

namespace detail {
struct State {
  std::mutex m;
  bool open = true;
};
inline State& state() {
  // 故意泄漏、永不析构：worker 线程可能在静态析构之后仍调 runIfOpen。
  static State* const s = new State;
  return *s;
}
}  // namespace detail

// 应用退出时（QApplication 析构之前）调用一次，永久关闭门闸。
inline void close() {
  detail::State& s = detail::state();
  const std::lock_guard<std::mutex> lk(s.m);
  s.open = false;
}

// 持锁检查门闸：未关闭则执行 fn 并返回 true；已关闭直接返回 false、不执行 fn。
// fn 里通常做 QMetaObject::invokeMethod(qApp, ...)。
template <typename Fn>
bool runIfOpen(Fn&& fn) {
  detail::State& s = detail::state();
  const std::lock_guard<std::mutex> lk(s.m);
  if (!s.open) return false;
  fn();
  return true;
}

}  // namespace uwf::postgate
