cv::Mat mat2 = (cv::Mat_<uchar>(1,6)<<0,255,0,0,255,0);
cv::Rect r1{0,0,5,1};
cv::Rect r2{1,0,5,1};
cv::Mat result = (~mat2)(r1)&(mat2)(r2);

cout<<"mm: "<<(~mat2)(r1)<<" "<<(mat2)(r2)<<endl;
cout<<"result: "<<result<<endl;

/*地图从Mat转为二维枚举容器*/
cv::Mat src_32s;
src.convertTo(src_32s,CV_32S);
randu(src,0,29);
vector<vector<GridOccupyType>>vecG(src.rows,vector<GridOccupyType>(src.cols));
for (size_t i = 0; i < src.rows; i++)
{
    memcpy(&vecG[i][0],src_32s.ptr<int>(i),sizeof(int)*src.cols);
}
