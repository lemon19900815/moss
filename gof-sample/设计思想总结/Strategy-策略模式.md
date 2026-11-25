### Strategy-策略模式

- 意图

  定义一系列的算法，把它们一个个封装起来，并且使它们可以相互替换。本模式使得算法可独立于使用它的客户而变化。

- 优缺点

  - 相关算法系列，Stategy类层次为Context定义一系列的可供重用的算法或行为。继承有助于析取出这些算法中的公共功能。

  - 一个替代继承的方法，继承提供了另一种支持多种算法或行为的方法。你可以直接生成一个Context类的子类，从而给它以不同的行为。但这会将行为硬行编码到Context中，而将算法的实现与Context的实现混合起来，从而使Context难以理解、难以维护和难以扩展，而且还不能动态地改变算法。最后你得到一堆相关的类，它们之间的唯一差别是它们所使用的算法或行为。将算法封装在独立的Stategy类中使得你可以独立于其Context改变它，使它易于切换、易于理解、易于扩展。

  - 消除一些条件语句，Strategy膳魔师提供了用条件语句选择所需的行为以外的另一种选择。当不同的行为堆砌在一个类中时，很难避免使用条件语句来选择合适的行为。将行为封装在一个个独立的Strategy类中消除这些条件语句。

    ```c++
    // 不使用Strategy
    void Context::Repair() {
        switch(breaking_strategy_) {
        case SimpleStrategy:
        	ComposeWithSimpleCompositor();
        	break;
        case TeXStrategy:
        	ComposeWithTeXCompositor();
        	break;
        // ...
        }
        // merge results with existing composition, if necessary
    }
    
    // 使用Strategy
    void Context::Repair() {
        compositor_->Compose();
        // merge results with existing composition, if necessary
    }
    ```

  - 客户必须了解不同的Strategy，本模式有一个潜在的**缺点**，就是一个客户要选择一个合适的Strategy就必须知道这些Strategy到底有何不同。此时可能不得不向客户暴露具体的实现问题。因此仅当这些不同行为变体与客户相关的行为时，才需要使用Strategy模式。

  - Strategy和Context之间的通信开销，无论各个ConcreteStrategy实现的算法是简单还是复杂，它们都共享Strategy定义的接口。因此很可能某些ConcreteStrategy不会都用到所有通过这个接口传递给它们的信息；简单的ConcreteStrategy可能不使用其中的任何信息！这就意味着有时Context会创建和初始化一些永远不会用到的参数。如果存在这样问题，那么将需要在Strategy和Context之间进行更紧密的耦合。

  - 增加了对象的数目，Strategy增加了一个应用中的对象的数目。有时你可以将Strategy实现为可供各Context共享的无状态的对象来减少这一开销。任何其余的状态都由Context维护。Context在每一次对Strategy对象的请求中都将这个状态传递过去。共享的Stratety不应在各次调用之间维护状态。Flyweight模式更详细地描述了这一方法。

- UML图

  ![Strategy模式](../img/Strategy.png)

- 代码示例

  ```c++
  class AbstractStategy {
  public:
    virtual ~AbstractStategy() { }
    CLASS_PTR(AbstractStategy);
  
    virtual int32_t Operate(int32_t num1, int32_t num2) = 0;
  };
  
  class OperationAdd : public AbstractStategy {
  public:
    int32_t Operate(int32_t num1, int32_t num2) override {
      return num1 + num2;
    }
  };
  
  class OperationSub : public AbstractStategy {
  public:
    int32_t Operate(int32_t num1, int32_t num2) override {
      return num1 - num2;
    }
  };
  
  class OperationMultiply : public AbstractStategy {
  public:
    int32_t Operate(int32_t num1, int32_t num2) override {
      return num1 * num2;
    }
  };
  
  class OperationDivide : public AbstractStategy {
  public:
    int32_t Operate(int32_t num1, int32_t num2) override {
      if (num2 == 0) {
        std::cerr << "num2 is zero." << std::endl;
        assert(false);
        return -1;
      }
      return num1 / num2;
    }
  };
  
  class Context {
  public:
    Context(AbstractStategy::Ptr strategy) {
      strategy_ = strategy;
    }
  
    virtual ~Context() {
    }
  
    int32_t Calculate(int32_t num1, int32_t num2) {
      return strategy_->Operate(num1, num2);
    }
  
  private:
    AbstractStategy::Ptr strategy_;
  };
  
  void test() {
    std::cout << "\n\n strategy pattern." << std::endl;
  
    Context add(std::make_shared<OperationAdd>());
    Context sub(std::make_shared<OperationSub>());
    Context multiply(std::make_shared<OperationMultiply>());
    Context divide(std::make_shared<OperationDivide>());
  
    std::cout << "3 + 5 = " << add.Calculate(3, 5) << std::endl;
    std::cout << "1 - 7 = " << sub.Calculate(1, 7) << std::endl;
    std::cout << "3 * 5 = " << multiply.Calculate(3, 5) << std::endl;
    std::cout << "8 / 2 = " << divide.Calculate(8, 2) << std::endl;
  }
  ```

  