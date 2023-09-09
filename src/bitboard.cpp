#include "bitboard.h"
#include "constants.h"
#include "board_declaration.h"
#include <stdint.h>
#include <vector>
#include <string>

std::vector<std::string> splitString(std::string str, char delim)
{
    std::vector<std::string> result;
    std::string buf = "";
    int i = 0;
    while (i < str.length())
    {
        if (str[i] != delim)
            buf += str[i];
        else if (buf.length() > 0)
        {
            result.push_back(buf);
            buf = "";
        }
        i++;
    }
    if (!buf.empty())
        result.push_back(buf);
    return result;
}