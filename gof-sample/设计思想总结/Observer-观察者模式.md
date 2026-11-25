# Observer-观察者模式

- 意图

  定义对象间的一种一对多的依赖关系，当一个对象的状态发生改变时，所有依赖于它的对象都得到通知并被自动更新。

- 别名

  依赖（Dependents），发布-订阅（Publish-Subscribe）

- 优缺点

  - Observer模式允许你独立的改变目标（Subject）和观察者（Observer）；
  - 目标和观察者间的抽象耦合：一个目标所知道的仅仅是它有一系列观察者，每个都符合抽象的Observer类的简单接口。目标不知道任何一个观察者属于哪一个具体的类。这样目标和观察者之间的耦合是抽象的和最小的。
  - 支持广播通信：不像通常的请求，目标发送的通知不需要指定它的接收者。通知被自动广播给所有已向该目标对象登记的有关对象。目标对象并不关心到底有多少对象对自己感兴趣；它唯一的职责就是通知它的各观察者。这给了你在任何时刻增加和删除观察者的自由。处理还是忽略一个通知取决于观察者。
  - 意外的更新（缺点）：因为一个观察者并不知道其他观察者的存在，它可能对改变目标的最终代价一无所知。在目标上一个看似无害的操作可能会引起一系列对观察者以及依赖这些观察者的那些对象的更新。此外，如果依赖准则的定义或维护不当，常常会引起错误的更新，这种错误通常很难捕捉。简单的更新协议不提供具体细节说明目标中什么被改变了，这就使得上述问题变得更加严重。如果没有其他协议帮助观察者发现什么发生了改变，它们可能会被迫尽力减少改变。

- UML图

  ![Observer模式](../img/Observer.png)
  ![Observer结合Mediator](../img/Observer-Mediator.png)

- 代码示例

  ```c++
  // observer.h
  class Subject;
  class Observer {
  public:
    virtual ~Observer() { }
    CLASS_PTR(Observer);
  
    virtual void Update(std::shared_ptr<Subject> subject) = 0;
  };
  
  class BinaryObserver : public Observer {
  public:
    void Update(std::shared_ptr<Subject> subject) override {
      std::cout << "BinaryObserver update." << std::endl;
    }
  };
  
  class OctalObserver : public Observer {
  public:
    void Update(std::shared_ptr<Subject> subject) override {
      std::cout << "OctalObserver update." << std::endl;
    }
  };
  
  class HexaObserver : public Observer {
  public:
    void Update(std::shared_ptr<Subject> subject) override {
      std::cout << "HexaObserver update." << std::endl;
    }
  };
  
  // subject.h
  class Subject : std::enable_shared_from_this<Subject> {
  public:
    Subject() {
      state_ = 0;
    }
    CLASS_PTR(Subject);
  
    int GetState() {
      return state_;
    }
  
    void SetState(int state) {
      state_ = state;
      NotifyAll();
    }
  
    void Attach(Observer::Ptr observer) {
      observers_.push_back(observer);
    }
  
    void NotifyAll() {
      auto this_sub = shared_from_this();
      for (auto& it : observers_) {
        it->Update(this_sub);
      }
    }
  
  private:
    int32_t state_;
    std::list<Observer::Ptr> observers_;
  };
  
  void test() {
    std::cout << "\n\n observer pattern." << std::endl;
  
    auto subject = std::make_shared<Subject>();
    auto observer1 = std::make_shared<BinaryObserver>();
    auto observer2 = std::make_shared<OctalObserver>();
    auto observer3 = std::make_shared<HexaObserver>();
  
    subject->Attach(observer1);
    subject->Attach(observer2);
    subject->Attach(observer3);
  
    subject->SetState(1);
  }
  ```

