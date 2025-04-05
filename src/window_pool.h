#ifndef WINDOW_POOL_H_E46A8EBB
#define WINDOW_POOL_H_E46A8EBB

#include <stack>
#include <vector>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

template <typename T>
class WindowPool {
 public:
  explicit WindowPool(wxWindow* parent) : parent_(parent) {}

  template <typename... Args>
  T* Allocate(Args&&... args) {
    if (pool_.empty()) {
      pool_.push(new T(parent_, wxID_ANY, std::forward<Args>(args)...));
    }

    T* popped = pool_.top();
    pool_.pop();
    used_.push_back(popped);

    popped->Show();

    return popped;
  }

  void Reset() {
    for (T* window : used_) {
      window->Hide();
      window->GetContainingSizer()->Detach(window);
      pool_.push(window);
    }

    used_.clear();
  }

 private:
  wxWindow* parent_;
  std::stack<T*> pool_;
  std::vector<T*> used_;
};

#endif /* end of include guard: WINDOW_POOL_H_E46A8EBB */
