点云数据：
1 2 3 
4 5 6
7 8 9

void test()
{
    
    std::ifstream filein("../build/point.txt");
    std::string str;
    while (getline(filein,str))
    {
        std::stringstream ss(str);
        float x,y,z;
        while (!ss.eof())
        {
            ss>>x>>y>>z;
        }    
    }
}
