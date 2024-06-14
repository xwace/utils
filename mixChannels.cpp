#pragma once

#include"opencv2/opencv.hpp"
#include <cstdint>
using namespace cv;

namespace mycv
{
    template<typename T> static void
        mixChannels_(const T** src, const int* sdelta,
            T** dst, const int* ddelta,
            int len, int npairs)
    {
        int i, k;
        for (k = 0; k < npairs; k++)
        {
            const T* s = src[k];//取出源图指定通道指针
            T* d = dst[k];//目标通道指针
            //ds是当前点对应图像的通道数
            //s+ds表示同一通道的下一个点point(i,j)->point(i+1,j)
            int ds = sdelta[k], dd = ddelta[k];
            if (s)
            {
                //s+=ds*2,一次取两个数据做循环展开
                for (i = 0; i <= len - 2; i += 2, s += ds * 2, d += dd * 2)
                {
                    T t0 = s[0], t1 = s[ds];
                    d[0] = t0; d[dd] = t1;//实现
                }
                if (i < len)
                    d[0] = s[0];
            }
            else
            {
                for (i = 0; i <= len - 2; i += 2, d += dd * 2)
                    d[0] = d[dd] = 0;
                if (i < len)
                    d[0] = 0;
            }
        }
    }

    static void mixChannels8u(const uchar** src, const int* sdelta,
        uchar** dst, const int* ddelta,
        int len, int npairs)
    {
        mixChannels_(src, sdelta, dst, ddelta, len, npairs);
    }

    static void mixChannels16u(const ushort** src, const int* sdelta,
        ushort** dst, const int* ddelta,
        int len, int npairs)
    {
        mixChannels_(src, sdelta, dst, ddelta, len, npairs);
    }

    static void mixChannels32s(const int** src, const int* sdelta,
        int** dst, const int* ddelta,
        int len, int npairs)
    {
        mixChannels_(src, sdelta, dst, ddelta, len, npairs);
    }

    static void mixChannels64s(const int64** src, const int* sdelta,
        int64** dst, const int* ddelta,
        int len, int npairs)
    {
        mixChannels_(src, sdelta, dst, ddelta, len, npairs);
    }

    typedef void (*MixChannelsFunc)(const uchar** src, const int* sdelta,
        uchar** dst, const int* ddelta, int len, int npairs);

    static MixChannelsFunc getMixchFunc(int depth)
    {
        static MixChannelsFunc mixchTab[] =
        {
            (MixChannelsFunc)mixChannels8u, (MixChannelsFunc)mixChannels8u, (MixChannelsFunc)mixChannels16u,
            (MixChannelsFunc)mixChannels16u, (MixChannelsFunc)mixChannels32s, (MixChannelsFunc)mixChannels32s,
            (MixChannelsFunc)mixChannels64s, 0
        };

        return mixchTab[depth];
    }


void mixChannels(const Mat* src, size_t nsrcs, Mat* dst, size_t ndsts, const int* fromTo, size_t npairs)
{
    if (npairs == 0)
        return;
    CV_Assert(src && nsrcs > 0 && dst && ndsts > 0 && fromTo && npairs > 0);

    size_t i, j, k, esz1 = dst[0].elemSize1();
    int depth = dst[0].depth();

    AutoBuffer<uchar> buf((nsrcs + ndsts + 1) * (sizeof(Mat*) + sizeof(uchar*)) + npairs * (sizeof(uchar*) * 2 + sizeof(int) * 6));
    const Mat** arrays = (const Mat**)(uchar*)buf.data();//Mat*占buf的4或8字节
    uchar** ptrs = (uchar**)(arrays + nsrcs + ndsts);
    const uchar** srcs = (const uchar**)(ptrs + nsrcs + ndsts + 1);
    uchar** dsts = (uchar**)(srcs + npairs);
    int* tab = (int*)(dsts + npairs);//tab存放npair与图像关系
    int* sdelta = (int*)(tab + npairs * 4), * ddelta = sdelta + npairs;

    for (i = 0; i < nsrcs; i++)
        arrays[i] = &src[i];
    for (i = 0; i < ndsts; i++)
        arrays[i + nsrcs] = &dst[i];
    ptrs[nsrcs + ndsts] = 0;//疑似初始化错误

    for (i = 0; i < npairs; i++)
    {
        int i0 = fromTo[i * 2], i1 = fromTo[i * 2 + 1];
        if (i0 >= 0)
        {
            //如i0=7，src由三张3通道图组成，则7-3-3=1(i0),tab[4i]=j=2;表示io对应第二张源图的第一通道
            for (j = 0; j < nsrcs; i0 -= src[j].channels(), j++)
                if (i0 < src[j].channels())
                    break;
            CV_Assert(j < nsrcs && src[j].depth() == depth);
            //i0为在原图+目标图所有通道中的下标，需转为第n图第n通道
            //tab[4i]表示i0通道对应第n张源图,tab[4i+1]存放io通道对应第n张源图的第n通道
            tab[i * 4] = (int)j; tab[i * 4 + 1] = (int)(i0 * esz1);
            sdelta[i] = src[j].channels();
        }
        else
        {
            tab[i * 4] = (int)(nsrcs + ndsts); tab[i * 4 + 1] = 0;
            sdelta[i] = 0;
        }

        for (j = 0; j < ndsts; i1 -= dst[j].channels(), j++)
            if (i1 < dst[j].channels())
                break;
        CV_Assert(i1 >= 0 && j < ndsts && dst[j].depth() == depth);
        //tab[4i+2]表示i1通道对应第n张目标图,tab[4i+3]对应第n张目标图的第n通道
        //ptrs将源图+目标图统一存放在数组中，nsrcs张源图，ndsts目标图，所以i1的转为第n目标图后，还要加上nsrcs，转为对应ptrs数组中的目标图下标
        tab[i * 4 + 2] = (int)(j + nsrcs); tab[i * 4 + 3] = (int)(i1 * esz1);
        ddelta[i] = dst[j].channels();
    }

    

    #define BLOCK_SIZE 1024
    NAryMatIterator it(arrays, ptrs, (int)(nsrcs + ndsts));
    int total = (int)it.size, blocksize = std::min(total, (int)((BLOCK_SIZE + esz1 - 1) / esz1));

    MixChannelsFunc func = getMixchFunc(depth);

    for (i = 0; i < it.nplanes; i++, ++it)
    {
        for (k = 0; k < npairs; k++)
        {
            srcs[k] = ptrs[tab[k * 4]] + tab[k * 4 + 1];
            dsts[k] = ptrs[tab[k * 4 + 2]] + tab[k * 4 + 3];
        }

        for (int t = 0; t < total; t += blocksize)
        {
            int bsz = std::min(total - t, blocksize);
            func(srcs, sdelta, dsts, ddelta, bsz, (int)npairs);

            if (t + blocksize < total)
                for (k = 0; k < npairs; k++)
                {
                    srcs[k] += blocksize * sdelta[k] * esz1;
                    dsts[k] += blocksize * ddelta[k] * esz1;
                }
        }
    }
}

void mixChannels(InputArrayOfArrays src, InputOutputArrayOfArrays dst,
    const std::vector<int>& fromTo)
{
    if (fromTo.empty())
        return;

    bool src_is_mat = src.kind() != _InputArray::STD_VECTOR_MAT &&
        src.kind() != _InputArray::STD_ARRAY_MAT &&
        src.kind() != _InputArray::STD_VECTOR_VECTOR &&
        src.kind() != _InputArray::STD_VECTOR_UMAT;
    bool dst_is_mat = dst.kind() != _InputArray::STD_VECTOR_MAT &&
        dst.kind() != _InputArray::STD_ARRAY_MAT &&
        dst.kind() != _InputArray::STD_VECTOR_VECTOR &&
        dst.kind() != _InputArray::STD_VECTOR_UMAT;
    int i;
    int nsrc = src_is_mat ? 1 : (int)src.total();
    int ndst = dst_is_mat ? 1 : (int)dst.total();

    CV_Assert(fromTo.size() % 2 == 0 && nsrc > 0 && ndst > 0);
    cv::AutoBuffer<Mat> _buf(nsrc + ndst);
    Mat* buf = _buf.data();
    for (i = 0; i < nsrc; i++)
        buf[i] = src.getMat(src_is_mat ? -1 : i);
    for (i = 0; i < ndst; i++)
        buf[nsrc + i] = dst.getMat(dst_is_mat ? -1 : i);
    //输入图像为vector<Mat>类型
    mycv::mixChannels(&buf[0], nsrc, &buf[nsrc], ndst, &fromTo[0], fromTo.size() / 2);
}

void mixChannels(InputArrayOfArrays src, InputOutputArrayOfArrays dst,
    const int* fromTo, size_t npairs)
{
    
    if (npairs == 0 || fromTo == NULL)
        return;

        bool src_is_mat = src.kind() != _InputArray::STD_VECTOR_MAT &&
        src.kind() != _InputArray::STD_ARRAY_MAT &&
        src.kind() != _InputArray::STD_VECTOR_VECTOR &&
        src.kind() != _InputArray::STD_VECTOR_UMAT;
    bool dst_is_mat = dst.kind() != _InputArray::STD_VECTOR_MAT &&
        dst.kind() != _InputArray::STD_ARRAY_MAT &&
        dst.kind() != _InputArray::STD_VECTOR_VECTOR &&
        dst.kind() != _InputArray::STD_VECTOR_UMAT;
    int i;
    int nsrc = src_is_mat ? 1 : (int)src.total();
    int ndst = dst_is_mat ? 1 : (int)dst.total();

    CV_Assert(nsrc > 0 && ndst > 0);
    cv::AutoBuffer<Mat> _buf(nsrc + ndst);
    Mat* buf = _buf.data();
    for (i = 0; i < nsrc; i++)
        buf[i] = src.getMat(src_is_mat ? -1 : i);
    for (i = 0; i < ndst; i++)
        buf[nsrc + i] = dst.getMat(dst_is_mat ? -1 : i);
    mycv::mixChannels(&buf[0], nsrc, &buf[nsrc], ndst, fromTo, npairs);
}

}

