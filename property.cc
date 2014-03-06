// #include "property.h"
#include <functional>
#include <gtest/gtest.h>
#include <iostream>

// template<class Container, class Value>
// class property {
//  public:
//   using const_getter_type = std::function<Value(const Container*)>;
//   using mutable_getter_type = std::function<Value&(const Container*)>;

//   using movable_setter_type = std::function<void(Container*, Value&&)>;

//   using getter_type = const_getter;
//   using setter_type = std::function<void(Container*, const Value&)>;

//   property(Container* c, setter_type setter) : container(c), set(setter) {}
//   property(Container* c, getter_type getter) : container(c), get(getter) {}
//   property(Container* c, getter_type getter, setter_type setter) : container(c), set(setter), get(getter) {}
//   property(Container* c, setter_type setter, getter_type getter) : container(c), set(setter), get(getter) {}

//   property& operator=(const Value& value) {
//     set(container, value);
//   }
  

// };


template<class Container, class Value>
class property {
 public:
  // using const_getter_type = std::function<Value(const Container*)>;
  // using mutable_getter_type = std::function<Value&(const Container*)>;
  using const_getter_type = std::function<Value()>;
  // using mutable_getter_type = std::function<Value&()>;

  // using movable_setter_type = std::function<void(Container*, Value&&)>;

  using getter_type = const_getter_type;
  // using setter_type = std::function<void(Container*, const Value&)>;
  using setter_type = std::function<void(const Value&)>;

  // property(Container* c, setter_type setter) : container(c), set(setter) {}
  // property(Container* c, getter_type getter) : container(c), get(getter) {}
  property(Container* c, getter_type getter, setter_type setter) : container(c), set(setter), get(getter) {}
  property(Container* c, setter_type setter, getter_type getter) : container(c), set(setter), get(getter) {}
  // property(Container* c, setter_type setter, getter_type getter, mutable_getter_type mutable_getter) : container(c), set(setter), get(getter), m_get(mutable_getter) {}

  // operator Value&() {
  //   return m_get(container);
  // }

  operator Value() {
    return get();
  }

  operator Value() const {
    return get();
  }

  property& operator=(const Value& value) {
    // set(container, value);
    set(value);
    return *this;
  }
  
 private:
  Container* container;
  setter_type set;
  getter_type get;
  // mutable_getter_type m_get;
};

template<class T>
std::function<T&(void)> identity_function(T& value) {
  return [&value]() { return value; };
}

template<class T>
std::function<const T&(void)> identity_function(const T& value) {
  return [&value]() { return value; };
}

template<class Container, class T>
property<Container, T> make_property(Container* c,
                                     typename property<Container, T>::getter_type getter,
                                     typename property<Container, T>::setter_type setter) {
  return property<Container, T>(c, setter, getter);
}

template<class Container, class T>
property<Container, T> make_property(Container* c,
                                     typename property<Container, T>::setter_type setter,
                                     typename property<Container, T>::getter_type getter) {
  return property<Container, T>(c, setter, getter);
}

// template<class Container, class T>
// property<Container, T> make_property(Container* c,
//                                      typename property<Container, T>::setter_type setter,
//                                      typename property<Container, T>::getter_type getter,
//                                      typename property<Container, T>::mutable_getter_type mutable_getter) {
//   return property<Container, T>(c, setter, getter, mutable_getter);
// }


// template<class Container, class T>
// property<Container, T> make_property(Container* container, property<Container, T>::getter_type get, property<Container, T>::setter_type) {
//   return property<Container, T>(container, 
// }

class Foo {
 public:
  Foo() : data([this]() { std::cout << "poop\n"; return this->data_*2; },
               [this](int d) { this->data_ = d; std::cout << "d= " << d << std::endl; }
               ) {}

  int data_ = 40;
  
  // property<Foo, int> data = make_property<int>(this,
  property<Foo, int> data;
};

TEST(PropertyTest, READ_WRITE) {
  auto t = Foo();
  ASSERT_EQ(t.data_, 40);
  ASSERT_EQ(t.data, 80);
  t.data = 50;
  ASSERT_EQ(t.data, 100);
  std::cout << t.data << std::endl;
}

// TEST(PropertyTest, ORDER) {
//   auto t = property<Foo, 
// }

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

