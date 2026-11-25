# Command-命令模式

- 意图

  将一个请求封装为一个对象，从而使你可用不同的请求对客户进行参数化；对请求排队或者记录日志请求，以及支持可撤销的操作。

- 优缺点

  - Command模式将调用操作的对象与知道如何实现该操作的对象解耦。

- Command是头等的对象。它们可像其他的对象一样被操纵和扩展。

  - 你可以将多个命令装配成一个复合命令。

- 增加新的Command很容易，因此这无需改变已有的类。

- UML图

![Command模式](../img/Command.png)

![Command模式示例](../img/Command-Stock.png)

- 代码示例

  ```c++
  class Stock {
    public:
    Stock(std::string name, int amount) {
      name_ = name;
      amount_ = amount;
    }
    ~Stock() { }
    CLASS_PTR(Stock);
  
    void buy() {
      std::cout << "buy stock[name: " << name_
        << ", amount: " << amount_ << "]" << std::endl;
    }
  
    void sell() {
      std::cout << "sell stock[name: " << name_
        << ", amount: " << amount_ << "]" << std::endl;
    }
  
  private:
    std::string name_;
    int32_t amount_;
  };
  
  // command接口
  class Order {
  public:
    virtual ~Order() { }
    CLASS_PTR(Order);
    virtual void execute() = 0;
    };
  
  class BuyStock : public Order {
  public:
    BuyStock(Stock::Ptr stock) {
      stock_ = stock;
    }
  
    void execute() {
      stock_->buy();
    }
  
  private:
      Stock::Ptr stock_;
  };
  
  class SellStock : public Order {
  public:
    SellStock(Stock::Ptr stock) {
      stock_ = stock;
    }
  
    void execute() {
      stock_->sell();
    }
  
  private:
    Stock::Ptr stock_;
  };
  
  class Broker {
  public:
    Broker() { }
  
    ~Broker() {
      orderList_.clear();
    }
  
    void takeOrder(Order::Ptr order) {
      orderList_.emplace_back(order);
    }
  
    void placeOrders() {
      for (auto& it : orderList_) {
        it->execute();
      }
    }
  
  private:
    std::list<Order::Ptr> orderList_;
  };
  
  void test() {
    std::cout << "\n\n command pattern." << std::endl;
  
    Stock::Ptr qq = std::make_shared<Stock>("qq", 100);
    Stock::Ptr alibaba = std::make_shared<Stock>("alibaba", 400);
  
    auto broker = std::make_shared<Broker>();
    broker->takeOrder(std::make_shared<BuyStock>(qq));
    broker->takeOrder(std::make_shared<SellStock>(qq));
  
    broker->takeOrder(std::make_shared<BuyStock>(alibaba));
    broker->takeOrder(std::make_shared<SellStock>(alibaba));
  
    broker->placeOrders();
  }
  ```

  