cv::Ptr<cv::ml::EM>em_model;
void GMM_calculate(cv::Mat points, cv::Mat sample) {
    cv::Mat labels;
    if (em_model.empty())//只训练一次，节省训练时间
    {
        em_model = cv::ml::EM::create();
        em_model->setClustersNumber(2);
        em_model->setCovarianceMatrixType(cv::ml::EM::COV_MAT_SPHERICAL);
        em_model->setTermCriteria(cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 100, 0.1));
        em_model->trainEM(points, cv::noArray(), labels);
        cout << "labels: \n" << labels<<endl;
    }

    auto result = em_model->predict2(sample, noArray());
    cout << "高斯混合模型预测结果: " << result<< " 样本： "<< (int)sample.at<double>(0,0)<<" 预测值: "<<result[1]<<endl;
}

int main()
{
    cv::Mat points(11, 1, CV_64FC1);
    cv::Mat sample(1, 1, CV_64FC1);

    points.at<double>(0, 0) = 1;
    points.at<double>(1, 0) = 2;
    points.at<double>(2, 0) = 3;
    points.at<double>(3, 0) = 5;
    points.at<double>(4, 0) = 4;
    points.at<double>(5, 0) = 5;
    points.at<double>(6, 0) = 6;
    points.at<double>(7, 0) = 46;
    points.at<double>(8, 0) = 99;
    points.at<double>(9, 0) = 87;
    points.at<double>(10, 0) = 86;
   
    sample.at<double>(0, 0) = 90;
    GMM_calculate(points, sample);
    sample.at<double>(0, 0) = 1;
    GMM_calculate(points, sample);
    sample.at<double>(0, 0) = 8;
    GMM_calculate(points, sample);
    sample.at<double>(0, 0) = 60;
    GMM_calculate(points, sample);
    sample.at<double>(0, 0) = 50;
    GMM_calculate(points, sample);
}


/*
labels:
[0;
 0;
 0;
 0;
 0;
 0;
 0;
 1;
 1;
 1;
 1]
高斯混合模型预测结果: [-5.06498, 1] 样本： 90 预测值: 1
高斯混合模型预测结果: [-3.20852, 0] 样本： 1 预测值: 0
高斯混合模型预测结果: [-5.18811, 0] 样本： 8 预测值: 0
高斯混合模型预测结果: [-5.40109, 1] 样本： 60 预测值: 1
高斯混合模型预测结果: [-6.01205, 1] 样本： 50 预测值: 1
*/
