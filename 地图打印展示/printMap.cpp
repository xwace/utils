#define LOGD(format, ...) printf("[%s][Line: %d][%s]: " format "\n", TAG, __LINE__, __func__, ##__VA_ARGS__)
#define PRINT_IMG(img,name) cv::String imgStr; imgStr << img; LOGD(name":\n%s", imgStr.c_str());
#define COUT_IMG(img) std::cout<<cv::format(img, cv::Formatter::FMT_C)<<std::endl;

#define imgshow(img, ...) IMGshow(img, #img, __func__, __LINE__, ##__VA_ARGS__)
inline void IMGshow(cv::Mat img, std::string picname, std::string funName, int line_indx, std::string opt_name = "")
{
    if (!opt_name.empty())
        picname = opt_name;
    system("mkdir -p ../build/imgs");
    std::string filename = "../build/imgs/" + funName + "_" + std::to_string(line_indx) + "_" + picname + ".png";
    cv::imwrite(filename, img);
}

#define IMSHOW(x,...) imgShow(##x,#x,__LINE__,__func__,##__VA_ARGS__);
inline void imgShow(cv::Mat src, std::string imgName, int line, std::string func, std::string optName = "")
{
    if (src.empty()) return;
    if (!optName.empty()) imgName = optName;
    func = "[" + func + "]";
    std::string imName = std::to_string(line) + imgName + func;
    cv::namedWindow(imName, 2);
    cv::imshow(imName, src);
}

#define COUT(x,...) cout_func(#x,x,##__VA_ARGS__);
template<typename T>
void cout_func_helper(const T& value)
{
    std::cout << value <<" ";
}

template<typename T, typename... Args>
void cout_func_helper(const T& value, const Args&... args)
{
    std::cout << value <<" ";
    cout_func_helper(args...);
}

template<typename... Args>
void cout_func(const std::string& x, const Args&... args)
{
    std::cout << x <<" ";
    cout_func_helper(args...);
    std::cout << std::endl;
}
