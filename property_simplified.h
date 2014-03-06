#ifndef PROPERTY_SIMPLIFIED_H_
#define PROPERTY_SIMPLIFIED_H_

#include <functional>

template<class Value>
class property {
 public:
  using getter_type = std::function<Value()>;
  using setter_type = std::function<void(const Value&)>;

  property(getter_type getter, setter_type setter) : set(setter), get(getter) {}
  property(setter_type setter, getter_type getter) : set(setter), get(getter) {}

  operator Value() {
    return get();
  }

  operator Value() const {
    return get();
  }

  property& operator=(const Value& value) {
    set(value);
    return *this;
  }
  
 private:
  setter_type set;
  getter_type get;
};

#endif  // PROPERTY_SIMPLIFIED_H_
