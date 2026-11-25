# Composite组合模式

- 意图

  将对象组合成树形结构以表示“部分-整体”的层次结构。Composite模式使得用户对单个对象和组合对象的使用具有一致性。

- 优点

  - 定义了包含基本对象和组合对象的类层次结构。基本对象可以被组合成更复杂的组合对象，而这个组合对象又可以被组合，这样不断的递归下去。客户代码中，任何用到基本对象的地方都可以使用组合对象。
  - 简化客户代码，客户可以一致地使用组个结构和单个对象
  - 更容易增加新类型的组件
  - （缺点）使你的设计更加一般化，容易增加新组件也会产生一些问题，那就是很难限制组合中的组件。有时你希望一个组合只能有某些特定的组件。使用Composite时，你不能依赖类型系统施加这些约束，而必须在运行时刻进行检查
  - （缺点）基础接口组件会暴露Composite组件的接口，但是Composite接口（如Add,Remove,GetChild等）又不是其他非Composite组件接口所必须的，一个可能的办法是在公共基础组件中增加GetComposite接口，用于获取Composite组件，默认的公共接口Component提供缺省的默认实现返回空对象。参见UML图-Composite-Equipment

- UML图

  ![Composite模式](../img/Composite.png)
  ![Composite模式-Equipment示例](../img/Composite-Equipment.png)

- 代码示例

  ```c++
  class CompositeEquipment;
  
  class Equipment {
  public:
    Equipment(std::string name) { name_ = name; }
    virtual ~Equipment() { }
    CLASS_PTR(Equipment);
  
    // 这个接口为了屏蔽Composite类的接口Add&Remove;减少公共类中不需要的接口
    virtual std::shared_ptr<CompositeEquipment> GetComposite() { return nullptr; }
    virtual std::string toString() { return name_; }
  
    // 其他公共类接口定义...
  private:
    std::string name_;
    // other member...
  };
  
  class CompositeEquipment : public Equipment,
      public std::enable_shared_from_this<CompositeEquipment> {
  public:
    CompositeEquipment(std::string name);
    ~CompositeEquipment();
    CLASS_PTR(CompositeEquipment);
  
    void Add(Equipment::Ptr&& p);
    void Remove(Equipment::Ptr&& p);
      
    CompositeEquipment::Ptr GetComposite() override;
    std::string toString() override;
  
  private:
    std::list<Equipment::Ptr> composites_;
  };
  
  class FloppyDisk : public Equipment {
  public:
    FloppyDisk(std::string name) : Equipment(name) { }
  };
  
  // TODO：实现需要放到cpp中
  void test() {
    std::cout << "\n\n Composite Pattern." << std::endl;
  
    Equipment::Ptr equipment = std::make_shared<CompositeEquipment>("CE");
    auto composite = equipment->GetComposite();
    if (composite) {
      composite->Add(std::make_shared<FloppyDisk>("FloppyDisk1"));
      composite->Add(std::make_shared<FloppyDisk>("FloppyDisk2"));
      composite->Add(std::make_shared<FloppyDisk>("FloppyDisk3"));
    }
      
    std::cout << equipment->toString() << std::endl;
  }
  
  ```

