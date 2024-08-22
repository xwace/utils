float medianPartition(int* ofs,int* vals, int a, int b)
{
    int k, a0 = a, b0 = b;
    int middle = (a + b) / 2;

    while (b > a)
    {
        int i0 = a, i1 = (a + b) / 2, i2 = b;
        float v0 = vals[ofs[i0]], v1 = vals[ofs[i1]], v2 = vals[ofs[i2]];
        int ip = v0 < v1 ? (v1 < v2 ? i1 : v0 < v2 ? i2 : i0) :
            v0 < v2 ? (v1 == v0 ? i2 : i0) : (v1 < v2 ? i2 : i1);
        float pivot = vals[ofs[ip]];
        std::swap(ofs[ip], ofs[i2]);

        for (i1 = i0, i0--; i1 <= i2; i1++)
        {
            //小值放置左边，大值右边
            if (vals[ofs[i1]] <= pivot)
            {
                //i0指小值队列最右
                i0++;
                std::swap(ofs[i0], ofs[i1]);
                
                for (size_t i = a; i <= b; i++)
                {
                    cout << vals[ofs[i]] << " ";
                }cout << endl;
            }
            
        }
        cout << "one loop done, cut location: " <<i0<< endl;
        getchar();

        if (i0 == middle)
            break;

        //i0大于一半，这次以pivot二分的两堆数据，小堆偏多，继续分堆
        //如00000000011111 中间的值在0堆里面，对0堆再二分
        if (i0 > middle)
            b = i0 - (b == i0);
        else
            a = i0;//如000001111111111，a对1堆再拆分
    }

    float pivot = vals[ofs[middle]];
    cout << "mid: " << ofs[middle] << endl;

    return vals[ofs[middle]];
}

int main(){
    int ofs[5] = { 0,1,2,3,4 };
    int vals[5] = { 17,11,2,13,4 };
    medianPartition(ofs, vals, 0, 4);
}
