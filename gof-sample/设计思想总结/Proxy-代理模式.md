# Proxy代理模式

- 意图

  为其他对象提供一种代理以**控制**对这个对象的访问。

- 优点

  - 可以隐藏对象的实现细节。
  - 如果`RealSubject`和`Subject`接口相同，代理可以引用`Subject`。（变成了和**Decorator**的UML结构类似，但是它们的目的不同）

- UML图

  ![Proxy模式](../img/Proxy.png)
  ![Proxy模式-Image实例](../img/Proxy-Image.png)

- 代码示例

  ```c++
  class Image {
  public:
  	virtual ~Image() { }
      CLASS_PTR(Image);
  
  	virtual void display() = 0;
  };
  
  class RealImage : public Image {
  public:
  	RealImage(std::string& file) {
  		filename_ = file;
  		loadFromDisk();
  	}
  
  	void display() override {
  		std::cout << "display real image." << std::endl;
  	}
  
  private:
  	void loadFromDisk() {
  		std::cout << "load " << filename_ << " from disk." << std::endl;
  	}
  
  private:
  	std::string filename_;
  };
  
  class ProxyImage : public Image {
  public:
  	ProxyImage(std::string filename) {
          filename_ = filename;
  	}
  
  	void display() override {
          // 延迟实例化
          if (!real_img_) {
              real_img_ = std::make_shared<RealImage>(filename_);
          }
          real_img_->display();
  	}
  
  private:
      std::string filename_;
  	Image::Ptr real_img_;
  };
  
  void test() {
      std::cout << "\n\n proxy pattern." << std::endl;
  
      auto image = std::make_shared<ProxyImage>("beautiful.jpg");
      image->display();
  }
  ```

