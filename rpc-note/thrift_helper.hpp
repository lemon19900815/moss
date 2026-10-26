#ifndef __THRIFT_HELPER_HPP__
#define __THRIFT_HELPER_HPP__

#include <iostream>
#include <mutex> // call_once, once_flag
#include <future> // future, promise

#include <thrift/TBase.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>

#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TNonblockingServer.h>

#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/THttpServer.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/THttpClient.h>

#include <thrift/processor/TMultiplexedProcessor.h>

#include <thrift/async/TAsyncBufferProcessor.h>
#include <thrift/async/TAsyncProtocolProcessor.h>
#include <thrift/async/TEvhttpServer.h>

#include <thrift/concurrency/PlatformThreadFactory.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

// 传输层模板(协议)
template<typename TransportFactory, typename ProtocolFactory>
struct tf_tp_traits
{
    using transport_factory = TransportFactory;
    using protocol_factory = ProtocolFactory;

    static std::shared_ptr<TTransportFactory> create_transport_factory()
    {
        static_assert(std::is_base_of<TTransportFactory, transport_factory>::value,
                      "TransportFactory must based on TTransportFactory!!!");
        return std::make_shared<transport_factory>();
    }

    static std::shared_ptr<TProtocolFactory> create_protocol_factory()
    {
        static_assert(std::is_base_of<TProtocolFactory, protocol_factory>::value,
                      "ProtocolFactory must based on TProtocolFactory!!!");
        return std::make_shared<protocol_factory>();
    }
};

// 业务层模板
template<typename Handler, typename HandlerFactory,
         typename Processor, typename ProcessorFactory>
struct tf_logic_traits
{
    using handler_type = Handler;
    using handler_factory = HandlerFactory;

    using processor_type = Processor;
    using processor_factory = ProcessorFactory;

    static std::shared_ptr<handler_type> create_handler()
    {
        return std::make_shared<handler_type>();
    }

    static std::shared_ptr<handler_factory> create_handler_factory()
    {
        return std::make_shared<handler_factory>();
    }

    static std::shared_ptr<processor_type> create_processor()
    {
        return std::make_shared<processor_type>(create_handler());
    }

    static std::shared_ptr<processor_factory> create_processor_factory()
    {
        return std::make_shared<processor_factory>(create_handler_factory());
    }
};

// logic_traits辅助定义
#define LOGIC_TRAITS(logic) \
    using logic_traits = tf_logic_traits <\
                         logic##Handler, logic##HandlerFactory,\
                         logic##Processor, logic##ProcessorFactory >

// 展开如何下所示，注意TunnelServiceHandler实现的命名规则
//using logic_traits = tf_logic_traits <
//                     TunnelServiceHandler, TunnelServiceHandlerFactory,
//                     TunnelServiceProcessor, TunnelServiceProcessorFactory >;

// 默认的传输方式和协议类型，用户可以对该默认进行修改
namespace default_tf_traits
{

// TFramedTransport – 以frame为单位进行传输，非阻塞式服务中使用（TNonblockingServer）
// TFileTransport – 将数据写入本地文件或从文件读取数据。主要用于本地文件系统的数据传输。
// TSimpleFileTransport - 以文件形式进行传输：只能append
// TBufferedTransport – 在底层传输协议之上添加缓冲区，以提高数据传输的效率。
// TZlibTransport – 使用 zlib 库对数据进行压缩和解压缩，以减小数据传输的大小
// THttpTransport - 使用http进行传输（SubClass: THttpClient,THttpServer）
using transport = TBufferedTransport;

// TBinaryProtocol – 二进制格式.
// TCompactProtocol – 紧凑的二进制格式
// TJSONProtocol – JSON格式
// TDebugProtocol – 使用易懂的可读的文本格式，便于调试
using protocol = TBinaryProtocol;

// 传输层工厂
// TFramedTransportFactory - TFramedTransport
// TBufferedTransportFactory - TBufferedTransport
// TZlibTransportFactory - TZlibTransport
// THttpServerTransportFactory - THttpServer(服务器)
using transport_factory = TBufferedTransportFactory;

// 协议层工厂
// TBinaryProtocolFactory - TBinaryProtocol
// TCompactProtocolFactory - TCompactProtocol
// TJSONProtocolFactory - TJSONProtocol
// TDebugProtocolFactory - TDebugProtocol
using protocol_factory = TBinaryProtocolFactory;

// 传输层工厂&协议层工厂, tp: transport&protocol
using tp_traits = tf_tp_traits<transport_factory, protocol_factory>;

}

// http使用json protocol
namespace tf_http_traits
{

using transport = THttpClient;
using protocol = TJSONProtocol;
using transport_factory = THttpServerTransportFactory;
using protocol_factory = TJSONProtocolFactory;
using tp_traits = tf_tp_traits<transport_factory, protocol_factory>;

}

// thrift的工作模式
enum class tf_workmode
{
    TF_SIMPLE,       // TSimpleServer
    TF_THREAD,       // TThreadedServer
    TF_NOBLOCKING,   // TNonblockingServer
    TF_POOL,         // TThreadPoolServer
};

// 辅助thrift创建服务帮助类，使用方法
// 1、使用宏LOGIC_TRAITS定义服务的Logic类型
// 2、指定服务的传输层工厂和协议层工厂类型，如：default_tf_traits::tp_traits
// 3、创建thrift_service对象
// 4、在指定端口以某种方式工作方式开启thrift服务
// LOGIC_TRAITS(TunnelService);
// thrift_service<logic_traits> service;
// service.start(9090, tf_workmode::TF_POOL);
// service.serve();

//template<typename Logic = tf_logic_traits<>, typename Trans = tf_tp_traits<>>
template<typename Logic, typename Trans = default_tf_traits::tp_traits>
class thrift_service
{
public:
    virtual ~thrift_service()
    {
        stop();
    }

    void stop()
    {
        if (stoped_.load())
        {
            return;
        }

        stoped_.store(true);

        if (server_)
        {
            server_->stop();
        }

        if (thd_.joinable())
        {
            thd_.join();
        }

        server_ = nullptr;
        //serve_promise_.set_value(true);
    }

    bool start(int32_t port, tf_workmode mode = tf_workmode::TF_POOL)
    {
        if (server_)
        {
            std::cerr << "thrift service is started." << std::endl;
            return false;
        }

        try
        {
            create_service(mode, port);
        }
        catch (TException &ex)
        {
            std::cerr << "Error : {}" << ex.what() << std::endl;
            return false;
        }
        catch (...)
        {
            std::cerr << "Unknown Error start thrift service." << std::endl;
            return false;
        }

        stoped_.store(false);
        thd_ = std::thread([this]()
        {
            try
            {
                is_running_.store(true);
                server_->serve(); // 在线程中阻塞提供服务，直到退出
            }
            catch (TException &ex)
            {
                std::cerr << "serve: " << ex.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "serve: Unknown Error." << std::endl;
            }
            is_running_.store(false);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return is_running_.load();
    }

    void serve()
    {
        while (!stoped_.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

private:
    void create_service(tf_workmode mode, int32_t port)
    {
        switch (mode)
        {
        case tf_workmode::TF_SIMPLE:
            create_simple_service(port);
            break;
        case tf_workmode::TF_THREAD:
            create_thread_server(port, true); // use single handler.
            break;
        case tf_workmode::TF_NOBLOCKING:
            create_noblocking_server(port);
            break;
        case tf_workmode::TF_POOL:
            create_threadpool_server(port);
            break;
        default:
            std::cerr << "unknown work mode." << std::endl;
            break;
        }
    }

    // 单线程工作模式
    void create_simple_service(int32_t port)
    {
        // This server only allows one
        // connection at a time, but spawns no threads
        auto server_sock = std::make_shared<TServerSocket>(port);

        auto trans_factory = Trans::create_transport_factory();
        auto proto_factory = Trans::create_protocol_factory();

        server_.reset(new TSimpleServer(
                          Logic::create_processor(),
                          server_sock, trans_factory, proto_factory));
    }

    // 多线程阻塞模式
    void create_thread_server(int32_t port, bool use_single_handler = false)
    {
        auto server_sock = std::make_shared<TServerSocket>(port);

        auto trans_factory = Trans::create_transport_factory();
        auto proto_factory = Trans::create_protocol_factory();

        if (use_single_handler)
        {
            // 注意：**确保接口是可重入的**
            // 使用同一个handler对象
            server_.reset(new TThreadedServer(
                              Logic::create_processor(),
                              server_sock, trans_factory, proto_factory));
        }
        else
        {
            // 使用不同的handler对象
            server_.reset(new TThreadedServer(
                              Logic::create_processor_factory(),
                              server_sock, trans_factory, proto_factory));
        }
    }

    // 非阻塞模式
    void create_noblocking_server(int32_t port)
    {
        // Thrift 的 TNonblockingServer 只支持 TFramedTransport 传输。
        // 这是因为 TNonblockingServer 使用帧（frame）作为消息的边界，
        // 而 TFramedTransport 提供了对帧传输的支持。

        // 创建传输层和协议层
        //auto trans_factory = std::make_shared<TFramedTransportFactory>();
        auto trans_factory = Trans::create_transport_factory();
        auto proto_factory = Trans::create_protocol_factory();

        // 创建非阻塞服务器模型
        auto server_sock = std::make_shared<TNonblockingServerSocket>(port);

        // 不能设置transportFactory，否则会出现下面的异常
        // TNonblockingServer: client died: Frame size has negative value

        //TNonblockingServer server(processorFactory,
        //                          transportFactory, transportFactory,
        //                          protocolFactory, protocolFactory,
        //                          serverTransport, threadManager);

        std::unique_ptr<TNonblockingServer> service;
        service.reset(new TNonblockingServer(
                          Logic::create_processor_factory(),
                          proto_factory, server_sock,
                          create_thread_manager()));
        service->setNumIOThreads(io_thread_num());

        server_ = std::move(service);
    }

    // 线程池服务器模型
    void create_threadpool_server(int32_t port)
    {
        auto server_sock = std::make_shared<TServerSocket>(port);

        auto trans_factory = Trans::create_transport_factory();
        auto proto_factory = Trans::create_protocol_factory();

        // This server allows "workerCount"
        // connection at a time, and reuses threads
        server_.reset(new TThreadPoolServer(
                          Logic::create_processor_factory(),
                          server_sock, trans_factory, proto_factory,
                          create_thread_manager()));
    }

    uint32_t threads_num()
    {
        // 最少4个线程
        return std::max<uint32_t>(4, std::thread::hardware_concurrency());
    }

    uint32_t io_thread_num()
    {
        return threads_num() / 2 + 1;
    }

    std::shared_ptr<ThreadManager> create_thread_manager()
    {
        auto worker_count = threads_num() * 2;

        auto thread_mgr = ThreadManager::newSimpleThreadManager(worker_count);
        thread_mgr->threadFactory(std::make_shared<::apache::thrift::concurrency::PlatformThreadFactory>());
        thread_mgr->start();

        return thread_mgr;
    }

private:
    std::thread thd_;
    std::unique_ptr<TServer> server_{nullptr};

    std::atomic_bool is_running_{ false };
    std::atomic_bool stoped_{true};
    //std::promise<bool> serve_promise_;
};

// 辅助客户端调用thrift服务，使用方法：
// 1.短链接方式
// thrift_client<TunnelServiceClient> cli;
// cli.call_once("localhost", 9090, [](TunnelServiceClient& stub)
// {
//     QcParam param;
//     param.__set_body("hello");
//
//     ResultStr ret;
//     stub.QcDownload(ret, param);
//     ret.printTo(std::cout);
// });
// 2.长链接方式
// thrift_client<TunnelServiceClient> cli;
// if(!cli.init("localhost", 9090)) return;
// cli.call([](TunnelServiceClient& stub)
// {
//     QcParam param;
//     param.__set_body("hello");
//
//     ResultStr ret;
//     stub.QcDownload(ret, param);
//     ret.printTo(std::cout);
// });

template <
    typename Stub,
    typename Transport = default_tf_traits::transport,
    typename Protocol = default_tf_traits::protocol >
class thrift_client
{
public:
    ~thrift_client()
    {
        release();
    }

    // 短链接模式：调用thrift接口，具体调用哪个接口由fn决定
    // @return: true-调用成功，false-调用失败
    bool call(std::string host, int32_t port,
              std::function<void(Stub &stub)> &&fn)
    {
        bool retval = false;

        if (init(host, port))
        {
            retval = call(std::move(fn));
            release();
        }

        return retval;
    }

    // 初始化连接
    bool init(std::string host, int32_t port)
    {
        release();

        // 使用模板特性SFNA做类型初始化
        init_transport<Transport>(host, port);

        // 注：thrift-0.12.0才需要做如下区分，
        //     thrift-0.16.0、0.18.0并不需要区分

        // c++17特性：根据Transport类型，调整构造过程，
        //if constexpr (std::is_same_v<Transport, THttpClient>)
        //{
        //    trans_ = std::make_shared<Transport>(host, port);
        //}
        //else
        //{
        //    auto sock = std::make_shared<TSocket>(host, port);
        //    trans_ = std::make_shared<Transport>(sock);
        //}

        proto_ = std::make_shared<Protocol>(trans_);

        error_ = "";

        try
        {
            trans_->open();
            stub_ = std::make_shared<Stub>(proto_);
            return true;
        }
        catch (TException &ex)
        {
            error_ = ex.what();
        }
        catch (...)
        {
            error_ = "Unknown ERROR";
        }

        trans_ = nullptr;
        proto_ = nullptr;
        stub_ = nullptr;

        return false;
    }

    // 长链接模式：调用thrift接口，具体调用哪个接口由fn决定
    // @return: true-调用成功，false-调用失败
    bool call(std::function<void(Stub &stub)> &&fn)
    {
        // noblocking工作模式只能以TFramedTransport作为传输方式
        // reference thrift paper：7.9- Nonblocking Operation
        // https://thrift.apache.org/static/files/thrift-20070401.pdf
        //auto transport = std::make_shared<TFramedTransport>(sock);
        //auto protocol = std::make_shared<TJSONProtocol>(transport);
        //auto protocol = std::make_shared<TCompactProtocol>(transport);

        error_ = "";

        if (!stub_)
        {
            error_ = "stub is not initialized.";
            return false;
        }

        try
        {
            fn(*stub_);
            return true;
        }
        catch (TException &ex)
        {
            error_ = ex.what();
        }
        catch (...)
        {
            error_ = "Unknown ERROR";
        }

        return false;
    }

    std::string error()
    {
        return error_;
    }

private:
    void release()
    {
        try
        {
            if (trans_)
            {
                trans_->close();
            }

            trans_ = nullptr;
            proto_ = nullptr;
            stub_ = nullptr;
        }
        catch (TException &ex)
        {
            std::cerr << __FUNCTION__ << ": " << ex.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << __FUNCTION__ << ": unknown error" << std::endl;
        }
    }

    // 使用模板特性SFNA做类型选择初始化，
    // c++17可以使用constexpr判断来实现相同的功能
    template<typename T>
    typename std::enable_if<std::is_same<T, THttpClient>::value, void>::type
    init_transport(std::string host, int32_t port)
    {
        trans_ = std::make_shared<T>(host, port);
    }

    template<typename T>
    typename std::enable_if < !std::is_same<T, THttpClient>::value, void >::type
    init_transport(std::string host, int32_t port)
    {
        auto sock = std::make_shared<TSocket>(host, port);
        trans_ = std::make_shared<T>(sock);
    }

private:
    std::string error_;
    std::shared_ptr<Stub> stub_{ nullptr };
    std::shared_ptr<Transport> trans_{nullptr};
    std::shared_ptr<Protocol> proto_{nullptr};
};

// 转换器：thrift对象和string相互转换
template<typename T, typename P>
class converter
{
public:
    // 从字符串到thrift对象的转换
    static T to_thrift(const std::string &content)
    {
        static_assert(std::is_base_of<TBase, T>::value,
                      "T must based on TBase");
        static_assert(std::is_base_of<TProtocol, P>::value,
                      "P must based on TProtocol");

        auto buffer = std::make_shared<TMemoryBuffer>(
                          (uint8_t *)content.c_str(),
                          static_cast<uint32_t>(content.size()));

        auto proto = std::make_shared<P>(buffer);

        T obj;
        obj.read(proto.get());

        return obj;
    }

    // 从thrift对象到字符串的转换
    static std::string to_string(const T &obj)
    {
        static_assert(std::is_base_of<TBase, T>::value,
                      "T must based on TBase");
        static_assert(std::is_base_of<TProtocol, P>::value,
                      "P must based on TProtocol");

        auto buffer = std::make_shared<TMemoryBuffer>();
        auto proto = std::make_shared<P>(buffer);

        obj.write(proto.get());

        return buffer->getBufferAsString();
    }
};

// 注：json的key是field id,与常规json存在差异
template<typename T>
using json_converter = converter<T, TJSONProtocol>;

#endif // __THRIFT_HELPER_HPP__
