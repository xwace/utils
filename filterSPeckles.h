template<typename T>
void mfilterSpeckles::myFilterSpeckles(cv::Mat& img, int newVal, int maxSpeckleSize, int maxDiff)
{
    int width = img.cols;
    int height = img.rows;
    int imgSize = width * height;
    int* pLabelBuf = (int*)malloc(sizeof(int) * imgSize);//标记值buffer
    Point2s* pPointBuf = (Point2s*)malloc(sizeof(short) * imgSize);//点坐标buffer
    uchar* pTypeBuf = (uchar*)malloc(sizeof(uchar) * imgSize);//blob判断标记buffer
    //初始化Labelbuffer
    int currentLabel = 0;
    memset(pLabelBuf, 0, sizeof(int) * imgSize);

    for (int i = 0; i < height; i++)
    {
        T* pData = img.ptr<T>(i);
        int* pLabel = pLabelBuf + width * i;
        for (int j = 0; j < width; j++)
        {
            if (pData[j] != newVal)
            {
                if (pLabel[j])
                {
                    if (pTypeBuf[pLabel[j]])
                    {
                        pData[j] = (T)newVal;
                    }
                }
                else
                {
                    Point2s* pWave = pPointBuf;
                    Point2s curPoint((T)j, (T)i);
                    currentLabel++;
                    int count = 0;
                    pLabel[j] = currentLabel;
                    while (pWave >= pPointBuf)
                    {
                        count++;
                        T* pCurPos = &img.at<T>(curPoint.y, curPoint.x);
                        T curValue = *pCurPos;
                        int* pCurLabel = pLabelBuf + width * curPoint.y + curPoint.x;
                        //bot
                        if (curPoint.y < height - 1 && !pCurLabel[+width] && pCurPos[+width] != newVal && abs(curValue - pCurPos[+width]) <= maxDiff)
                        {
                            pCurLabel[+width] = currentLabel;
                            *pWave++ = Point2s(curPoint.x, curPoint.y + 1);
                        }
                        //top
                        if (curPoint.y > 0 && !pCurLabel[-width] && pCurPos[-width] != newVal && abs(curValue - pCurPos[-width]) <= maxDiff)
                        {
                            pCurLabel[-width] = currentLabel;
                            *pWave++ = Point2s(curPoint.x, curPoint.y - 1);
                        }
                        //right
                        if (curPoint.x < width - 1 && !pCurLabel[+1] && pCurPos[+1] != newVal && abs(curValue - pCurPos[+1]) <= maxDiff)
                        {
                            pCurLabel[+1] = currentLabel;
                            *pWave++ = Point2s(curPoint.x + 1, curPoint.y);
                        }
                        //left
                        if (curPoint.x > 0 && !pCurLabel[-1] && pCurPos[-1] != newVal && abs(curValue - pCurPos[-1]) <= maxDiff)
                        {
                            pCurLabel[-1] = currentLabel;
                            *pWave++ = Point2s(curPoint.x - 1, curPoint.y);
                        }

                        --pWave;
                        curPoint = *pWave;
                    }

                    if (count <= maxSpeckleSize)
                    {
                        pTypeBuf[pLabel[j]] = 1;
                        pData[j] = (T)newVal;
                    }
                    else
                    {
                        pTypeBuf[pLabel[j]] = 0;
                    }
                }
            }
        }
    }

    cv::Mat label(img.size(), CV_32S, (int*)&pLabelBuf[0]);
    std::cout << label << std::endl;

    free(pLabelBuf);
    free(pPointBuf);
    free(pTypeBuf);
}

typedef cv::Point_<short> Point2s;

template <typename T> 
void mfilterSpeckles::filterSpecklesImpl(cv::Mat& img, int newVal, int maxSpeckleSize, int maxDiff, cv::Mat& _buf)
{
    using namespace cv;

    int width = img.cols, height = img.rows, npixels = width * height;
    size_t bufSize = npixels * (int)(sizeof(Point2s) + sizeof(int) + sizeof(uchar));
    if (!_buf.isContinuous() || _buf.empty() || _buf.cols * _buf.rows * _buf.elemSize() < bufSize)
        _buf.create(1, (int)bufSize, CV_8U);

    uchar* buf = _buf.ptr();
    int i, j, dstep = (int)(img.step / sizeof(T));
    int* labels = (int*)buf;
    buf += npixels * sizeof(labels[0]);
    Point2s* wbuf = (Point2s*)buf;
    buf += npixels * sizeof(wbuf[0]);
    uchar* rtype = (uchar*)buf;
    int curlabel = 0;

    // clear out label assignments
    memset(labels, 0, npixels * sizeof(labels[0]));

    for (i = 0; i < height; i++)
    {
        T* ds = img.ptr<T>(i);
        int* ls = labels + width * i;

        for (j = 0; j < width; j++)
        {
            if (ds[j] != newVal)   // not a bad disparity
            {
                if (ls[j])     // has a label, check for bad label
                {
                    if (rtype[ls[j]]) // small region, zero out disparity
                        ds[j] = (T)newVal;
                }
                // no label, assign and propagate
                else
                {
                    Point2s* ws = wbuf; // initialize wavefront
                    Point2s p((short)j, (short)i);  // current pixel
                    curlabel++; // next label
                    int count = 0;  // current region size
                    ls[j] = curlabel;

                    // wavefront propagation
                    while (ws >= wbuf) // wavefront not empty
                    {
                        count++;
                        // put neighbors onto wavefront
                        T* dpp = &img.at<T>(p.y, p.x); //current pixel value
                        T dp = *dpp;
                        int* lpp = labels + width * p.y + p.x; //current label value

                        //bot
                        if (p.y < height - 1 && !lpp[+width] && dpp[+dstep] != newVal && std::abs(dp - dpp[+dstep]) <= maxDiff)
                        {
                            lpp[+width] = curlabel;
                            *ws++ = Point2s(p.x, p.y + 1);
                        }
                        //top
                        if (p.y > 0 && !lpp[-width] && dpp[-dstep] != newVal && std::abs(dp - dpp[-dstep]) <= maxDiff)
                        {
                            lpp[-width] = curlabel;
                            *ws++ = Point2s(p.x, p.y - 1);
                        }
                        //right
                        if (p.x < width - 1 && !lpp[+1] && dpp[+1] != newVal && std::abs(dp - dpp[+1]) <= maxDiff)
                        {
                            lpp[+1] = curlabel;
                            *ws++ = Point2s(p.x + 1, p.y);
                        }
                        //left
                        if (p.x > 0 && !lpp[-1] && dpp[-1] != newVal && std::abs(dp - dpp[-1]) <= maxDiff)
                        {
                            lpp[-1] = curlabel;
                            *ws++ = Point2s(p.x - 1, p.y);
                        }


                        // pop most recent and propagate
                        // NB: could try least recent, maybe better convergence
                        p = *--ws;
                    }

                    // assign label type
                    if (count <= maxSpeckleSize)   // speckle region
                    {
                        rtype[ls[j]] = 1;   // small region label
                        ds[j] = (T)newVal;
                    }
                    else
                        rtype[ls[j]] = 0;   // large region label
                }
            }
        }
    }

    cv::Mat label(width, height, CV_32S, (int*)&labels[0]);
    std::cout << label << std::endl;
}
