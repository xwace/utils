static int idx{ 0 };
deque<pair<int,int>>win;
int maxSlideWin(int x) {
    idx++;

    if (idx == 100000 and !win.empty()) {
        for (auto& w : win) {
            w.first = w.first - 99000;
        }

        idx = idx - 99000;
    }

    if (win.size() < 10) {
        while (!win.empty() and win.back().second < x) {
            win.pop_back();
        }
        win.emplace_back(idx, x);
        return win.front().second;
    }

    while (!win.empty() and win.back().second < x) {
        win.pop_back();
    }
    win.emplace_back(idx, x);

    if (win.front().first <= idx - 10) {
        win.pop_front();
    }

    return win.front().second;
}

queue<int>mSlideWinQue;
int maxSlideWinVal(int input)
{
    queue<int> q;
    int maxVal{ INT_MIN };

    mSlideWinQue.push(input);
    if (mSlideWinQue.size() > 10)
        mSlideWinQue.pop();

    while (!mSlideWinQue.empty())
    {
        if (mSlideWinQue.front() > maxVal)
            maxVal = mSlideWinQue.front();
        q.push(mSlideWinQue.front());
        mSlideWinQue.pop();
    }

    swap(mSlideWinQue, q);
    return maxVal;
}
#include "unistd.h"
int main(){

    RNG rng;

    auto start = ::clock();
    for (size_t i = 1000; i > 0; i--)
    {
        usleep(30*1000);
        int val = rng.uniform(0,28);
        maxSlideWin(val);
    }
    auto interval = clock() - start;
    cout<<"new algorithm time: "<<interval<<endl;

    start = ::clock();
    for (size_t i = 1000; i > 0; i--)
    {
        usleep(30*1000);
        int val = rng.uniform(0,28);
        maxSlideWinVal(val);
    }
    interval = clock() - start;
    cout<<"old  algorithm time: "<<interval<<endl;

}
