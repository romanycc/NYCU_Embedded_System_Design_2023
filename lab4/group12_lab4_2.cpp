#include <fcntl.h> 
#include <fstream>
#include <linux/fb.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/ioctl.h>
#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/fb.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>
#include <ctime>
#include <sstream>

#include <pthread.h>
#include <termios.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int mode = 0;
struct framebuffer_info
{
    uint32_t bits_per_pixel;    // framebuffer depth
    uint32_t xres_virtual;      // how many pixel in a row in virtual screen
};

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path);

int main(int argc, const char *argv[])
{
    cv::Mat image;
    cv::Size2f image_size;
    
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");
    std::ofstream ofs("/dev/fb0");
    image = cv::imread("picture.png", cv::IMREAD_COLOR);
    image_size = image.size();
    std::cout<<image_size.width<<" "<<image_size.height;
    if (image.empty()) {
	    std::cerr << "Error: Could not read the image file." << std::endl;
	    return -1;
    }
    cv::Mat image_bgr565;     
    cv::cvtColor(image, image_bgr565, cv::COLOR_BGR2BGR565);
    // output to framebufer row by row
	int count = 0;

	pthread_t my_thread;
	void* command(void* arg);
    // 创建线程
    if (pthread_create(&my_thread, NULL, command, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }
	
	int get_mode = 0;

	while(true){
		for (int y = 350; y <770; y++)
		{
			pthread_mutex_lock(&mutex);
        	get_mode = mode;
        	pthread_mutex_unlock(&mutex);
			if (mode==1){
		    int position = y * fb_info.xres_virtual * fb_info.bits_per_pixel / 8;
		    ofs.seekp(position);
		    ofs.write(reinterpret_cast<char*>(image_bgr565.ptr(y))+count, image_size.width*2); // 2 bytes per pixel (BGR565)
			}
			else{
				int position = y * fb_info.xres_virtual * fb_info.bits_per_pixel / 8;
		    	ofs.seekp(position);
		   	 	ofs.write(reinterpret_cast<char*>(image_bgr565.ptr(y))+(7680*2-count%7680), image_size.width*2); // 2 bytes per
			}

		}
		count = count + 384;
	}
    return 0;
}

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path)
{
    struct framebuffer_info fb_info;        // Used to return the required attrs.
    struct fb_var_screeninfo screen_info;   // Used to get attributes of the device from OS kernel.

    int fb_fd = open(framebuffer_device_path, O_RDWR); // read/write
    if (fb_fd == -1)
    {
        perror("Error opening framebuffer device");
        exit(1);
    }
   
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &screen_info)) 
    {
        perror("Error reading framebuffer info");
        exit(1);
    }

    fb_info.xres_virtual = screen_info.xres_virtual;
    fb_info.bits_per_pixel = screen_info.bits_per_pixel;
	std::cout<<fb_info.xres_virtual<<" "<<screen_info.yres_virtual<<" "<<fb_info.bits_per_pixel;
    return fb_info;
};


void* command(void* arg) {
	while(true){
		// 獲取終端的屬性
		struct termios originalTerm, modifiedTerm;
		tcgetattr(STDIN_FILENO, &originalTerm);
		// 修改終端屬性以啟用非規範模式
		modifiedTerm = originalTerm;
		modifiedTerm.c_lflag &= ~(ICANON);
		tcsetattr(STDIN_FILENO, TCSANOW, &modifiedTerm);
		char input;
		std::cin >> input;  // 從標準輸入讀取一個字符
		if (input == 'j') {
			std::cout<<7<<"\n";
			pthread_mutex_lock(&mutex);
			mode=0;
			pthread_mutex_unlock(&mutex);
		}
		else if (input == 'l') {
			std::cout<<9<<"\n";
			pthread_mutex_lock(&mutex);
			mode=1;
			pthread_mutex_unlock(&mutex);
		}
		else {
			// 恢復終端的原始屬性
			tcsetattr(STDIN_FILENO, TCSANOW, &originalTerm);
			std::cout<<123<<"\n";
			break;
		}
		tcsetattr(STDIN_FILENO, TCSANOW, &originalTerm);
		std::cout<<111<<"\n";
		//exit(EXIT_SUCCESS);
    }
    return NULL;
}

