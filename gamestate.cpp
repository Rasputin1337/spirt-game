#include "gamestate.h"

bool GameState::isValidDifficulty(int colorCount)
{
    return colorCount >= 2 && colorCount <= 4;
}
