#include "property_simplified.h"
#include <gtest/gtest.h>
#include <iostream>

class Foo {
 public:
  int data_ = 40;

  Foo() : data([this]() { std::cout << "poop\n"; return this->data_*2; },
               [this](int d) { this->data_ = d; std::cout << "d= " << d << std::endl; }
               ) {}

  
  property<int> data;
};


TEST(PropertyTest, READ_WRITE) {
  auto t = Foo();
  ASSERT_EQ(t.data_, 40);
  ASSERT_EQ(t.data, 80);
  t.data = 50;
  ASSERT_EQ(t.data, 100);
  std::cout << t.data << std::endl;
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
