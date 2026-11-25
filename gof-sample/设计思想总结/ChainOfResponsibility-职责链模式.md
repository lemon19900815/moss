# Chain Of Responsibility（职责链模式）

  - 意图

    使多个对象都有机会处理请求，从而避免请求的发送者和接收者之间的耦合关系。将这些对象形成一条链，并沿着这条链传递该请求指导有一个对象处理它为止。

  - 优缺点

    - 降低耦合度：该模式使得一个对象无需知道使其他哪一个对象处理其请求。

    - 增强了给对象指派职责的灵活性：你可以通过在运行时刻对该链进行动态的增加或修改来增加或改变处理一个请求的那些职责。你可以将这种机制与静态的特例化处理对象的继承机制结合起来使用。

    - 不保证被接受（缺点）：既然一个请求没有明确的接收者，那么就不能保证它一定会被处理——该请求可能一直到链的末端都得不到处理。一个请求也可能因该链没有被正确配置而得不到处理。

  - UML图
    ![ChainOfResponsibility](../img/ChainOfResponsibility.png)
    ![ChainOfResponsibility-Logger](../img/ChainOfResponsibility-Logger.png)

  - 代码示例

    ```c++
    constexpr int INFO  = 1;
    constexpr int DEBUG = 2;
    constexpr int ERROR = 3;
    
    class AbstractLogger {
    public:
        virtual ~AbstractLogger() { }
        CLASS_PTR(AbstractLogger);
    
        void setNextLogger(AbstractLogger::Ptr nextLogger) {
            next_logger_ = nextLogger;
        }
    
        void logMessage(int level, std::string message) {
            if (level_ <= level) {
                write(message);
            }
    
            if (next_logger_) {
                next_logger_->logMessage(level, message);
            }
        }
    
    private:
        virtual void write(std::string& message) = 0;
    
    protected:
        int32_t level_;
        AbstractLogger::Ptr next_logger_ = nullptr;
    };
    
    class ConsoleLogger : public AbstractLogger {
    public:
        ConsoleLogger() {
            level_ = INFO;
        }
    
        ~ConsoleLogger() { }
    
    private:
        void write(std::string& message) {
            std::cout << "INFO: " << message << std::endl;
        }
    };
    
    class DebugLogger : public AbstractLogger {
    public:
        DebugLogger() {
            level_ = DEBUG;
        }
    
        ~DebugLogger() { }
    
    private:
        void write(std::string& message) {
            std::cout << "DEBUG: " << message << std::endl;
        }
    };
    
    class ErrorLogger : public AbstractLogger {
    public:
        ErrorLogger() {
            level_ = ERROR;
        }
    
        ~ErrorLogger() { }
    
    private:
        void write(std::string& message) {
            std::cout << "ERROR: " << message << std::endl;
        }
    };
    
    void test() {
        std::cout << "\n\n chain of responsibility pattern." << std::endl;
    
        auto infoLogger = std::make_shared<ConsoleLogger>();
        auto debugLogger = std::make_shared<DebugLogger>();
        auto errorLogger = std::make_shared<ErrorLogger>();
    
        errorLogger->setNextLogger(debugLogger);
        debugLogger->setNextLogger(infoLogger);
    
        errorLogger->logMessage(ERROR, "failed to find a");
        errorLogger->logMessage(DEBUG, "failed to find b");
        errorLogger->logMessage(INFO,  "failed to find c");
    }
    
    ```

    