# Facade外观模式

- 意图

  为子系统中的一组接口（Shape）提供一个一致的界面，Facade模式定义了一个高层接口，这个接口使得这一子系统可以更加容易使用。

- 优点

  - 对客户屏蔽子系统组件，有利于降低系统的复杂性。
  - 实现了子系统与客户程序之间的松耦合关系，而子系统内部的功能组件是紧耦合的。
  - 使用Facade的客户程序不需要直接访问子系统对象，可以消除复杂的循环依赖关系，这一点在客户程序和子系统分别实现的时候尤为重要。
  - 如果应用需要，它并不限制它们使用子系统类。因此你可以在系统易用性和通用性之间加以选择。

- UML图

  ![Facade模式](../img/Facade.png)

- 代码示例

  ```c++
  class Shape {
  public:
      virtual ~Shape() { }
      CLASS_PTR(Shape);
  
      virtual void draw() = 0;
  };
  
  class Circle : public Shape {
  public:
      ~Circle() { }
  
      void draw() override {
          std::cout << "circle draw." << std::endl;
      }
  };
  
  class Square : public Shape {
  public:
      ~Square() { }
      void draw() override {
          std::cout << "square draw." << std::endl;
      }
  };
  
  class Rectangle : public Shape
  {
  public:
      ~Rectangle() { }
  
      void draw() override {
          std::cout << "rectangle draw." << std::endl;
      }
  };
  
  class ShapeFacade {
  public:
  	ShapeFacade() {
  		circle_ = std::make_shared<Circle>();
  		square_ = std::make_shared<Square>();
  		rectangle_ = std::make_shared<Rectangle>();
  	}
  
  	~ShapeFacade() {
  	}
  
  	void drawCircle() {
  		circle_->draw();
  	}
  
  	void drawSquare() {
  		square_->draw();
  	}
  
  	void drawRectangle() {
  		rectangle_->draw();
  	}
  
  private:
      std::shared_ptr<Shape> circle_;
      std::shared_ptr<Shape> square_;
      std::shared_ptr<Shape> rectangle_;
  };
  
  void test() {
  	std::cout << "\n\n facade pattern." << std::endl;
  
  	auto sm = std::make_shared<ShapeFacade>();
  	sm->drawCircle();
  	sm->drawSquare();
  	sm->drawRectangle();
  }
  
  ```

  