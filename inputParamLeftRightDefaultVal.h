/*
  模板函数实现输入参数：左值引用，右值，默认值。利用模板的万能引用特性。
*/

bool isRobotInDividedArea(SLAM::RealtimePosition&& output);

template<typename T = SLAM::RealtimePosition>
bool isRobotInDividedArea(T&& output = SLAM::RealtimePosition()){
  return isRobotInDividedArea(std::forward<SLAM::RealtimePosition>(output));
};

/* cpp文件实现
  isRobotInDividedArea(SLAM::RealtimePosition&& output){};
*/

bool isRobotInDividedArea(SLAM::RealtimePosition& output);

bool isRobotInDividedArea(){
  SLAM::RealtimePosition tmp;
  return isRobotInDividedArea(tmp);
}
