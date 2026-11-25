# Adapter适配器模式

  - 意图

    将一个类的接口转换成客户希望的另外一个接口。Adapter模式使得原本由于接口不兼容而不能一起工作的那些类可以一起工作

  - 优缺点

    - 兼容老的接口，适配新接口，让它们可以一起工作
    - 改变一个已有对象的接口

  - UML 图

![Adapter模式](../img/Adapter.png)
  ![Adapter模式示例MediaPlayer](../img/Adapter-MediaPlayer.png)

  - 代码示例

    ```c++
    class AdvancedMediaPlayer {
    public:
        CLASS_PTR(AdvancedMediaPlayer);
        virtual ~AdvancedMediaPlayer() { }
        virtual void play(const std::string& file) = 0;
    };
    
    class VlcPlayer : public AdvancedMediaPlayer {
    public:
        ~VlcPlayer() { }
        void play(const std::string& file) override {
            std::cout << "vlc play: " << file << std::endl;
        }
    };
    
    class Mp4Player : public AdvancedMediaPlayer {
    public:
        ~Mp4Player() { }
        void play(const std::string& file) override {
            std::cout << "mp4 play: " << file << std::endl;
        }
    };
    
    class MediaPlayer {
    public:
    	virtual ~MediaPlayer() { }
    	virtual void play(const std::string& audioType, const std::string& file) = 0;
    };
    
    class MediaAdapter : public MediaPlayer {
    public:
        MediaAdapter(AdvancedMediaPlayer::Ptr player) {
            advanced_player_ = player;
        }
    	~MediaAdapter() override { }
    
    	void play(const std::string& audioType, const std::string& file) override {
    		if (advanced_player_) {
    			advanced_player_->play(file);
    		}
    	}
    
    private:
    	AdvancedMediaPlayer::Ptr advanced_player_;
    };
    
    class AudioPlayer : public MediaPlayer {
    public:
    	AudioPlayer(AdvancedMediaPlayer::Ptr vlc_player,
                AdvancedMediaPlayer::Ptr mp4_player) {
            vlc_player_ = vlc_player;
            mp4_player_ = mp4_player;
    	}
    	~AudioPlayer() override { }
    
    	void play(const std::string& audioType, const std::string& file) override {
    		if (audioType == "mp3") {
    			std::cout << "player mp3: " << file << std::endl;
    		} else if (audioType == "vlc" && vlc_player_) {
                vlc_player_->play(file);
            }
            else if (audioType == "mp4" && mp4_player_) {
                mp4_player_->play(file);
            }
    	}
    
    private:
    	AdvancedMediaPlayer::Ptr vlc_player_;
        AdvancedMediaPlayer::Ptr mp4_player_;
    };
    
    void test() {
        std::cout << "\n\n adapter pattern." << std::endl;
    
        auto vlc_player = std::make_shared<VlcPlayer>();
        auto mp4_player = std::make_shared<Mp4Player>();
    
        auto vlc_media_adapter = std::make_shared<MediaAdapter>(vlc_player);
        auto mp4_media_adapter = std::make_shared<MediaAdapter>(mp4_player);
        vlc_media_adapter->play("vlc", "as long as you love me.vlc");
        mp4_media_adapter->play("mp4", "sexlove.mp4");
    
        auto audio_player = std::make_shared<AudioPlayer>(vlc_player, mp4_player);
        audio_player->play("mp3", "my love.mp3");
        audio_player->play("vlc", "as long as you love me.vlc");
        audio_player->play("mp4", "sexlove.mp4");
    }
    ```

