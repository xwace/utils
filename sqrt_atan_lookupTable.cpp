/*输入坐标：米单位的横纵坐标 输出平方根距离
*查表法：提前存储范围内Rect(-0.08,-0.32,0.16,0.40) 坐标对应的平方根值到Mat像素，使用时直接查找输出
* pose转node点，自定义坐标原点在图像中心，设定分辨率
* a²+b²=r² 画出半径，并给圆周上的像素赋值为半径
*
*/

cv::Mat dist_lookup_, angle_lookup_;
void drawDistAndAngleLookUp()
{
	// 相关参数设置，这里设置成m单位，避免后面需要进行单位转换
	float dist_resolution_ = 0.001f; // 1mm的距离分辨率
	float angle_resolution_ = 1.0f;  // 1度的角度分辨率

	// 扫地机正前侧，320mm开始减速，90度
	float dist_front = 0.32f;
	// 扫地机后方距离80mm距离检测，270度
	float dist_back = 0.08f;
	// 扫地机左侧距离20mm，0度
	float dist_left = 0.08f;
	// 扫地机右侧距离50mm， 180度
	float dist_right = 0.08f;
	// 机身半径
	float robot_radius = 0.177f;
	// 计算扫地机中心点mat中的位置
	cv::Point robot_center = cv::Point(std::round((dist_left + robot_radius) / dist_resolution_) + 1, std::round((dist_front + robot_radius) / dist_resolution_) + 1);
	// mat图像的尺寸
	int height = std::round((dist_front + robot_radius * 2 + dist_back) / dist_resolution_) + 2;
	int width = std::round((dist_left + robot_radius * 2 + dist_right) / dist_resolution_) + 2;

	dist_lookup_ = cv::Mat::zeros(height, width, CV_16UC1);
	angle_lookup_ = cv::Mat::zeros(height, width, CV_16UC1);

	cout << "mapsize: " << dist_lookup_.rows << " " << dist_lookup_.cols << endl;
	cv::Mat look;

	int raduis_diag = std::round(std::hypot(robot_center.x, robot_center.y)) + 8;
	// 绘制dist_lookup_上半部分的距离圆圈，小圈覆盖大圆部分
	for (int i = raduis_diag; i > 0; i--)
	{
		// 待绘制图像、中心点、横纵轴长、180为角度起点，绘制角度范围为0~180度，填充的数值，-1表示填充满
		cv::ellipse(dist_lookup_, robot_center, cv::Size(i, i), 180, 0, 180, i, -1);
	}
	cout << "rdius1: " << raduis_diag << endl;
	dist_lookup_.convertTo(look, CV_32F);
	normalize(look, look, 0, 1, NORM_MINMAX);
	imshow("look0", look); waitKey();

	// 绘制角度mat angle_lookup_
	// 已左侧为起点，逆时针旋转，即将扫地机的坐标系逆时针90度
	for (int i = 0; i < 360; i++)
	{
		cv::ellipse(angle_lookup_, robot_center, cv::Size(raduis_diag, raduis_diag), 270, 360 - i - 1, 360 - i, i, -1);
	}


	int radius_back = std::round((dist_back + robot_radius) / dist_resolution_);
	raduis_diag = std::round(std::hypot(radius_back, radius_back)) + 8;
	// 绘制dist_lookup_上半部分的距离圆圈，小圈覆盖大圆部分
	for (int i = raduis_diag; i > 0; i--)
	{
		cv::ellipse(dist_lookup_, robot_center, cv::Size(i, i), 180, 180, 360, i, -1);

	}
	cout << "rdius2: " << raduis_diag << endl;


	dist_lookup_.convertTo(look, CV_32F);
	normalize(look, look, 0, 1, NORM_MINMAX);
	imshow("look0", look);
	imshow("look", look); waitKey();
}

bool GetDistAngleLookUp(float diff_x, float diff_y, float pose_theta, int& dist, int& angle)
{
	float dist_resolution_ = 0.001f; // 1mm的距离分辨率
	float angle_resolution_ = 1.0f;  // 1度的角度分辨率
	// 扫地机正前侧，320mm开始减速，90度
	float dist_front = 0.32f;
	// 扫地机后方距离80mm距离检测，270度
	float dist_back = 0.08f;
	// 扫地机左侧距离20mm，0度
	float dist_left = 0.08f;
	// 扫地机右侧距离50mm， 180度
	float dist_right = 0.08f;
	// 机身半径
	float robot_radius = 0.177f;
	// 计算扫地机中心点mat中的位置
	cv::Point robot_center = cv::Point(std::round((dist_left + robot_radius) / dist_resolution_) + 1, std::round((dist_front + robot_radius) / dist_resolution_) + 1);

	// 将扫地机坐标系转换到图像坐标系
	int node_x = std::round(-(diff_y) / dist_resolution_) + robot_center.x;
	int node_y = std::round(-(diff_x) / dist_resolution_) + robot_center.y;

	int height = std::round((dist_front + robot_radius * 2 + dist_back) / dist_resolution_) + 2;
	int width = std::round((dist_left + robot_radius * 2 + dist_right) / dist_resolution_) + 2;

	cout << "node_x: " << node_x <<" y: "<< node_y << endl;

	drawDistAndAngleLookUp();
	if (node_x < 0 or node_x >= dist_lookup_.cols or node_y < 0 or node_y >= dist_lookup_.rows)
		return false;

	// 查表距离(mm)
	dist = static_cast<int>(dist_lookup_.at<ushort>(node_y, node_x)) * dist_resolution_ * 1000;
	cout << "result dist: " << dist << endl;

	// 查表角度，并获取扫地机坐标系的角度，转换到雷达坐标系
	if (pose_theta < 0.0f)
		pose_theta += 360.0f;
	if (pose_theta > 360.0f)
		pose_theta -= 360.0f;
	angle = 90 - std::round(static_cast<float>(angle_lookup_.at<ushort>(node_y, node_x)) - pose_theta);
	if (angle < 0)
		angle += 360;
	if (angle >= 360)//屏蔽angle = 360情况，避免出现angle = 360导致一直打印错误(外部数据为[0,360))
		angle -= 360;

	// LOGD("### [%d, %d], dist=%hu, angle=%hu\n", node_x, node_y, dist_lookup_.at<ushort>(node_y, node_x), angle_lookup_.at<ushort>(node_y, node_x));
	return true;
}

void test()
{
  int dist;
	int angle;
	GetDistAngleLookUp(0.03, 0.04, 0.1, dist, angle);
}
