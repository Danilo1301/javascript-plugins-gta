#pragma once

#include <cstdint>
#include <unordered_map>

struct Touch
{
    int x;
    int y;
    uint32_t startTime;
};

class Input
{
public:
    static void Initialize();

    static bool IsTouching(int trackNum);
    static bool GetTouch(int trackNum, int& x, int& y);

    static bool IsTouchPointPressed(int point, uint32_t minTime);

private:
    static void OnTouchEvent(int actionType, int trackNum, int x, int y);

    static std::unordered_map<int, Touch> _touches;
};