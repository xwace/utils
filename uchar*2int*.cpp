void main() {
    Mat s(5, 5, CV_8UC4);
    Mat d1(5, 5, CV_8UC1, Scalar(2));
    Mat d2(5, 5, CV_8UC1,Scalar(3));
    Mat d3(5, 5, CV_8UC1, Scalar(4));
    Mat d4(5, 5, CV_8UC1, Scalar(5));


    //int*取4字节，即d1第1行的前四个值(2,2,2,2,2),
    //每个值代表8位，k0*256^3+k1*256^2+k2*256^1+k3*256^0
    uchar* ptr = d1.ptr<uchar>(1);
    cout << "uchar* to int*: " << *(int*)ptr << endl;

    vector<Mat> v1{ d1,d2,d3,d4};
    AutoBuffer<uchar>buf(2 * sizeof(Mat*));
    //Mat*指针占8字节，可以看出一个8字节的类型K，(K*)buf.data()
    Mat** m = (Mat**)buf.data();
    m[0] = &v1[0];
    cout << "buffer stores Mat pointer: " << m[0][3] << endl;

    AutoBuffer<uchar>buf_(4);
    buf_[0] = 1;
    cout << "1*2^0 == int0: " << (int)*(uchar*)buf_.data() << endl;
    memset(buf_, 0, 4);

    buf_[1] = 1;
    cout << "1*2^8 == int1: " << *(int*)buf_.data() << endl;
    memset(buf_, 0, 4);

    buf_[2] = 1;
    cout << "1*2^16 == int2: " << *(int*)buf_.data() << endl;
    memset(buf_, 0, 4);

    buf_[3] = 1;
    cout << "1*2^24 == int3: " << *(int*)buf_.data() << endl;
    memset(buf_, 0, 4);
}
