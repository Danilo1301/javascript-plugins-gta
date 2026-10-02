#include "Input.h"

#include "aml-psdk/game_sa/base/Timer.h"
#include "aml-psdk/game_sa/Events.h"
#include "aml-psdk/game_sa/engine/OS.h"

std::unordered_map<int, Touch> Input::_touches;

void Input::Initialize()
{
    Events::touchScreenEvent += [](int actionType, int trackNum, int x, int y) { OnTouchEvent(actionType, trackNum, x, y); };
}

void Input::OnTouchEvent(int actionType, int trackNum, int x, int y)
{
    switch (actionType)
    {
        case 2: // DOWN
        {
            Touch touch;
            touch.x = x;
            touch.y = y;
            touch.startTime = CTimer::GetTimeInMS();

            _touches[trackNum] = touch;
            break;
        }

        case 3: // MOVE
        {
            auto it = _touches.find(trackNum);

            if (it != _touches.end())
            {
                it->second.x = x;
                it->second.y = y;
            }

            break;
        }

        case 1: // UP
        {
            _touches.erase(trackNum);
            break;
        }
    }
}

bool Input::IsTouching(int trackNum)
{
    return _touches.find(trackNum) != _touches.end();
}

bool Input::GetTouch(int trackNum, int& x, int& y)
{
    auto it = _touches.find(trackNum);

    if (it == _touches.end()) return false;

    x = it->second.x;
    y = it->second.y;

    return true;
}

bool Input::IsTouchPointPressed(int point, uint32_t minTime)
{
    if (point < 1 || point > 9) return false;

    const int screenWidth = OS_ScreenGetWidth();
    const int screenHeight = OS_ScreenGetHeight();

    const int column = (point - 1) / 3;
    const int row = (point - 1) % 3;

    const int minX = column * screenWidth / 3;
    const int maxX = (column + 1) * screenWidth / 3;

    const int minY = row * screenHeight / 3;
    const int maxY = (row + 1) * screenHeight / 3;

    const uint32_t now = CTimer::GetTimeInMS();

    for (const auto& [trackNum, touch] : _touches)
    {
        if (touch.x < minX || touch.x >= maxX || touch.y < minY || touch.y >= maxY) { continue; }

        if (now - touch.startTime >= minTime) return true;
    }

    return false;
}
