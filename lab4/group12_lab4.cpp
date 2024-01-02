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

struct framebuffer_info
{
    uint32_t bits_per_pixel;    // depth of framebuffer
    uint32_t xres_virtual;      // how many pixel in a row in virtual screen
};

struct framebuffer_info get_framebuffer_info ( const char *framebuffer_device_path );

int main ( int argc, const char *argv[] )
{
    // variable to store the frame get from video stream
    cv::Mat frame;
    cv::Size2f frame_size;
    cv::VideoCapture camera(2);
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");
    std::ofstream ofs("/dev/fb0");
    if( !ofs )
    {
        std::cerr << "Could not open video device." << std::endl;
        return 1;
    }
    camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480); // Set your desired height here
    camera.set(cv::CAP_PROP_FPS, 60);
    int count = 0;
 	char screenshot_name[100];
    while ( true )
    {
        camera >> frame;
        frame_size = frame.size();
		pid_t pid = fork();	
        count = count+1;
		if (pid==0){
			char input;
        	char targetChar = 'c';
        	char quitChar = 'q';
        	// 獲取終端的屬性
        	struct termios originalTerm, modifiedTerm;
        	tcgetattr(STDIN_FILENO, &originalTerm);

        	// 修改終端屬性以啟用非規範模式
        	modifiedTerm = originalTerm;
        	modifiedTerm.c_lflag &= ~(ICANON);
        	tcsetattr(STDIN_FILENO, TCSANOW, &modifiedTerm);
        	std::cin >> input;  // 從標準輸入讀取一個字符
			camera >> frame;
			cv::imwrite("screenshot_2.png", frame);
			if (input == 'c') {
				std::cout<<456<<"\n";
				std::time_t currentTime = std::time(0);
				//std::cout<<(int)(currentTime);
				std::stringstream ss;
//		ss << (int)(currentTime);
				ss << count;
 				std::string strNumber = ss.str();
//		strNumber = "./screenshot/" + strNumber + ".png";
				strNumber = "./screenshot/" + strNumber + ".png" ;
				cv::imwrite(strNumber, frame);
				
			}
			else {
            	// 恢復終端的原始屬性
    			tcsetattr(STDIN_FILENO, TCSANOW, &originalTerm);
				std::cout<<123<<"\n";
        	}
			tcsetattr(STDIN_FILENO, TCSANOW, &originalTerm);
			std::cout<<789<<"\n";
			exit(EXIT_SUCCESS);
		}
		cv::cvtColor(frame, frame, cv::COLOR_BGR2BGR565);
        // output the video frame to framebufer row by row
		if (pid>0){
		    for ( int y = 0; y < frame_size.height; y++ )
		    {
				int center = (fb_info.xres_virtual - frame_size.width)/2;			
				int position = ( y * fb_info.xres_virtual+center)*2; 					//16bit = 2byte
		        ofs.seekp(position);													//output position
		        ofs.write(reinterpret_cast<char*>(frame.ptr(y)), frame_size.width * 2);	//the number of row byte
		    }
		}
	}
    camera.release ();

    return 0;
}

struct framebuffer_info get_framebuffer_info ( const char *framebuffer_device_path )
{
    struct framebuffer_info fb_info;        // Used to return the required attrs.
    struct fb_var_screeninfo screen_info;   // Used to get attributes of the device from OS kernel.

    int fd = open(framebuffer_device_path, O_RDWR);
    if (fd >= 0) {
        if (!ioctl(fd, FBIOGET_VSCREENINFO, &screen_info)) {
            fb_info.xres_virtual = screen_info.xres_virtual;
            fb_info.bits_per_pixel = screen_info.bits_per_pixel;
        }
    }
	//std::cout<<fb_info.xres_virtual<<"\n";
	//std::cout<<fb_info.bits_per_pixel;
    return fb_info;
};
