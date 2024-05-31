#include <iostream>
#include <vector>
#include <memory>

class Area {
 public:
  template <typename T> 
    void  Add(T* shape)  { 
        shape_.emplace_back(new Wrapper(shape)); 
    }

    void Print() {
      for (auto &&elem : shape_) {
        std::cout << elem->GetArea() << "\n";
      }
    }
 private:
  class MyShape {
 public:
  virtual double GetArea() const = 0;
  virtual ~MyShape() {}
};

template<typename T>
class Wrapper : public MyShape {
 public:
  Wrapper(T *t) : t_(t) {}
  double GetArea() const {
    return t_->GetArea();
  }
 private:
  T *t_ = nullptr;
};

std::vector<MyShape*> shape_;
};

class MyShape {
 public:
  virtual double GetArea() const = 0;
  virtual ~MyShape() {}
};

class Square {
 public:
  Square(double side) : side_(side) {}

  double GetArea() const {
    return side_ * side_;
  }
 private:
  double side_;
};

class Rectangle {
 public:
  Rectangle(double width, double length) : w_(width), h_(length) {}

  double GetArea() const {
    return w_ * h_;
  }
 private:
  double w_;
  double h_;
};

class Circle  {
    
 public:
  Circle(double radius) : radius_(radius) {}
  
  double GetArea() const {
    return 3.14 * radius_ * radius_;
  }
 private:
  double radius_;
};

int main() {
  Square s{1.0};
  Rectangle r{1.0, 2.0};
  Circle c{3.0};

  
  Area area;
  area.Add(&s);
  area.Add(&r);
  area.Add(&c);

  area.Print();

  return 0;
}
