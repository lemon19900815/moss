# Builder

   - 优缺点

     - 构建复杂对象，内部各个部分构建完成后，组合成复杂对象
     - 用户只需要关心我最后实际使用的复杂对象，而不必关心中间的构建过程

   - UML 图
     ![一般模式](../img/Builder-General.png)
     ![迷宫示例](../img/Builder-MazeGame.png)

   - 代码示例

     ```c++
     // 建造者/生成器模式
     class MazeBuilder {
     public:
         virtual void BuildMaze() {}
         virtual void BuildRoom(int32_t num) {}
         virtual void BuildDoor(int32_t room_from, int32_t room_to) {}
         virtual Maze::Ptr GetMaze() { return nullptr; }
     
     protected:
         MazeBuilder() {}
     };
     
     class StandardBuilder : public MazeBuilder {
       ...  
     };
     
     class CountingBuilder : public MazeBuilder {
       ...
     };
     
     Maze::Ptr CreateMaze(MazeBuilder& builder) {
         builder.BuildMaze();
     
         builder.BuildRoom(1);
         builder.BuildRoom(2);
         builder.BuildDoor(1, 2);
     
         return builder.GetMaze();
     }
     
     void builderTest() {
         MazeGame game;
         StandardMazeBuilder builder;
     
         auto maze1 = game.CreateMaze(builder);
     
         CountingMazeBuilder counting_builder;
         auto maze2 = game.CreateMaze(counting_builder);
     
         int32_t rooms, doors;
         counting_builder.GetCounts(rooms, doors);
     
         std::cout << "The maze has "
             << rooms << " rooms and "
             << doors << " doors" << std::endl;
     
         // builder和factory比较
         // 1、builder着重于一步步构建一个复杂的对象
     }
     ```

