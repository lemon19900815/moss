### **Prototype** 原型模式

  - 优缺点

    - 原型不需要再创建工厂实例（像`BombedFactory & EnchantedFactory`等）
    - 缺点：该模式最大的缺点就是每一个Product类都需要实现Clone接口，该接口可能存在实现困难的情况

  - UML 图

    ![原型模式](../img/Prototype.png)

  - 代码示例

    ```c++
    class MazePrototypeFactory : public MazeFactory {
    public:
        MazePrototypeFactory(Maze::Ptr prototype_maze,
            Wall::Ptr prototype_wall, Room::Ptr prototype_room,
            Door::Ptr prototype_door) {
            prototype_maze_ = prototype_maze;
            prototype_wall_ = prototype_wall;
            prototype_room_ = prototype_room;
            prototype_door_ = prototype_door;
        }
    
        Maze::Ptr MakeMaze() override {
            return prototype_maze_->Clone();
        }
        Wall::Ptr MakeWall() override {
            return prototype_wall_->Clone();
        }
        Room::Ptr MakeRoom(int num) override {
            auto room = prototype_room_->Clone();
            room->Initilize(num);
            return room;
        }
        Door::Ptr MakeDoor(Room::Ptr r1, Room::Ptr r2) override {
            auto door = prototype_door_->Clone();
            door->Initialize(r1, r2);
            return door;
        }
    
    private:
        Maze::Ptr prototype_maze_;
        Wall::Ptr prototype_wall_;
        Room::Ptr prototype_room_;
        Door::Ptr prototype_door_;
    };
    
    void prototypeTest() {
        MazeGame game;
    
        auto prototype_maze = std::make_shared<Maze>();
        auto prototype_wall = std::make_shared<Wall>();
        auto prototype_room = std::make_shared<Room>(0);
        auto prototype_door = std::make_shared<Door>(
            std::make_shared<Room>(1), std::make_shared<Room>(2));
    
        MazePrototypeFactory simpleMazeFactory(
            prototype_maze,
            prototype_wall,
            prototype_room,
            prototype_door);
    
        auto simple_maze = game.CreateMaze(simpleMazeFactory);
    
        auto bombed_wall = std::make_shared<BombedWall>();
        MazePrototypeFactory bombedMazeFactory(
            prototype_maze,
            bombed_wall,
            prototype_room,
            prototype_door);
    
        auto bombed_maze = game.CreateMaze(bombedMazeFactory);
    
        /* 
            原型模式和抽象工厂的比较：
            1、原型不需要再创建工厂实例（像BombedFactory&EnchantedFactory等）
            2、使用抽象工厂时，不需要为每一个Product对象实现Clone方法，特别是Clone存在困难时
        */
    }
    
    ```

