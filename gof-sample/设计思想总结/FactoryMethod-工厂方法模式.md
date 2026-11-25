### **Factory Method** 工厂方法

  - 优缺点

    - 上层可以只定义默认操作过程，方法实现可以延迟到子类中进行（可以参见 `InviteCode.Exchange` 的使用）
    - Abstract Factory 通常用工厂方法实现

  - UML 图

    ![一般模式](../img/FactoryMethod.png)
    ![邀请码示例](../img/FactoryMethod-InviteCode.png)

  - 代码示例1（迷宫游戏创建）

  ```c++
  class MazeGameV2 {
  public:
      Maze::Ptr CreateMaze() {
          auto maze = MakeMaze();
  
          auto r1 = MakeRoom(1);
          auto r2 = MakeRoom(2);
  
          maze->AddRoom(r1);
          maze->AddRoom(r2);
  
          auto door = MakeDoor(r1, r2);
  
          r1->SetSide(North, MakeWall());
          r1->SetSide(East, door);
          r1->SetSide(South, MakeWall());
          r1->SetSide(West, MakeWall());
  
          r2->SetSide(North, MakeWall());
          r2->SetSide(East, MakeWall());
          r2->SetSide(South, MakeWall());
          r2->SetSide(West, door);
  
          return maze;
      }
  
      // factory method:
      virtual Maze::Ptr MakeMaze() {
          return std::make_shared<Maze>();
      }
      virtual Room::Ptr MakeRoom(int32_t n) {
          return std::make_shared<Room>(n);
      }
      virtual Wall::Ptr MakeWall() {
          return std::make_shared<Wall>();
      }
      virtual Door::Ptr MakeDoor(Room::Ptr r1, Room::Ptr r2) {
          return std::make_shared<Door>(r1, r2);
      }
  };
  
  class BombedMazeGame : public MazeGameV2 {
  public:
      Wall::Ptr MakeWall() override {
          return std::make_shared<BombedWall>();
      }
      Room::Ptr MakeRoom(int32_t n) override {
          return std::make_shared<RoomWithABomb>(n);
      }
  };
  
  // 测试用例
  void factoryMethodTest() {
      MazeGameV2 game;
      auto maze1 = game.CreateMaze();
  
      BombedMazeGame game2;
      auto maze2 = game2.CreateMaze();
  }
  
  ```

  - 代码示例2（邀请码使用）

  ```c++
  class InviteCode {
  public:
      virtual ~InviteCode() {}
  
      bool Exchange() {
          if (!CheckCode())
              return false;
          if (!CheckValid())
              return false;
          if (!GiveReward())
              return false;
          Record();
          return true;
      }
  
      CLASS_PTR(InviteCode);
  
  protected:
      virtual bool CheckCode() {
          return true;
      }
  
      virtual bool CheckValid() {
          return true;
      }
  
      virtual bool GiveReward() {
          return true;
      }
  
      virtual void Record() {
      }
  };
  
  class ErrorCode : public InviteCode {
  protected:
      bool CheckCode() override {
          return false;
      }
  };
  
  class OnceCode : public InviteCode {
  protected:
      bool CheckValid() override {
          // TODO：检查玩家是否已经领取过同类型的邀请码
          return true;
      }
  
      void Record() override {
          // TODO：记录领取过该类型的邀请码，以后不允许再领取该类型
      }
  };
  
  class ActivityCode : public InviteCode {
  protected:
      bool CheckValid() override {
          // TODO：检查玩家是否已经领取过同类型的邀请码
          return true;
      }
  
      void Record() override {
          // TODO：记录领取过该类型的邀请码，以后不允许再领取该类型
      }
  };
  
  // 模板工厂
  class InviteCodeFactory {
  public:
      template<class TYPE>
      static TYPE::Ptr Create() {
          return std::make_shared<TYPE>();
      }
  };
  
  void factoryMethodTest() {
      ...
  
      // 其他示例：邀请码使用
      auto invite_code = InviteCodeFactory::Create<OnceCode>();
      invite_code->Exchange();
  
      invite_code = InviteCodeFactory::Create<ActivityCode>();
      invite_code->Exchange();
  }
  ```

