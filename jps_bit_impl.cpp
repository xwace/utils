cv::Mat mat2 = (cv::Mat_<uchar>(1,6)<<0,255,0,0,255,0);
cv::Rect r1{0,0,5,1};
cv::Rect r2{1,0,5,1};
cv::Mat result = (~mat2)(r1)&(mat2)(r2);

cout<<"mm: "<<(~mat2)(r1)<<" "<<(mat2)(r2)<<endl;
cout<<"result: "<<result<<endl;
