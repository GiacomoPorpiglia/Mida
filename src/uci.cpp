#include "uci.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include "make_move.h"
#include "perft.h"
#include "hashing.h"
#include "search.h"
#include "game_constants.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/select.h>
#include <time.h>
#endif

// exit from engine flag
int quit = 0;

// UCI "movestogo" command moves counter
int movestogo = 30;

// UCI "movetime" command time counter
int movetime = -1;

// UCI "time" command holder (ms)
int my_time = -1;

// UCI "inc" command's time increment holder
int inc = 0;

// UCI "starttime" command time holder
int starttime = 0;

// UCI "stoptime" command time holder
int stoptime = 0;

// variable to flag time control availability
int timeset = 0;

// variable to flag when the time is up
int stopped = 0;

/*
static inline int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME system_time;
    FILETIME file_time;
    uint64_t time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}
*/
int get_time_ms()
{
#ifdef _WIN32
    return GetTickCount();
#else
    timeval time_value;
    gettimeofday(&time_value, NULL);
    return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
#endif
}


int input_waiting()
{
#ifndef WIN32
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    select(16, &readfds, 0, 0, &tv);

    return (FD_ISSET(fileno(stdin), &readfds));
#else
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;

    if (!init)
    {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe)
        {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }

    if (pipe)
    {
        if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL))
            return 1;
        return dw;
    }

    else
    {
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }

#endif
}

void read_input()
{
    // bytes to read holder
    int bytes;

    // GUI/user input
    char input[256] = "", *endc;

    // "listen" to STDIN

    /*
    if (input_waiting())
    {
        // tell engine to stop calculating
        stopped = 1;

        // loop to read bytes from STDIN
        do
        {
            // read bytes from STDIN
            bytes=read(fileno(stdin), input, 256);
        }

        // until bytes available
        while (bytes < 0);

        // searches for the first occurrence of '\n'
        endc = strchr(input,'\n');

        // if found new line set value at pointer to 0
        if (endc) *endc=0;

        // if input is available
        if (strlen(input) > 0)
        {
            // match UCI "quit" command
            if (!strncmp(input, "quit", 4))
            {
                // tell engine to terminate exacution
                quit = 1;
            }

            // // match UCI "stop" command
            else if (!strncmp(input, "stop", 4))    {
                // tell engine to terminate exacution
                quit = 1;
            }
        }
    }
    */
}

void communicate()
{
    // if time is up break here
    if (timeset == 1 && get_time_ms() > stoptime)
    {
        // tell engine to stop calculating
        stopped = 1;
    }

    // read GUI input
    read_input();
}

static inline MOVE parse_move(char const *move_string)
{
    movesList moveList;
    // create move list instance
    board.calculateMoves(board.colorToMove, &moveList);

    // parse source square
    int source_square = (move_string[0] - 'a') + (move_string[1] - '1') * 8;

    // parse target square
    int target_square = (move_string[2] - 'a') + (move_string[3] - '1') * 8;

    // loop over the moves within a move list
    for (int count = 0; count < moveList.count; count++)
    {
        MOVE move = moveList.moves[count];
        // make sure source & target squares are available within the generated move
        if (source_square == getSquareFrom(move) && target_square == getSquareTo(move))
        {
            // init promoted piece
            int promoted_piece = getNewPieceType(move);

            // promoted piece is available
            if (promoted_piece != board.allPieces[source_square])
            {
                // promoted to queen
                if ((promoted_piece == 1) && move_string[4] == 'q')
                    // return legal move
                    return move;

                // promoted to rook
                else if ((promoted_piece == 2) && move_string[4] == 'r')
                    // return legal move
                    return move;

                // promoted to bishop
                else if ((promoted_piece == 3) && move_string[4] == 'b')
                    // return legal move
                    return move;

                // promoted to knight
                else if ((promoted_piece == 4) && move_string[4] == 'n')
                    // return legal move
                    return move;

                // continue the loop on possible wrong promotions (e.g. "e7e8f")
                continue;
            }

            // return legal move
            return move;
        }
    }

    // return illegal move
    return ILLEGAL_MOVE;
}

void parse_position(char const *command)
{
    // shift pointer to the right where next token begins
    command += 9;

    // init pointer to the current character in the command string
    char const *current_char = command;

    // parse UCI "startpos" command
    if (strncmp(command, "startpos", 8) == 0)
    {
        // init chess board with start position
        board.loadFenPosition(startpos);
        hash_key = generate_hash_key();
    }

    // parse UCI "fen" command
    else
    {
        // make sure "fen" command is available within command string
        current_char = strstr(command, "fen");

        // if no "fen" command is available within command string
        if (current_char == NULL)
        {
            // init chess board with start position
            board.loadFenPosition(startpos);
            hash_key = generate_hash_key();
        }
        // found "fen" substring
        else
        {
            // shift pointer to the right where next token begins
            current_char += 4;
            // init chess board with position from FEN string
            board.loadFenPosition(current_char);
            hash_key = generate_hash_key();
        }
    }

    // parse moves after position
    current_char = strstr(command, "moves");

    // moves available

    if (current_char != NULL)
    {
        // shift pointer to the right where next token begins
        current_char += 6;

        // loop over moves within a move string
        while (*current_char)
        {
            // parse next move
            MOVE move = parse_move(current_char);
            // if no more moves
            if (move == ILLEGAL_MOVE)
            {
                // break out of the loop
                break;
            }
            repetition_index++;
            repetition_table[repetition_index] = hash_key;
            // make move on the chess board
            playMove(move);
            // move current character mointer to the end of current move
            while (*current_char && *current_char != ' ')
                current_char++;

            // go to the next move
            current_char++;
        }

        printf("%s\n", current_char);
    }
}

void reset_time_control()
{
    quit = 0;
    movestogo = 30;
    movetime = -1;
    my_time = -1;
    inc = 0;
    starttime = 0;
    stoptime = 0;
    timeset = 0;
    stopped = 0;
}

static inline void parse_go(char *command)
{

    reset_time_control();

    // init parameters
    int depth = -1;

    // init argument
    char *argument = NULL;

    if (argument = strstr(command, "perft"))
    {
        depth = atoi(argument + 6);
        auto start = chrono::steady_clock::now();
        int totalMoves = perft(depth, depth);
        auto end = chrono::steady_clock::now();
        printf("Total nodes: %d\n", totalMoves);
        printf("Time (ms): %d\n", chrono::duration_cast<chrono::milliseconds>(end - start).count());
        clearTranspositionTable();
    }
    else
    {
        // infinite search
        if ((argument = strstr(command, "infinite")))
        {
        }

        // match UCI "binc" command
        if ((argument = strstr(command, "binc")) && board.colorToMove == 0)
            // parse black time increment
            inc = atoi(argument + 5);

        // match UCI "winc" command
        if ((argument = strstr(command, "winc")) && board.colorToMove == 1)
            // parse white time increment
            inc = atoi(argument + 5);

        // match UCI "wtime" command
        if ((argument = strstr(command, "wtime")) && board.colorToMove == 1)
            // parse white time limit
            my_time = atoi(argument + 6);

        // match UCI "btime" command
        if ((argument = strstr(command, "btime")) && board.colorToMove == 0)
            // parse black time limit
            my_time = atoi(argument + 6);

        // match UCI "movestogo" command
        if ((argument = strstr(command, "movestogo")))
            // parse number of moves to go
            movestogo = atoi(argument + 10);

        // match UCI "movetime" command
        if ((argument = strstr(command, "movetime")))
            // parse amount of time allowed to spend to make a move
            movetime = atoi(argument + 9);

        // match UCI "depth" command
        if ((argument = strstr(command, "depth")))
            // parse search depth
            depth = atoi(argument + 6);

        // if move time is not available
        if (movetime != -1)
        {
            // set time equal to move time
            my_time = movetime;

            // set moves to go to 1
            movestogo = 1;
        }

        // init start time
        starttime = get_time_ms();

        // init search depth
        depth = depth;

        // if time control is available
        if (my_time != -1)
        {
            // flag we're playing with time control
            timeset = 1;

            // set up timing
            my_time /= movestogo;

            // "illegal" (empty) move bug fix
            if (my_time > 1500)
                my_time -= 50;

            // init stoptime
            stoptime = starttime + my_time + inc;
        }

        // if depth is not available
        if (depth == -1)
            // set depth to 64 plies (takes ages to complete...)
            depth = 64;

        // print debug info
        // printf("time:%d start:%d stop:%d depth:%d timeset:%d\n", my_time, starttime, stoptime, depth, timeset);
        // search position
        search_position(depth);
    }
}

void uci_loop()
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char input[2000];

    printf("id name MIDA 2.2\n");
    printf("id author Giacomo Porpiglia\n");
    printf("uciok\n");

    while (1)
    {
        memset(input, 0, sizeof(input));

        fflush(stdout);

        if (!fgets(input, 2000, stdin))
            continue;

        else if (input[0] == '\n')
            continue;

        else if (strncmp(input, "isready", 7) == 0)
        {
            printf("readyok\n");
        }

        else if (strncmp(input, "position", 8) == 0)
        {
            parse_position(input);
        }

        else if (strncmp(input, "ucinewgame", 10) == 0)
        {
            parse_position("position startpos");
            clearTranspositionTable();
        }

        else if (strncmp(input, "go", 2) == 0)
        {
            parse_go(input);
        }

        else if(!strncmp(input, "setoption name Hash value ", 26)) {
            int currentHashSize = hash_table_size;
            int newHashSize;
            sscanf(input,"%*s %*s %*s %*s %d",&newHashSize);
			if(newHashSize < 4) newHashSize = 4;
			if(newHashSize > MAX_HASH) newHashSize = MAX_HASH;

            if(newHashSize!=currentHashSize) {
                hash_table_size = newHashSize;
                free(transposition_table);
                init_hash_table();
            }
        }

        else if (strncmp(input, "quit", 4) == 0)
        {
            free(transposition_table);
            break;
        }

        else if (strncmp(input, "uci", 3) == 0)
        {
            printf("id name MIDA 2.2\n");
            printf("id author Giacomo Porpiglia\n");
            printf("setoption name Hash type spin default 64 min 4 max 1024\n");

            printf("uciok\n");
        }
    }
}