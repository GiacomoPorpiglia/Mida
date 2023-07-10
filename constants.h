#ifndef ENGINE_CONSTANTS_H
#define ENGINE_CONSTANTS_H
#define CH char
#define MOVE uint16_t
#define ILLEGAL_MOVE 0
// transposition table hash flags
#define HASH_FLAG_EXACT 0
#define HASH_FLAG_ALPHA 1
#define HASH_FLAG_BETA 2

// bounds for the range of mating scores
// [-infinity, -mate_value ... -mate_score, normal_scores, ...mate_score, ...mate value, infinity]
#define MATE_SCORE 48000
#define MATE_VALUE 49000

#define HASH_TABLE_SIZE 8000000 // 200 MB
#define NULL_HASH_ENTRY 100000  // to make sure it goes outside the alpha-beta window

#define WHITE 1
#define BLACK 0

//0, 1230, 620, 420, 410, 100
const int pieceValues[6] = {0, 1120, 605, 410, 400, 100};

const int K = 0;
const int Q = 1;
const int R = 2;
const int B = 3;
const int N = 4;
const int P = 5;
const int ALL_PIECES = 6;

#define inf 50000

#define max_ply 64

#define WIN32_LEAN_AND_MEAN
#define startpos "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <stdint.h>
#include <bit>

typedef struct
{
    MOVE moves[256];
    int count;
} movesList;

// not a constant. it is here because it is needed everywhere, and this file is imported everywhere.
//
//  [side][piece type (also all pieces, index = 6)]
uint64_t pieces_bb[2][7];

/*
knight attack table, with a bitboard for each square
the knight can be in, precomputed for faster performance
*/
uint64_t knight_attacks[64] = {
    132096,
    329728,
    659712,
    1319424,
    2638848,
    5277696,
    10489856,
    4202496,
    33816580,
    84410376,
    168886289,
    337772578,
    675545156,
    1351090312,
    2685403152,
    1075839008,
    8657044482,
    21609056261,
    43234889994,
    86469779988,
    172939559976,
    345879119952,
    687463207072,
    275414786112,
    2216203387392,
    5531918402816,
    11068131838464,
    22136263676928,
    44272527353856,
    88545054707712,
    175990581010432,
    70506185244672,
    567348067172352,
    1416171111120896,
    2833441750646784,
    5666883501293568,
    11333767002587136,
    22667534005174272,
    45053588738670592,
    18049583422636032,
    145241105196122112,
    362539804446949376,
    725361088165576704,
    1450722176331153408,
    2901444352662306816,
    5802888705324613632,
    11533718717099671552,
    4620693356194824192,
    288234782788157440,
    576469569871282176,
    1224997833292120064,
    2449995666584240128,
    4899991333168480256,
    9799982666336960512,
    1152939783987658752,
    2305878468463689728,
    1128098930098176,
    2257297371824128,
    4796069720358912,
    9592139440717824,
    19184278881435648,
    38368557762871296,
    4679521487814656,
    9077567998918656};

/*
king attack table, with a bitboard for each square
the knight can be in, precomputed for faster performance
*/
uint64_t king_attacks[64] = {
    770,
    1797,
    3594,
    7188,
    14376,
    28752,
    57504,
    49216,
    197123,
    460039,
    920078,
    1840156,
    3680312,
    7360624,
    14721248,
    12599488,
    50463488,
    117769984,
    235539968,
    471079936,
    942159872,
    1884319744,
    3768639488,
    3225468928,
    12918652928,
    30149115904,
    60298231808,
    120596463616,
    241192927232,
    482385854464,
    964771708928,
    825720045568,
    3307175149568,
    7718173671424,
    15436347342848,
    30872694685696,
    61745389371392,
    123490778742784,
    246981557485568,
    211384331665408,
    846636838289408,
    1975852459884544,
    3951704919769088,
    7903409839538176,
    15806819679076352,
    31613639358152704,
    63227278716305408,
    54114388906344448,
    216739030602088448,
    505818229730443264,
    1011636459460886528,
    2023272918921773056,
    4046545837843546112,
    8093091675687092224,
    16186183351374184448,
    13853283560024178688,
    144959613005987840,
    362258295026614272,
    724516590053228544,
    1449033180106457088,
    2898066360212914176,
    5796132720425828352,
    11592265440851656704,
    4665729213955833856};

/*
white pawn attack table, with a bitboard for each square
the pawn can be in, precomputed for faster performance
*/
uint64_t white_pawn_attacks[64] = {
    512,
    1280,
    2560,
    5120,
    10240,
    20480,
    40960,
    16384,
    131072,
    327680,
    655360,
    1310720,
    2621440,
    5242880,
    10485760,
    4194304,
    33554432,
    83886080,
    167772160,
    335544320,
    671088640,
    1342177280,
    2684354560,
    1073741824,
    8589934592,
    21474836480,
    42949672960,
    85899345920,
    171798691840,
    343597383680,
    687194767360,
    274877906944,
    2199023255552,
    5497558138880,
    10995116277760,
    21990232555520,
    43980465111040,
    87960930222080,
    175921860444160,
    70368744177664,
    562949953421312,
    1407374883553280,
    2814749767106560,
    5629499534213120,
    11258999068426240,
    22517998136852480,
    45035996273704960,
    18014398509481984,
    144115188075855872,
    360287970189639680,
    720575940379279360,
    1441151880758558720,
    2882303761517117440,
    5764607523034234880,
    11529215046068469760,
    4611686018427387904,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

/*
black pawn attack table, with a bitboard for each square
the pawn can be in, precomputed for faster performance
*/
uint64_t black_pawn_attacks[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    5,
    10,
    20,
    40,
    80,
    160,
    64,
    512,
    1280,
    2560,
    5120,
    10240,
    20480,
    40960,
    16384,
    131072,
    327680,
    655360,
    1310720,
    2621440,
    5242880,
    10485760,
    4194304,
    33554432,
    83886080,
    167772160,
    335544320,
    671088640,
    1342177280,
    2684354560,
    1073741824,
    8589934592,
    21474836480,
    42949672960,
    85899345920,
    171798691840,
    343597383680,
    687194767360,
    274877906944,
    2199023255552,
    5497558138880,
    10995116277760,
    21990232555520,
    43980465111040,
    87960930222080,
    175921860444160,
    70368744177664,
    562949953421312,
    1407374883553280,
    2814749767106560,
    5629499534213120,
    11258999068426240,
    22517998136852480,
    45035996273704960,
    18014398509481984,
};

// file masks for each square, where the hot bits of the mask represent the highlighted file
const uint64_t file_masks[64] = {
    72340172838076673,
    144680345676153346,
    289360691352306692,
    578721382704613384,
    1157442765409226768,
    2314885530818453536,
    4629771061636907072,
    9259542123273814144,
    72340172838076673,
    144680345676153346,
    289360691352306692,
    578721382704613384,
    1157442765409226768,
    2314885530818453536,
    4629771061636907072,
    9259542123273814144,
    72340172838076673,
    144680345676153346,
    289360691352306692,
    578721382704613384,
    1157442765409226768,
    2314885530818453536,
    4629771061636907072,
    9259542123273814144,
    72340172838076673,
    144680345676153346,
    289360691352306692,
    578721382704613384,
    1157442765409226768,
    2314885530818453536,
    4629771061636907072,
    9259542123273814144,
    72340172838076673,
    144680345676153346,
    289360691352306692,
    578721382704613384,
    1157442765409226768,
    2314885530818453536,
    4629771061636907072,
    9259542123273814144,
    72340172838076673,
    144680345676153346,
    289360691352306692,
    578721382704613384,
    1157442765409226768,
    2314885530818453536,
    4629771061636907072,
    9259542123273814144,
    72340172838076673,
    144680345676153346,
    289360691352306692,
    578721382704613384,
    1157442765409226768,
    2314885530818453536,
    4629771061636907072,
    9259542123273814144,
    72340172838076673,
    144680345676153346,
    289360691352306692,
    578721382704613384,
    1157442765409226768,
    2314885530818453536,
    4629771061636907072,
    9259542123273814144};

// rank masks for each square, where the hot bits of the mask represent the highlighted rank
const uint64_t rank_masks[64] = {
    255,
    255,
    255,
    255,
    255,
    255,
    255,
    255,
    65280,
    65280,
    65280,
    65280,
    65280,
    65280,
    65280,
    65280,
    16711680,
    16711680,
    16711680,
    16711680,
    16711680,
    16711680,
    16711680,
    16711680,
    4278190080,
    4278190080,
    4278190080,
    4278190080,
    4278190080,
    4278190080,
    4278190080,
    4278190080,
    1095216660480,
    1095216660480,
    1095216660480,
    1095216660480,
    1095216660480,
    1095216660480,
    1095216660480,
    1095216660480,
    280375465082880,
    280375465082880,
    280375465082880,
    280375465082880,
    280375465082880,
    280375465082880,
    280375465082880,
    280375465082880,
    71776119061217280,
    71776119061217280,
    71776119061217280,
    71776119061217280,
    71776119061217280,
    71776119061217280,
    71776119061217280,
    71776119061217280,
    18374686479671623680,
    18374686479671623680,
    18374686479671623680,
    18374686479671623680,
    18374686479671623680,
    18374686479671623680,
    18374686479671623680,
    18374686479671623680};

// rank indices for each position
const uint64_t get_rank[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7};

const char coordFromPosition[64][3] = {
    "a1",
    "b1",
    "c1",
    "d1",
    "e1",
    "f1",
    "g1",
    "h1",
    "a2",
    "b2",
    "c2",
    "d2",
    "e2",
    "f2",
    "g2",
    "h2",
    "a3",
    "b3",
    "c3",
    "d3",
    "e3",
    "f3",
    "g3",
    "h3",
    "a4",
    "b4",
    "c4",
    "d4",
    "e4",
    "f4",
    "g4",
    "h4",
    "a5",
    "b5",
    "c5",
    "d5",
    "e5",
    "f5",
    "g5",
    "h5",
    "a6",
    "b6",
    "c6",
    "d6",
    "e6",
    "f6",
    "g6",
    "h6",
    "a7",
    "b7",
    "c7",
    "d7",
    "e7",
    "f7",
    "g7",
    "h7",
    "a8",
    "b8",
    "c8",
    "d8",
    "e8",
    "f8",
    "g8",
    "h8",
};

// isolated pawn masks for each position (basically if a pawn is on the F file, the isolated mask will be the E file + G file, meaning the neighboring files)
const uint64_t isolated_masks[64] = {
    144680345676153346,
    361700864190383365,
    723401728380766730,
    1446803456761533460,
    2893606913523066920,
    5787213827046133840,
    11574427654092267680,
    4629771061636907072,
    144680345676153346,
    361700864190383365,
    723401728380766730,
    1446803456761533460,
    2893606913523066920,
    5787213827046133840,
    11574427654092267680,
    4629771061636907072,
    144680345676153346,
    361700864190383365,
    723401728380766730,
    1446803456761533460,
    2893606913523066920,
    5787213827046133840,
    11574427654092267680,
    4629771061636907072,
    144680345676153346,
    361700864190383365,
    723401728380766730,
    1446803456761533460,
    2893606913523066920,
    5787213827046133840,
    11574427654092267680,
    4629771061636907072,
    144680345676153346,
    361700864190383365,
    723401728380766730,
    1446803456761533460,
    2893606913523066920,
    5787213827046133840,
    11574427654092267680,
    4629771061636907072,
    144680345676153346,
    361700864190383365,
    723401728380766730,
    1446803456761533460,
    2893606913523066920,
    5787213827046133840,
    11574427654092267680,
    4629771061636907072,
    144680345676153346,
    361700864190383365,
    723401728380766730,
    1446803456761533460,
    2893606913523066920,
    5787213827046133840,
    11574427654092267680,
    4629771061636907072,
    144680345676153346,
    361700864190383365,
    723401728380766730,
    1446803456761533460,
    2893606913523066920,
    5787213827046133840,
    11574427654092267680,
    4629771061636907072};

// white passed pawn mask, it shows all the squares in front of a pawn on its file and the 2 adjacent files.
uint64_t white_passed_mask[64] = {
    217020518514230016,
    506381209866536704,
    1012762419733073408,
    2025524839466146816,
    4051049678932293632,
    8102099357864587264,
    16204198715729174528,
    13889313184910721024,
    217020518514229248,
    506381209866534912,
    1012762419733069824,
    2025524839466139648,
    4051049678932279296,
    8102099357864558592,
    16204198715729117184,
    13889313184910671872,
    217020518514032640,
    506381209866076160,
    1012762419732152320,
    2025524839464304640,
    4051049678928609280,
    8102099357857218560,
    16204198715714437120,
    13889313184898088960,
    217020518463700992,
    506381209748635648,
    1012762419497271296,
    2025524838994542592,
    4051049677989085184,
    8102099355978170368,
    16204198711956340736,
    13889313181676863488,
    217020505578799104,
    506381179683864576,
    1012762359367729152,
    2025524718735458304,
    4051049437470916608,
    8102098874941833216,
    16204197749883666432,
    13889312357043142656,
    217017207043915776,
    506373483102470144,
    1012746966204940288,
    2025493932409880576,
    4050987864819761152,
    8101975729639522304,
    16203951459279044608,
    13889101250810609664,
    216172782113783808,
    504403158265495552,
    1008806316530991104,
    2017612633061982208,
    4035225266123964416,
    8070450532247928832,
    16140901064495857664,
    13835058055282163712,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0};

// black passed pawn mask, it shows all the squares in front of a pawn on its file and the 2 adjacent files.
uint64_t black_passed_mask[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    3,
    7,
    14,
    28,
    56,
    112,
    224,
    192,
    771,
    1799,
    3598,
    7196,
    14392,
    28784,
    57568,
    49344,
    197379,
    460551,
    921102,
    1842204,
    3684408,
    7368816,
    14737632,
    12632256,
    50529027,
    117901063,
    235802126,
    471604252,
    943208504,
    1886417008,
    3772834016,
    3233857728,
    12935430915,
    30182672135,
    60365344270,
    120730688540,
    241461377080,
    482922754160,
    965845508320,
    827867578560,
    3311470314243,
    7726764066567,
    15453528133134,
    30907056266268,
    61814112532536,
    123628225065072,
    247256450130144,
    211934100111552,
    847736400446211,
    1978051601041159,
    3956103202082318,
    7912206404164636,
    15824412808329272,
    31648825616658544,
    63297651233317088,
    54255129628557504};

const uint64_t black_squares_bb = 12273903644374837845;
const uint64_t white_squares_bb = 6172840429334713770;

const uint64_t queenSide = file_masks[0] | file_masks[1] | file_masks[2] | file_masks[3];
const uint64_t kingSide = file_masks[4] | file_masks[5] | file_masks[6] | file_masks[7];
const uint64_t CenterFiles = file_masks[2] | file_masks[3] | file_masks[4] | file_masks[5];

//indexed by [<file>]
const uint64_t kingFlank[8] = {
    queenSide ^ file_masks[3], queenSide, queenSide,
    CenterFiles, CenterFiles,
    kingSide, kingSide, kingSide ^ file_masks[4]};

const uint64_t center = 69122129920;

#endif