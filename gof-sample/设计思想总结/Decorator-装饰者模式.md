# Decorator装饰者模式

  - 意图

    动态地给一个对象添加一些额外的职责。就新增功能来说，Decorator模式相比生成子类更为灵活。

  - 优缺点

    - 比静态继承更灵活
    - 避免在层次结构高层的类有太多的特征
    - 产生许多小对象（缺点）

  - UML图

    ![Decorator模式](../img/Decorator.png)
    ![Decorator模式-Stream实例](../img/Decorator-Stream.png)

  - 代码示例

    ```c++
    class Shape {
    public:
        virtual~Shape() { }
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
    
    class Rectangle : public Shape {
    public:
        ~Rectangle() { }
    
        void draw() override {
            std::cout << "rectangle draw." << std::endl;
        }
    };
    
    class ShapeDecorator : public Shape {
    public:
        ShapeDecorator(Shape::Ptr& shape) {
            shape_ = shape;
        }
    
    protected:
        Shape::Ptr shape_;
    };
    
    class RedShapeDecorator : public ShapeDecorator
    {
    public:
        RedShapeDecorator(Shape::Ptr shape) : ShapeDecorator(shape) { }
    
        void draw() override {
            shape_->draw();
            std::cout << "red:" << std::endl;
        }
    };
    
    void test() {
        std::cout << "\n\n decorator pattern." << std::endl;
    
        auto sd1 = std::make_shared<RedShapeDecorator>(
            std::make_shared<Circle>());
        sd1->draw();
    
        auto sd2 = std::make_shared<RedShapeDecorator>(
            std::make_shared<Rectangle>());
        sd2->draw();
    
        auto sd3 = std::make_shared<RedShapeDecorator>(sd2);
        sd3->draw();
    }
    ```

