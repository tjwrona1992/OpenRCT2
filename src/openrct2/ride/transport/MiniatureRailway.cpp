/*****************************************************************************
 * Copyright (c) 2014-2019 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "../../common.h"
#include "../../interface/Viewport.h"
#include "../../paint/Paint.h"
#include "../../paint/Supports.h"
#include "../../sprites.h"
#include "../../world/Map.h"
#include "../RideData.h"
#include "../Track.h"
#include "../TrackPaint.h"
#include "../VehiclePaint.h"

enum
{
    SPR_MINIATURE_RAILWAY_FLAT_SW_NE = 23341,
    SPR_MINIATURE_RAILWAY_FLAT_NW_SE = 23342,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_0 = 23343,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_1 = 23344,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_2 = 23345,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_3 = 23346,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_4 = 23347,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_0 = 23348,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_1 = 23349,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_2 = 23350,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_3 = 23351,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_4 = 23352,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_0 = 23353,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_1 = 23354,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_2 = 23355,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_3 = 23356,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_4 = 23357,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_0 = 23358,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_1 = 23359,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_2 = 23360,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_3 = 23361,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_4 = 23362,
    SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SW_NE_PART_0 = 23363,
    SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SW_NE_PART_1 = 23364,
    SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SW_NE_PART_2 = 23365,
    SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SW_NE_PART_3 = 23366,
    SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SW_NE_PART_0 = 23367,
    SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SW_NE_PART_1 = 23368,
    SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SW_NE_PART_2 = 23369,
    SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SW_NE_PART_3 = 23370,
    SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SE_NW_PART_0 = 23371,
    SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SE_NW_PART_1 = 23372,
    SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SE_NW_PART_2 = 23373,
    SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SE_NW_PART_3 = 23374,
    SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SE_NW_PART_0 = 23375,
    SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SE_NW_PART_1 = 23376,
    SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SE_NW_PART_2 = 23377,
    SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SE_NW_PART_3 = 23378,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_SW_NE = 23379,
    SPR_MINIATURE_RAILWAY_FLAT_TO_25_DEG_UP_SW_NE = 23380,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_NE_SW = 23381,
    SPR_MINIATURE_RAILWAY_FLAT_TO_25_DEG_UP_NE_SW = 23382,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_TO_FLAT_NE_SW = 23383,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_TO_FLAT_SW_NE = 23384,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_NW_SE = 23385,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_SE_NW = 23386,
    SPR_MINIATURE_RAILWAY_FLAT_TO_25_DEG_UP_NW_SE = 23387,
    SPR_MINIATURE_RAILWAY_FLAT_TO_25_DEG_UP_SE_NW = 23388,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_TO_FLAT_SE_NW = 23389,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_TO_FLAT_NW_SE = 23390,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SW_SE_PART_0 = 23391,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SW_SE_PART_1 = 23392,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SW_SE_PART_2 = 23393,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NW_SW_PART_0 = 23394,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NW_SW_PART_1 = 23395,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NW_SW_PART_2 = 23396,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NE_NW_PART_0 = 23397,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NE_NW_PART_1 = 23398,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NE_NW_PART_2 = 23399,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SE_NE_PART_0 = 23400,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SE_NE_PART_1 = 23401,
    SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SE_NE_PART_2 = 23402,
    SPR_MINIATURE_RAILWAY_FLAT_NO_BASE_SW_NE = 23403,
    SPR_MINIATURE_RAILWAY_FLAT_NO_BASE_NW_SE = 23404,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_E_PART_0 = 23405,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_E_PART_1 = 23406,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_E_PART_2 = 23407,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_E_PART_3 = 23408,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_S_PART_0 = 23409,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_S_PART_1 = 23410,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_S_PART_2 = 23411,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_S_PART_3 = 23412,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_W_PART_0 = 23413,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_W_PART_1 = 23414,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_W_PART_2 = 23415,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_W_PART_3 = 23416,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_N_PART_0 = 23417,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_N_PART_1 = 23418,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_N_PART_2 = 23419,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_N_PART_3 = 23420,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_N_PART_0 = 23421,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_N_PART_1 = 23422,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_N_PART_2 = 23423,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_N_PART_3 = 23424,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_E_PART_0 = 23425,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_E_PART_1 = 23426,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_E_PART_2 = 23427,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_E_PART_3 = 23428,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_S_PART_0 = 23429,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_S_PART_1 = 23430,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_S_PART_2 = 23431,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_S_PART_3 = 23432,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_W_PART_0 = 23433,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_W_PART_1 = 23434,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_W_PART_2 = 23435,
    SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_W_PART_3 = 23436,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_W_E = 23437,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_N_S = 23438,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_E_W = 23439,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_S_N = 23440,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_TO_25_DEG_UP_W_E = 23441,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_TO_25_DEG_UP_N_S = 23442,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_TO_25_DEG_UP_E_W = 23443,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_TO_25_DEG_UP_S_N = 23444,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_TO_FLAT_W_E = 23445,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_TO_FLAT_N_S = 23446,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_TO_FLAT_E_W = 23447,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_TO_FLAT_S_N = 23448,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_W_E = 23449,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_N_S = 23450,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_E_W = 23451,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_S_N = 23452,
};

static constexpr const uint32_t miniature_railway_track_floor[4] = { SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS_90_DEG,
                                                                     SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS_90_DEG };

static constexpr const uint32_t miniature_railway_track_pieces_flat[4] = {
    SPR_MINIATURE_RAILWAY_FLAT_SW_NE,
    SPR_MINIATURE_RAILWAY_FLAT_NW_SE,
    SPR_MINIATURE_RAILWAY_FLAT_SW_NE,
    SPR_MINIATURE_RAILWAY_FLAT_NW_SE,
};

static constexpr const uint32_t miniature_railway_station_floor[4] = { SPR_STATION_BASE_A_SW_NE, SPR_STATION_BASE_A_NW_SE,
                                                                       SPR_STATION_BASE_A_SW_NE, SPR_STATION_BASE_A_NW_SE };

static constexpr const uint32_t miniature_railway_track_pieces_flat_station[4] = {
    SPR_MINIATURE_RAILWAY_FLAT_NO_BASE_SW_NE,
    SPR_MINIATURE_RAILWAY_FLAT_NO_BASE_NW_SE,
    SPR_MINIATURE_RAILWAY_FLAT_NO_BASE_SW_NE,
    SPR_MINIATURE_RAILWAY_FLAT_NO_BASE_NW_SE,
};

static constexpr const uint32_t miniature_railway_track_pieces_flat_quarter_turn_5_tiles[4][5] = {
    {
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_0,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_1,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_2,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_3,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SW_SE_PART_4,
    },
    {
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_0,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_1,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_2,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_3,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NW_SW_PART_4,
    },
    {
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_0,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_1,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_2,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_3,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_NE_NW_PART_4,
    },
    {
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_0,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_1,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_2,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_3,
        SPR_MINIATURE_RAILWAY_QUARTER_TURN_5_TILES_SE_NE_PART_4,
    }
};

static constexpr const uint32_t miniature_railway_track_pieces_25_deg_up[4] = {
    SPR_MINIATURE_RAILWAY_25_DEG_UP_SW_NE,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_NW_SE,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_NE_SW,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_SE_NW,
};

static constexpr const uint32_t miniature_railway_track_pieces_flat_to_25_deg_up[4] = {
    SPR_MINIATURE_RAILWAY_FLAT_TO_25_DEG_UP_SW_NE,
    SPR_MINIATURE_RAILWAY_FLAT_TO_25_DEG_UP_NW_SE,
    SPR_MINIATURE_RAILWAY_FLAT_TO_25_DEG_UP_NE_SW,
    SPR_MINIATURE_RAILWAY_FLAT_TO_25_DEG_UP_SE_NW,
};

static constexpr const uint32_t miniature_railway_track_pieces_25_deg_up_to_flat[4] = {
    SPR_MINIATURE_RAILWAY_25_DEG_UP_TO_FLAT_SW_NE,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_TO_FLAT_NW_SE,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_TO_FLAT_NE_SW,
    SPR_MINIATURE_RAILWAY_25_DEG_UP_TO_FLAT_SE_NW,
};

static constexpr const uint32_t miniature_railway_track_pieces_s_bend_left[2][4] = {
    {
        SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SW_NE_PART_0,
        SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SW_NE_PART_1,
        SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SW_NE_PART_2,
        SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SW_NE_PART_3,
    },
    {
        SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SE_NW_PART_3,
        SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SE_NW_PART_2,
        SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SE_NW_PART_1,
        SPR_MINIATURE_RAILWAY_S_BEND_LEFT_SE_NW_PART_0,
    }
};

static constexpr const uint32_t miniature_railway_track_pieces_s_bend_right[2][4] = {
    {
        SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SW_NE_PART_0,
        SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SW_NE_PART_1,
        SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SW_NE_PART_2,
        SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SW_NE_PART_3,
    },
    {
        SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SE_NW_PART_3,
        SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SE_NW_PART_2,
        SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SE_NW_PART_1,
        SPR_MINIATURE_RAILWAY_S_BEND_RIGHT_SE_NW_PART_0,
    }
};

static constexpr const uint32_t miniature_railway_track_pieces_flat_quarter_turn_3_tiles[4][3] = {
    { SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SW_SE_PART_0, SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SW_SE_PART_1,
      SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SW_SE_PART_2 },
    { SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NW_SW_PART_0, SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NW_SW_PART_1,
      SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NW_SW_PART_2 },
    { SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NE_NW_PART_0, SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NE_NW_PART_1,
      SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_NE_NW_PART_2 },
    { SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SE_NE_PART_0, SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SE_NE_PART_1,
      SPR_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SE_NE_PART_2 }
};

static constexpr const uint32_t miniature_railway_track_pieces_right_eight_to_diag[4][4] = {
    {
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_E_PART_0,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_E_PART_1,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_E_PART_2,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_E_PART_3,
    },
    {
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_S_PART_0,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_S_PART_1,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_S_PART_2,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_S_PART_3,
    },
    {
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_W_PART_0,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_W_PART_1,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_W_PART_2,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_W_PART_3,
    },
    {
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_N_PART_0,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_N_PART_1,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_N_PART_2,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_N_PART_3,
    },
};

static constexpr const CoordsXYZ miniature_railway_track_pieces_right_eight_to_diag_bounds[4][4] = {
    {
        { 32, 32, 2 },
        { 32, 16, 2 },
        { 16, 16, 2 },
        { 32, 32, 0 },
    },
    {
        { 32, 32, 2 },
        { 16, 32, 2 },
        { 16, 16, 2 },
        { 16, 16, 2 },
    },
    {
        { 32, 32, 2 },
        { 34, 16, 2 },
        { 28, 28, 2 },
        { 32, 34, 0 },
    },
    {
        { 32, 32, 2 },
        { 16, 32, 2 },
        { 16, 16, 2 },
        { 30, 30, 0 },
    },
};

static constexpr const CoordsXY miniature_railway_track_pieces_right_eight_to_diag_offset[4][4] = {
    {
        { 0, 0 },
        { 0, 16 },
        { 0, 0 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 16, 0 },
        { 0, 16 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 0, 0 },
        { 4, 4 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 0, 0 },
        { 16, 0 },
        { 0, 0 },
    },
};

static constexpr const uint32_t miniature_railway_track_pieces_left_eight_to_diag[4][4] = {
    {
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_N_PART_0,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_N_PART_1,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_N_PART_2,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SW_N_PART_3,
    },
    {
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_E_PART_0,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_E_PART_1,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_E_PART_2,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NW_E_PART_3,
    },
    {
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_S_PART_0,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_S_PART_1,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_S_PART_2,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_NE_S_PART_3,
    },
    {
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_W_PART_0,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_W_PART_1,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_W_PART_2,
        SPR_MINIATURE_RAILWAY_EIGHT_TO_DIAG_SE_W_PART_3,
    },
};

static constexpr const CoordsXYZ miniature_railway_track_pieces_left_eight_to_diag_bounds[4][4] = {
    {
        { 32, 32, 2 },
        { 32, 16, 2 },
        { 16, 16, 2 },
        { 30, 30, 0 },
    },
    {
        { 32, 32, 2 },
        { 16, 34, 2 },
        { 14, 14, 2 },
        { 34, 32, 0 },
    },
    {
        { 32, 32, 2 },
        { 32, 16, 2 },
        { 16, 16, 2 },
        { 16, 16, 2 },
    },
    {
        { 32, 32, 2 },
        { 16, 32, 2 },
        { 16, 16, 2 },
        { 32, 32, 0 },
    },
};

static constexpr const CoordsXY miniature_railway_track_pieces_left_eight_to_diag_offset[4][4] = {
    {
        { 0, 0 },
        { 0, 0 },
        { 0, 16 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 0, 0 },
        { 16, 16 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 0, 16 },
        { 16, 0 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 16, 0 },
        { 0, 0 },
        { 0, 0 },
    },
};

static constexpr const CoordsXYZ miniature_railway_track_pieces_right_eight_to_orthog_bounds[4][4] = {
    {
        { 32, 32, 2 },
        { 32, 16, 2 },
        { 16, 16, 2 },
        { 14, 14, 2 },
    },
    {
        { 32, 32, 2 },
        { 16, 34, 2 },
        { 14, 14, 2 },
        { 18, 16, 2 },
    },
    {
        { 32, 32, 2 },
        { 32, 16, 2 },
        { 16, 16, 2 },
        { 16, 16, 2 },
    },
    {
        { 32, 32, 2 },
        { 16, 32, 2 },
        { 16, 16, 2 },
        { 16, 16, 2 },
    },
};

static constexpr const CoordsXY miniature_railway_track_pieces_right_eight_to_orthog_offset[4][4] = {
    {
        { 0, 0 },
        { 0, 0 },
        { 0, 16 },
        { 16, 16 },
    },
    {
        { 0, 0 },
        { 0, 0 },
        { 16, 16 },
        { 16, 0 },
    },
    {
        { 0, 0 },
        { 0, 16 },
        { 16, 0 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 16, 0 },
        { 0, 0 },
        { 0, 16 },
    },
};

static constexpr const CoordsXYZ miniature_railway_track_pieces_left_eight_to_orthog_bounds[4][4] = {
    {
        { 32, 32, 2 },
        { 32, 16, 2 },
        { 16, 16, 2 },
        { 16, 16, 2 },
    },
    {
        { 32, 32, 2 },
        { 16, 32, 2 },
        { 16, 16, 2 },
        { 16, 16, 2 },
    },
    {
        { 32, 32, 2 },
        { 34, 16, 2 },
        { 28, 28, 2 },
        { 16, 18, 2 },
    },
    {
        { 32, 32, 2 },
        { 16, 32, 2 },
        { 16, 16, 2 },
        { 14, 14, 2 },
    },
};

static constexpr const CoordsXY miniature_railway_track_pieces_left_eight_to_orthog_offset[4][4] = {
    {
        { 0, 0 },
        { 0, 16 },
        { 0, 0 },
        { 16, 0 },
    },
    {
        { 0, 0 },
        { 16, 0 },
        { 0, 16 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 0, 0 },
        { 4, 4 },
        { 0, 16 },
    },
    {
        { 0, 0 },
        { 0, 0 },
        { 16, 0 },
        { 16, 16 },
    },
};

static constexpr const uint32_t miniature_railway_track_pieces_diag_flat[4] = {
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_W_E,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_N_S,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_E_W,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_S_N,
};

static constexpr const uint32_t miniature_railway_track_pieces_diag_flat_to_25_deg_up[4] = {
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_TO_25_DEG_UP_W_E,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_TO_25_DEG_UP_N_S,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_TO_25_DEG_UP_E_W,
    SPR_MINIATURE_RAILWAY_DIAG_FLAT_TO_25_DEG_UP_S_N,
};

static constexpr const uint32_t miniature_railway_track_pieces_diag_25_deg_up_to_flat[4] = {
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_TO_FLAT_W_E,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_TO_FLAT_N_S,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_TO_FLAT_E_W,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_TO_FLAT_S_N,
};

static constexpr const uint32_t miniature_railway_track_pieces_diag_25_deg_up[4] = {
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_W_E,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_N_S,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_E_W,
    SPR_MINIATURE_RAILWAY_DIAG_25_DEG_UP_S_N,
};

static uint32_t miniature_railway_track_to_gravel(uint32_t imageId)
{
    return imageId - SPR_MINIATURE_RAILWAY_FLAT_SW_NE + SPR_G2_MINIATURE_RAILWAY_GRAVEL_SW_NE;
}

static uint32_t miniature_railway_track_to_grooved(uint32_t imageId)
{
    return imageId - SPR_MINIATURE_RAILWAY_FLAT_SW_NE + SPR_G2_MINIATURE_RAILWAY_GROOVED_SW_NE;
}

static uint32_t miniature_railway_track_to_grooved_indent(
    uint32_t imageId, const TileElement* path, uint8_t direction, uint8_t rotation)
{
    if (!path)
    {
        return 0;
    }

    uint32_t imageIdAlt = SPR_G2_MINIATURE_RAILWAY_GROOVED_SW_NE;

    uint8_t correctedEdges = path->AsPath()->GetEdges();
    correctedEdges |= correctedEdges << 4;
    correctedEdges >>= 4 - rotation;
    correctedEdges &= 0x0F;

    if (direction & 0x1)
    {
        uint32_t imageIds[2][2] = { { SPR_G2_MINIATURE_RAILWAY_INSET_NW_SE, SPR_G2_MINIATURE_RAILWAY_INSET_END_NW },
                                    { SPR_G2_MINIATURE_RAILWAY_INSET_END_SE, SPR_G2_MINIATURE_RAILWAY_INSET_END_NW_SE } };
        imageIdAlt = imageIds[(correctedEdges & 0x2) ? 0 : 1][(correctedEdges & 0x8) ? 0 : 1];
    }
    else
    {
        uint32_t imageIds[2][2] = { { SPR_G2_MINIATURE_RAILWAY_INSET_SW_NE, SPR_G2_MINIATURE_RAILWAY_INSET_END_SW },
                                    { SPR_G2_MINIATURE_RAILWAY_INSET_END_NE, SPR_G2_MINIATURE_RAILWAY_INSET_END_SW_NE } };
        imageIdAlt = imageIds[(correctedEdges & 0x1) ? 0 : 1][(correctedEdges & 0x4) ? 0 : 1];
    }

    return imageIdAlt;
}

/** rct2: 0x008AD0C0 */
static void paint_miniature_railway_track_flat(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    bool paintAsGravel = false;
    bool paintGrooved = false;

    if (session->PathElementOnSameHeight)
    {
        paintAsGravel = true;
        paintGrooved = true;
    }

    bool isSupported = wooden_a_supports_paint_setup(
        session, direction & 1, 0, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);
    uint32_t imageId, imageIdAlt;

    // In the following 3 calls to sub_98197C_rotated/sub_98199C_rotated, we add 1 to the
    //  bound_box_offset_z argument to make straight tracks draw above footpaths
    if (isSupported)
    {
        imageId = miniature_railway_track_floor[direction] | session->TrackColours[SCHEME_SUPPORTS];
        sub_98197C_rotated(session, direction, imageId, 0, 0, 32, 20, 2, height, 0, 6, height);

        imageId = miniature_railway_track_pieces_flat[direction] | session->TrackColours[SCHEME_TRACK];
        if (!paintAsGravel)
        {
            sub_98199C_rotated(session, direction, imageId, 0, 6, 32, 20, 2, height, 0, 6, height);
        }
        else
        {
            imageIdAlt = miniature_railway_track_to_gravel(imageId);
            sub_98199C_rotated(session, direction, imageIdAlt, 0, 6, 32, 20, 2, height, 0, 6, height);
        }
        if (paintGrooved)
        {
            imageIdAlt = miniature_railway_track_to_grooved(imageId);
            sub_98199C_rotated(session, direction, imageIdAlt, 0, 6, 32, 20, 2, height, 0, 6, height + 2);
            imageIdAlt = miniature_railway_track_to_grooved_indent(
                imageId, session->PathElementOnSameHeight, direction, session->CurrentRotation);
            sub_98199C_rotated(
                session, direction,
                (imageIdAlt & 0x7FFFF) | IMAGE_TYPE_REMAP | IMAGE_TYPE_TRANSPARENT | (PALETTE_DARKEN_2 << 19), 0, 6, 32, 20, 2,
                height, 0, 6, height + 2);
        }
    }
    else
    {
        imageId = miniature_railway_track_pieces_flat[direction] | session->TrackColours[SCHEME_TRACK];
        if (!paintAsGravel)
        {
            sub_98197C_rotated(session, direction, imageId, 0, 6, 32, 20, 2, height, 0, 6, height);
        }
        else
        {
            imageIdAlt = miniature_railway_track_to_gravel(imageId);
            sub_98197C_rotated(session, direction, imageIdAlt, 0, 6, 32, 20, 2, height, 0, 6, height);
        }
        if (paintGrooved)
        {
            imageIdAlt = miniature_railway_track_to_grooved(imageId);
            sub_98197C_rotated(session, direction, imageIdAlt, 0, 6, 32, 20, 2, height, 0, 6, height + 2);
            imageIdAlt = miniature_railway_track_to_grooved_indent(
                imageId, session->PathElementOnSameHeight, direction, session->CurrentRotation);
            sub_98197C_rotated(
                session, direction,
                (imageIdAlt & 0x7FFFF) | IMAGE_TYPE_REMAP | IMAGE_TYPE_TRANSPARENT | (PALETTE_DARKEN_2 << 19), 0, 6, 32, 20, 2,
                height, 0, 6, height + 2);
        }
    }

    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_6);

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 32, 0x20);
}

/** rct2: 0x008AD170, 0x008AD180, 0x008AD190 */
static void paint_miniature_railway_station(
    paint_session* session, ride_id_t rideIndex, [[maybe_unused]] uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    uint32_t imageId;

    wooden_a_supports_paint_setup(session, direction & 1, 0, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);

    imageId = miniature_railway_station_floor[direction] | session->TrackColours[SCHEME_MISC];
    sub_98197C_rotated(session, direction, imageId, 0, 0, 32, 28, 2, height - 2, 0, 2, height);

    imageId = miniature_railway_track_pieces_flat_station[direction] | session->TrackColours[SCHEME_TRACK];
    sub_98199C_rotated(session, direction, imageId, 0, 6, 32, 20, 2, height, 0, 0, height);

    paint_util_push_tunnel_rotated(session, direction, height, TUNNEL_6);

    track_paint_util_draw_station_3(session, rideIndex, direction, height + 2, height, tileElement);
    // covers shouldn't be offset by +2

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 30, 0x20);
}

/** rct2: 0x008AD0D0 */
static void paint_miniature_railway_track_25_deg_up(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    uint32_t imageId = miniature_railway_track_pieces_25_deg_up[direction] | session->TrackColours[SCHEME_TRACK];

    sub_98197C_rotated(session, direction, imageId, 0, 2, 32, 25, 2, height, 0, 3, height);

    switch (direction)
    {
        case 0:
            paint_util_push_tunnel_left(session, height - 8, TUNNEL_7);
            break;
        case 1:
            paint_util_push_tunnel_right(session, height + 8, TUNNEL_8);
            break;
        case 2:
            paint_util_push_tunnel_left(session, height + 8, TUNNEL_8);
            break;
        case 3:
            paint_util_push_tunnel_right(session, height - 8, TUNNEL_7);
            break;
    }

    wooden_a_supports_paint_setup(
        session, direction & 1, 45 + direction, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 56, 0x20);
}

/** rct2: 0x008AD0E0 */
static void paint_miniature_railway_track_flat_to_25_deg_up(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    uint32_t imageId = miniature_railway_track_pieces_flat_to_25_deg_up[direction] | session->TrackColours[SCHEME_TRACK];

    sub_98197C_rotated(session, direction, imageId, 0, 2, 32, 25, 2, height, 0, 3, height);

    switch (direction)
    {
        case 0:
            paint_util_push_tunnel_left(session, height, TUNNEL_6);
            break;
        case 1:
            paint_util_push_tunnel_right(session, height, TUNNEL_8);
            break;
        case 2:
            paint_util_push_tunnel_left(session, height, TUNNEL_8);
            break;
        case 3:
            paint_util_push_tunnel_right(session, height, TUNNEL_6);
            break;
    }

    wooden_a_supports_paint_setup(
        session, direction & 1, 37 + direction, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 48, 0x20);
}

/** rct2: 0x008AD0F0 */
static void paint_miniature_railway_track_25_deg_up_to_flat(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    uint32_t imageId = miniature_railway_track_pieces_25_deg_up_to_flat[direction] | session->TrackColours[SCHEME_TRACK];

    sub_98197C_rotated(session, direction, imageId, 0, 2, 32, 25, 2, height, 0, 3, height);

    switch (direction)
    {
        case 0:
            paint_util_push_tunnel_left(session, height - 8, TUNNEL_6);
            break;
        case 1:
            paint_util_push_tunnel_right(session, height + 8, TUNNEL_14);
            break;
        case 2:
            paint_util_push_tunnel_left(session, height + 8, TUNNEL_14);
            break;
        case 3:
            paint_util_push_tunnel_right(session, height - 8, TUNNEL_6);
            break;
    }

    wooden_a_supports_paint_setup(
        session, direction & 1, 41 + direction, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 40, 0x20);
}

/** rct2: 0x008AD100 */
static void paint_miniature_railway_track_25_deg_down(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    paint_miniature_railway_track_25_deg_up(session, rideIndex, trackSequence, (direction + 2) % 4, height, tileElement);
}

/** rct2: 0x008AD110 */
static void paint_miniature_railway_track_flat_to_25_deg_down(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    paint_miniature_railway_track_25_deg_up_to_flat(
        session, rideIndex, trackSequence, (direction + 2) % 4, height, tileElement);
}

/** rct2: 0x008AD120 */
static void paint_miniature_railway_track_25_deg_down_to_flat(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    paint_miniature_railway_track_flat_to_25_deg_up(
        session, rideIndex, trackSequence, (direction + 2) % 4, height, tileElement);
}

// clang-format off
static constexpr const CoordsXY miniature_railway_right_quarter_turn_5_tiles_offsets[4][5] = { {
                                                                                  { 0, 2 },
                                                                                  { 0, 16 },
                                                                                  { 0, 0 },
                                                                                  { 16, 0 },
                                                                                  { 2, 0 },
                                                                              },
                                                                              {
                                                                                  { 2, 0 },
                                                                                  { 16, 0 },
                                                                                  { 0, 16 },
                                                                                  { 0, 0 },
                                                                                  { 0, 2 },
                                                                              },
                                                                              {
                                                                                  { 0, 2 },
                                                                                  { 0, 0 },
                                                                                  { 16, 16 },
                                                                                  { 0, 0 },
                                                                                  { 2, 0 },
                                                                              },
                                                                              {
                                                                                  { 2, 0 },
                                                                                  { 0, 0 },
                                                                                  { 16, 0 },
                                                                                  { 0, 16 },
                                                                                  { 0, 2 },
                                                                              } };

static constexpr const CoordsXYZ miniature_railway_right_quarter_turn_5_tiles_bound_offsets[4][5] = { {
                                                                                         { 0, 2, 0 },
                                                                                         { 0, 16, 0 },
                                                                                         { 0, 0, 0 },
                                                                                         { 16, 0, 0 },
                                                                                         { 2, 0, 0 },
                                                                                     },
                                                                                     {
                                                                                         { 2, 0, 0 },
                                                                                         { 16, 0, 0 },
                                                                                         { 0, 16, 0 },
                                                                                         { 0, 0, 0 },
                                                                                         { 0, 2, 0 },
                                                                                     },
                                                                                     {
                                                                                         { 0, 2, 0 },
                                                                                         { 0, 0, 0 },
                                                                                         { 16, 16, 0 },
                                                                                         { 0, 0, 0 },
                                                                                         { 2, 0, 0 },
                                                                                     },
                                                                                     {
                                                                                         { 2, 0, 0 },
                                                                                         { 0, 0, 0 },
                                                                                         { 16, 0, 0 },
                                                                                         { 0, 16, 0 },
                                                                                         { 0, 2, 0 },
                                                                                     } };

static constexpr const CoordsXY miniature_railway_right_quarter_turn_5_tiles_bound_lengths[4][5] = { {
                                                                                        { 32, 32 },
                                                                                        { 32, 16 },
                                                                                        { 16, 16 },
                                                                                        { 16, 32 },
                                                                                        { 32, 32 },
                                                                                    },
                                                                                    {
                                                                                        { 32, 32 },
                                                                                        { 16, 34 },
                                                                                        { 16, 16 },
                                                                                        { 32, 16 },
                                                                                        { 32, 27 },
                                                                                    },
                                                                                    {
                                                                                        { 32, 27 },
                                                                                        { 32, 16 },
                                                                                        { 16, 16 },
                                                                                        { 16, 32 },
                                                                                        { 27, 32 },
                                                                                    },
                                                                                    {
                                                                                        { 27, 32 },
                                                                                        { 16, 32 },
                                                                                        { 16, 16 },
                                                                                        { 32, 16 },
                                                                                        { 32, 32 },
                                                                                    } };
// clang-format on

static constexpr const uint32_t miniature_railway_right_quarter_turn_5_tiles_track_floor[4][5] = {
    { SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS_S_SEGMENT, SPR_FLOOR_PLANKS_N_SEGMENT, SPR_FLOOR_PLANKS_S_SEGMENT,
      SPR_FLOOR_PLANKS_90_DEG },
    { SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_W_SEGMENT, SPR_FLOOR_PLANKS_E_SEGMENT, SPR_FLOOR_PLANKS_W_SEGMENT,
      SPR_FLOOR_PLANKS },
    { SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS_N_SEGMENT, SPR_FLOOR_PLANKS_S_SEGMENT, SPR_FLOOR_PLANKS_N_SEGMENT,
      SPR_FLOOR_PLANKS_90_DEG },
    { SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_E_SEGMENT, SPR_FLOOR_PLANKS_W_SEGMENT, SPR_FLOOR_PLANKS_E_SEGMENT,
      SPR_FLOOR_PLANKS },
};

static constexpr const int8_t right_quarter_turn_5_supports_type[4][7] = {
    { 0, -1, 4, 2, -1, 4, 1 }, { 1, -1, 5, 3, -1, 5, 0 }, { 0, -1, 2, 4, -1, 2, 1 }, { 1, -1, 3, 5, -1, 3, 0 }
};

static constexpr const int8_t miniature_railway_right_quarter_turn_5_tiles_sprite_map[] = { 0, -1, 1, 2, -1, 3, 4 };

/** rct2: 0x008AD140 */
static void paint_miniature_railway_track_right_quarter_turn_5_tiles(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    if (right_quarter_turn_5_supports_type[direction][trackSequence] != -1)
    {
        bool isSupported = wooden_a_supports_paint_setup(
            session, right_quarter_turn_5_supports_type[direction][trackSequence], 0, height,
            session->TrackColours[SCHEME_SUPPORTS], nullptr);

        if (!isSupported || (trackSequence == 3 && direction == 2))
        {
            track_paint_util_right_quarter_turn_5_tiles_paint(
                session, 2, height, direction, trackSequence, session->TrackColours[SCHEME_TRACK],
                miniature_railway_track_pieces_flat_quarter_turn_5_tiles, miniature_railway_right_quarter_turn_5_tiles_offsets,
                miniature_railway_right_quarter_turn_5_tiles_bound_lengths, nullptr);
        }
        else
        {
            track_paint_util_right_quarter_turn_5_tiles_paint(
                session, 2, height, direction, trackSequence, session->TrackColours[SCHEME_SUPPORTS],
                miniature_railway_right_quarter_turn_5_tiles_track_floor, nullptr,
                miniature_railway_right_quarter_turn_5_tiles_bound_lengths,
                miniature_railway_right_quarter_turn_5_tiles_bound_offsets);

            int32_t index = miniature_railway_right_quarter_turn_5_tiles_sprite_map[trackSequence];
            uint32_t imageId = miniature_railway_track_pieces_flat_quarter_turn_5_tiles[direction][index]
                | session->TrackColours[SCHEME_TRACK];
            CoordsXY offset = miniature_railway_right_quarter_turn_5_tiles_offsets[direction][index];
            CoordsXY boundsLength = miniature_railway_right_quarter_turn_5_tiles_bound_lengths[direction][index];
            CoordsXYZ boundsOffset = CoordsXYZ(offset, 0);

            sub_98199C(
                session, imageId, (int8_t)offset.x, (int8_t)offset.y, boundsLength.x, boundsLength.y, 2, height, boundsOffset.x,
                boundsOffset.y, height + boundsOffset.z);
        }
    }
    if (direction == 0 && trackSequence == 0)
    {
        paint_util_push_tunnel_left(session, height, TUNNEL_6);
    }

    if (direction == 0 && trackSequence == 6)
    {
        paint_util_push_tunnel_right(session, height, TUNNEL_6);
    }

    if (direction == 1 && trackSequence == 6)
    {
        paint_util_push_tunnel_left(session, height, TUNNEL_6);
    }

    if (direction == 3 && trackSequence == 0)
    {
        paint_util_push_tunnel_right(session, height, TUNNEL_6);
    }

    int32_t blockedSegments = 0;
    switch (trackSequence)
    {
        case 0:
            blockedSegments = SEGMENTS_ALL;
            break;
        case 1:
            blockedSegments = SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC;
            break;
        case 2:
            blockedSegments = SEGMENT_D0 | SEGMENT_C4 | SEGMENT_D4 | SEGMENT_BC | SEGMENT_C0 | SEGMENT_CC;
            break;
        case 3:
            blockedSegments = SEGMENT_B4 | SEGMENT_B8 | SEGMENT_BC | SEGMENT_C4 | SEGMENT_C8 | SEGMENT_CC | SEGMENT_D0
                | SEGMENT_D4;
            break;
        case 4:
            blockedSegments = SEGMENT_B4 | SEGMENT_C8 | SEGMENT_CC;
            break;
        case 5:
            blockedSegments = SEGMENT_D4 | SEGMENT_C4 | SEGMENT_D0 | SEGMENT_B8 | SEGMENT_C0 | SEGMENT_C8;
            break;
        case 6:
            blockedSegments = SEGMENTS_ALL;
            break;
    }

    paint_util_set_segment_support_height(session, paint_util_rotate_segments(blockedSegments, direction), 0xFFFF, 0);

    paint_util_set_general_support_height(session, height + 32, 0x20);
}

/** rct2: 0x008AD130 */
static void paint_miniature_railway_track_left_quarter_turn_5_tiles(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    trackSequence = mapLeftQuarterTurn5TilesToRightQuarterTurn5Tiles[trackSequence];
    paint_miniature_railway_track_right_quarter_turn_5_tiles(
        session, rideIndex, trackSequence, (direction + 1) % 4, height, tileElement);
}

static constexpr const int8_t s_bend_left_supports_type[4][4] = {
    { 0, 5, 3, 0 }, { 1, 2, 4, 1 }, { 0, 5, 3, 0 }, { 1, 2, 4, 1 }
};

static constexpr const uint32_t miniature_railway_s_bend_left_tiles_track_floor[2][4] = {
    { SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS_W_SEGMENT, SPR_FLOOR_PLANKS_E_SEGMENT, SPR_FLOOR_PLANKS },
    {
        SPR_FLOOR_PLANKS_90_DEG,
        SPR_FLOOR_PLANKS_N_SEGMENT,
        SPR_FLOOR_PLANKS_S_SEGMENT,
        SPR_FLOOR_PLANKS_90_DEG,
    }
};

/** rct2: 0x8AD150 */
static void paint_miniature_railway_track_s_bend_left(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    if (direction == 2 || direction == 3)
    {
        trackSequence = 3 - trackSequence;
    }

    bool isSupported = wooden_a_supports_paint_setup(
        session, s_bend_left_supports_type[direction][trackSequence], 0, height, session->TrackColours[SCHEME_SUPPORTS],
        nullptr);

    static constexpr const CoordsXY offsetList[] = {
        { 0, 2 },
        { 0, 0 },
        { 0, 6 },
        { 0, 2 },
    };

    static constexpr const CoordsXY boundsList[] = {
        { 32, 27 },
        { 32, 26 },
        { 32, 26 },
        { 32, 27 },
    };

    uint32_t imageId = miniature_railway_track_pieces_s_bend_left[direction & 1][trackSequence]
        | session->TrackColours[SCHEME_TRACK];
    CoordsXY offset = offsetList[trackSequence];
    CoordsXY bounds = boundsList[trackSequence];

    if (!isSupported)
    {
        sub_98197C_rotated(
            session, direction, imageId, (int8_t)offset.x, (int8_t)offset.y, bounds.x, bounds.y, 2, height, offset.x, offset.y,
            height);
    }
    else
    {
        imageId = miniature_railway_s_bend_left_tiles_track_floor[direction & 1][trackSequence]
            | session->TrackColours[SCHEME_SUPPORTS];
        sub_98197C_rotated(session, direction, imageId, 0, 0, bounds.x, bounds.y, 2, height, offset.x, offset.y, height);

        imageId = miniature_railway_track_pieces_s_bend_left[direction & 1][trackSequence]
            | session->TrackColours[SCHEME_TRACK];
        sub_98199C_rotated(
            session, direction, imageId, (int8_t)offset.x, (int8_t)offset.y, bounds.x, bounds.y, 2, height, offset.x, offset.y,
            height);
    }
    if (direction == 0 || direction == 2)
    {
        if (trackSequence == 0)
        {
            paint_util_push_tunnel_left(session, height, TUNNEL_6);
        }
    }
    else
    {
        if (trackSequence == 3)
        {
            paint_util_push_tunnel_right(session, height, TUNNEL_6);
        }
    }

    int32_t blockedSegments = 0;
    switch (trackSequence)
    {
        case 0:
            blockedSegments = SEGMENTS_ALL;
            break;
        case 1:
            blockedSegments = SEGMENT_D0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_B8 | SEGMENT_C8 | SEGMENT_B4;
            break;
        case 2:
            blockedSegments = SEGMENT_D0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_C0 | SEGMENT_D4 | SEGMENT_BC;
            break;
        case 3:
            blockedSegments = SEGMENTS_ALL;
            break;
    }
    paint_util_set_segment_support_height(session, paint_util_rotate_segments(blockedSegments, direction & 1), 0xFFFF, 0);

    paint_util_set_general_support_height(session, height + 32, 0x20);
}

static constexpr const int8_t s_bend_right_supports_type[4][4] = {
    { 0, 4, 2, 0 }, { 1, 5, 3, 1 }, { 0, 4, 2, 0 }, { 1, 5, 3, 1 }
};

static constexpr const uint32_t miniature_railway_s_bend_right_tiles_track_floor[2][4] = {
    { SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS_S_SEGMENT, SPR_FLOOR_PLANKS_N_SEGMENT, SPR_FLOOR_PLANKS },
    {
        SPR_FLOOR_PLANKS_90_DEG,
        SPR_FLOOR_PLANKS_W_SEGMENT,
        SPR_FLOOR_PLANKS_E_SEGMENT,
        SPR_FLOOR_PLANKS_90_DEG,
    }
};

/** rct2: 0x008AD160 */
static void paint_miniature_railway_track_s_bend_right(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    if (direction == 2 || direction == 3)
    {
        trackSequence = 3 - trackSequence;
    }

    bool isSupported = wooden_a_supports_paint_setup(
        session, s_bend_right_supports_type[direction][trackSequence], 0, height, session->TrackColours[SCHEME_SUPPORTS],
        nullptr);

    static constexpr const CoordsXY offsetList[] = {
        { 0, 2 },
        { 0, 6 },
        { 0, 0 },
        { 0, 2 },
    };

    static constexpr const CoordsXY boundsList[] = {
        { 32, 27 },
        { 32, 26 },
        { 32, 26 },
        { 32, 27 },
    };

    uint32_t imageId = miniature_railway_track_pieces_s_bend_right[direction & 1][trackSequence]
        | session->TrackColours[SCHEME_TRACK];
    CoordsXY offset = offsetList[trackSequence];
    CoordsXY bounds = boundsList[trackSequence];
    if (!isSupported)
    {
        sub_98197C_rotated(
            session, direction, imageId, (int8_t)offset.x, (int8_t)offset.y, bounds.x, bounds.y, 2, height, offset.x, offset.y,
            height);
    }
    else
    {
        imageId = miniature_railway_s_bend_right_tiles_track_floor[direction & 1][trackSequence]
            | session->TrackColours[SCHEME_SUPPORTS];
        sub_98197C_rotated(session, direction, imageId, 0, 0, bounds.x, bounds.y, 2, height, offset.x, offset.y, height);

        imageId = miniature_railway_track_pieces_s_bend_right[direction & 1][trackSequence]
            | session->TrackColours[SCHEME_TRACK];
        sub_98199C_rotated(
            session, direction, imageId, (int8_t)offset.x, (int8_t)offset.y, bounds.x, bounds.y, 2, height, offset.x, offset.y,
            height);
    }

    if (direction == 0 || direction == 2)
    {
        if (trackSequence == 0)
        {
            paint_util_push_tunnel_left(session, height, TUNNEL_6);
        }
    }
    else
    {
        if (trackSequence == 3)
        {
            paint_util_push_tunnel_right(session, height, TUNNEL_6);
        }
    }

    int32_t blockedSegments = 0;
    switch (trackSequence)
    {
        case 0:
            blockedSegments = SEGMENTS_ALL;
            break;
        case 1:
            blockedSegments = SEGMENT_D0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_C0 | SEGMENT_D4 | SEGMENT_BC;
            break;
        case 2:
            blockedSegments = SEGMENT_D0 | SEGMENT_C4 | SEGMENT_CC | SEGMENT_B8 | SEGMENT_C8 | SEGMENT_B4;
            break;
        case 3:
            blockedSegments = SEGMENTS_ALL;
            break;
    }
    paint_util_set_segment_support_height(session, paint_util_rotate_segments(blockedSegments, direction & 1), 0xFFFF, 0);

    paint_util_set_general_support_height(session, height + 32, 0x20);
}

static constexpr const uint32_t miniature_railway_right_quarter_turn_3_tile_track_floor[4][3] = {
    {
        SPR_FLOOR_PLANKS_S_SEGMENT,
        0,
        SPR_FLOOR_PLANKS_S_SEGMENT,
    },
    {
        SPR_FLOOR_PLANKS_W_SEGMENT,
        0,
        SPR_FLOOR_PLANKS_W_SEGMENT,
    },
    {
        SPR_FLOOR_PLANKS_N_SEGMENT,
        0,
        SPR_FLOOR_PLANKS_N_SEGMENT,
    },
    {
        SPR_FLOOR_PLANKS_E_SEGMENT,
        0,
        SPR_FLOOR_PLANKS_E_SEGMENT,
    }
};

static constexpr const CoordsXYZ miniature_railway_right_quarter_turn_3_tile_bound_offsets[4][3] = { {
                                                                                                         { 0, 6, 0 },
                                                                                                         { 16, 16, 0 },
                                                                                                         { 6, 0, 0 },
                                                                                                     },
                                                                                                     {
                                                                                                         { 6, 0, 0 },
                                                                                                         { 16, 0, 0 },
                                                                                                         { 0, 6, 0 },
                                                                                                     },
                                                                                                     {
                                                                                                         { 0, 6, 0 },
                                                                                                         { 0, 0, 0 },
                                                                                                         { 6, 0, 0 },
                                                                                                     },
                                                                                                     {
                                                                                                         { 6, 0, 0 },
                                                                                                         { 0, 16, 0 },
                                                                                                         { 0, 6, 0 },
                                                                                                     } };

/** rct2: 0x008AD1B0 */
static void paint_miniature_railway_track_right_quarter_turn_3_tiles(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    bool isSupported = false;
    if (trackSequence != 1 && trackSequence != 2)
    {
        static constexpr const uint8_t supportType[] = { 4, 5, 2, 3 };
        isSupported = wooden_a_supports_paint_setup(
            session, supportType[direction], 0, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }
    if (!isSupported)
    {
        track_paint_util_right_quarter_turn_3_tiles_paint(
            session, 3, height, direction, trackSequence, session->TrackColours[SCHEME_TRACK],
            miniature_railway_track_pieces_flat_quarter_turn_3_tiles, defaultRightQuarterTurn3TilesOffsets,
            defaultRightQuarterTurn3TilesBoundLengths, nullptr);

        // The following piece was missing in vanilla RCT2
        if (trackSequence == 1 && direction == 0)
        {
            uint32_t imageId = SPR_G2_MINIATURE_RAILWAY_QUARTER_TURN_3_TILES_SW_SE_PART_3 | session->TrackColours[SCHEME_TRACK];
            sub_98197C(session, imageId, 0, 0, 8, 8, 2, height, 0, 0, height);
        }
    }
    else
    {
        track_paint_util_right_quarter_turn_3_tiles_paint(
            session, 3, height, direction, trackSequence, session->TrackColours[SCHEME_SUPPORTS],
            miniature_railway_right_quarter_turn_3_tile_track_floor, nullptr, defaultRightQuarterTurn3TilesBoundLengths,
            miniature_railway_right_quarter_turn_3_tile_bound_offsets);

        static constexpr const int8_t _right_quarter_turn_3_tiles_sprite_map[] = { 0, -1, 1, 2 };

        int32_t index = _right_quarter_turn_3_tiles_sprite_map[trackSequence];

        uint32_t imageId = miniature_railway_track_pieces_flat_quarter_turn_3_tiles[direction][index]
            | session->TrackColours[SCHEME_TRACK];
        CoordsXY offset = defaultRightQuarterTurn3TilesOffsets[direction][index];
        CoordsXY boundsLength = defaultRightQuarterTurn3TilesBoundLengths[direction][index];
        CoordsXYZ boundsOffset(offset, 0);

        sub_98199C(
            session, imageId, (int8_t)offset.x, (int8_t)offset.y, boundsLength.x, boundsLength.y, 3, height, boundsOffset.x,
            boundsOffset.y, height + boundsOffset.z);
    }
    track_paint_util_right_quarter_turn_3_tiles_tunnel(session, height, direction, trackSequence, TUNNEL_6);

    int32_t blockedSegments = 0;
    switch (trackSequence)
    {
        case 0:
            blockedSegments = SEGMENTS_ALL;
            break;
        case 2:
            blockedSegments = SEGMENT_D0 | SEGMENT_C4 | SEGMENT_D4 | SEGMENT_C0;
            break;
        case 3:
            blockedSegments = SEGMENTS_ALL;
            break;
    }
    paint_util_set_segment_support_height(session, paint_util_rotate_segments(blockedSegments, direction), 0xFFFF, 0);

    paint_util_set_general_support_height(session, height + 32, 0x20);
}

/** rct2: 0x008AD1A0 */
static void paint_miniature_railway_track_left_quarter_turn_3_tiles(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    trackSequence = mapLeftQuarterTurn3TilesToRightQuarterTurn3Tiles[trackSequence];
    paint_miniature_railway_track_right_quarter_turn_3_tiles(
        session, rideIndex, trackSequence, (direction + 1) % 4, height, tileElement);
}

static constexpr const int8_t paint_miniature_railway_eighth_to_diag_index[] = { 0, 1, 2, -1, 3 };

static constexpr const uint32_t miniature_railway_floor_track_pieces_left_eight_to_diag[4][5] = {
    { SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS_E_SEGMENT, SPR_FLOOR_PLANKS_W_SEGMENT, SPR_FLOOR_PLANKS_90_DEG },
    { SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_S_SEGMENT, SPR_FLOOR_PLANKS_N_SEGMENT,
      SPR_FLOOR_PLANKS },
    { SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS_W_SEGMENT, SPR_FLOOR_PLANKS_E_SEGMENT, SPR_FLOOR_PLANKS_90_DEG },
    { SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_N_SEGMENT, SPR_FLOOR_PLANKS_S_SEGMENT,
      SPR_FLOOR_PLANKS },
};

static constexpr const CoordsXYZ miniature_railway_track_floor_pieces_left_eight_to_diag_bounds[4][5] = {
    {
        { 32, 32, 2 },
        { 32, 16, 2 },
        { 16, 16, 2 },
        { 16, 16, 0 },
        { 30, 30, 0 },
    },
    {
        { 32, 32, 2 },
        { 16, 34, 2 },
        { 14, 14, 2 },
        { 16, 16, 0 },
        { 34, 32, 0 },
    },
    {
        { 32, 32, 2 },
        { 32, 16, 2 },
        { 16, 16, 2 },
        { 16, 16, 0 },
        { 16, 16, 2 },
    },
    {
        { 32, 32, 2 },
        { 16, 32, 2 },
        { 16, 16, 2 },
        { 16, 16, 0 },
        { 32, 32, 0 },
    },
};

static constexpr const CoordsXY miniature_railway_track_floor_pieces_left_eight_to_diag_offset[4][5] = {
    {
        { 0, 0 },
        { 0, 0 },
        { 0, 16 },
        { 16, 0 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 0, 0 },
        { 16, 16 },
        { 0, 0 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 0, 16 },
        { 16, 0 },
        { 0, 16 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 16, 0 },
        { 0, 0 },
        { 16, 16 },
        { 0, 0 },
    },
};

/** rct2: 0x008AD1C0 */
static void paint_miniature_railway_track_left_eighth_to_diag(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    const uint8_t supportType[4][5] = { { 0, 0, 3, 5, 1 }, { 1, 1, 4, 2, 0 }, { 0, 0, 5, 3, 1 }, { 1, 1, 2, 4, 0 } };

    bool isSupported = false;
    bool isRightEighthToOrthog = tileElement->AsTrack()->GetTrackType() == TRACK_ELEM_RIGHT_EIGHTH_TO_ORTHOGONAL;
    // Right eighth to orthogonal calls this function but we do not want to have a support call for it
    // for track sequence 4
    if (trackSequence != 4 || !isRightEighthToOrthog)
    {
        isSupported = wooden_a_supports_paint_setup(
            session, supportType[direction][trackSequence], 0, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }
    uint32_t imageId;
    if (!isSupported)
    {
        int8_t index = paint_miniature_railway_eighth_to_diag_index[trackSequence];
        if (index >= 0)
        {
            imageId = miniature_railway_track_pieces_left_eight_to_diag[direction][index] | session->TrackColours[SCHEME_TRACK];
            CoordsXY offset = miniature_railway_track_pieces_left_eight_to_diag_offset[direction][index];
            CoordsXYZ bounds = miniature_railway_track_pieces_left_eight_to_diag_bounds[direction][index];
            if (isRightEighthToOrthog)
            {
                bounds = miniature_railway_track_pieces_right_eight_to_orthog_bounds[direction][index];
                offset = miniature_railway_track_pieces_right_eight_to_orthog_offset[direction][index];
            }
            sub_98197C(session, imageId, 0, 0, bounds.x, bounds.y, (int8_t)bounds.z, height, offset.x, offset.y, height);
        }
    }
    else
    {
        imageId = miniature_railway_floor_track_pieces_left_eight_to_diag[direction][trackSequence]
            | session->TrackColours[SCHEME_SUPPORTS];
        CoordsXY offset = miniature_railway_track_floor_pieces_left_eight_to_diag_offset[direction][trackSequence];
        CoordsXYZ bounds = miniature_railway_track_floor_pieces_left_eight_to_diag_bounds[direction][trackSequence];
        sub_98197C(session, imageId, 0, 0, bounds.x, bounds.y, (int8_t)bounds.z, height, offset.x, offset.y, height);

        int8_t index = paint_miniature_railway_eighth_to_diag_index[trackSequence];
        if (index >= 0)
        {
            imageId = miniature_railway_track_pieces_left_eight_to_diag[direction][index] | session->TrackColours[SCHEME_TRACK];
            offset = miniature_railway_track_pieces_left_eight_to_diag_offset[direction][index];
            bounds = miniature_railway_track_pieces_left_eight_to_diag_bounds[direction][index];
            sub_98199C(session, imageId, 0, 0, bounds.x, bounds.y, (int8_t)bounds.z, height, offset.x, offset.y, height);
        }
    }

    if (direction == 0 && trackSequence == 0)
    {
        paint_util_push_tunnel_left(session, height, TUNNEL_6);
    }
    if (direction == 3 && trackSequence == 0)
    {
        paint_util_push_tunnel_right(session, height, TUNNEL_6);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 32, 0x20);
}

static constexpr const uint32_t miniature_railway_floor_track_pieces_right_eight_to_diag[4][5] = {
    { SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_N_SEGMENT, SPR_FLOOR_PLANKS_S_SEGMENT,
      SPR_FLOOR_PLANKS_90_DEG },
    { SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_E_SEGMENT, SPR_FLOOR_PLANKS_W_SEGMENT,
      SPR_FLOOR_PLANKS },
    { SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS, SPR_FLOOR_PLANKS_S_SEGMENT, SPR_FLOOR_PLANKS_N_SEGMENT, SPR_FLOOR_PLANKS_90_DEG },
    { SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_90_DEG, SPR_FLOOR_PLANKS_W_SEGMENT, SPR_FLOOR_PLANKS_E_SEGMENT,
      SPR_FLOOR_PLANKS },
};

static constexpr const CoordsXYZ miniature_railway_track_floor_pieces_right_eight_to_diag_bounds[4][5] = {
    {
        { 32, 32, 2 },
        { 32, 16, 2 },
        { 16, 16, 2 },
        { 16, 16, 0 },
        { 32, 32, 0 },
    },
    {
        { 32, 32, 2 },
        { 16, 32, 2 },
        { 16, 16, 2 },
        { 16, 16, 0 },
        { 16, 16, 2 },
    },
    {
        { 32, 32, 2 },
        { 34, 16, 2 },
        { 28, 28, 2 },
        { 16, 16, 0 },
        { 32, 34, 0 },
    },
    {
        { 32, 32, 2 },
        { 16, 32, 2 },
        { 16, 16, 2 },
        { 16, 16, 0 },
        { 30, 30, 0 },
    },
};

static constexpr const CoordsXY miniature_railway_track_floor_pieces_right_eight_to_diag_offset[4][5] = {
    {
        { 0, 0 },
        { 0, 16 },
        { 0, 0 },
        { 16, 16 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 16, 0 },
        { 0, 16 },
        { 16, 0 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 0, 0 },
        { 4, 4 },
        { 0, 0 },
        { 0, 0 },
    },
    {
        { 0, 0 },
        { 0, 0 },
        { 16, 0 },
        { 0, 16 },
        { 0, 0 },
    },
};

/** rct2: 0x008AD1D0 */
static void paint_miniature_railway_track_right_eighth_to_diag(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    const uint8_t supportType[4][5] = { { 0, 0, 2, 4, 1 }, { 1, 1, 3, 5, 0 }, { 0, 0, 4, 2, 1 }, { 1, 1, 5, 3, 0 } };

    bool isSupported = false;
    bool isLeftEighthToOrthog = tileElement->AsTrack()->GetTrackType() == TRACK_ELEM_LEFT_EIGHTH_TO_ORTHOGONAL;
    // Right eighth to orthogonal calls this function but we do not want to have a support call for it
    // for track sequence 4
    if (trackSequence != 4 || !isLeftEighthToOrthog)
    {
        isSupported = wooden_a_supports_paint_setup(
            session, supportType[direction][trackSequence], 0, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }

    uint32_t imageId;
    if (!isSupported)
    {
        int8_t index = paint_miniature_railway_eighth_to_diag_index[trackSequence];
        if (index >= 0)
        {
            imageId = miniature_railway_track_pieces_right_eight_to_diag[direction][index]
                | session->TrackColours[SCHEME_TRACK];
            CoordsXY offset = miniature_railway_track_pieces_right_eight_to_diag_offset[direction][index];
            CoordsXYZ bounds = miniature_railway_track_pieces_right_eight_to_diag_bounds[direction][index];
            if (isLeftEighthToOrthog)
            {
                bounds = miniature_railway_track_pieces_left_eight_to_orthog_bounds[direction][index];
                offset = miniature_railway_track_pieces_left_eight_to_orthog_offset[direction][index];
            }
            sub_98197C(session, imageId, 0, 0, bounds.x, bounds.y, (int8_t)bounds.z, height, offset.x, offset.y, height);
        }
    }
    else
    {
        imageId = miniature_railway_floor_track_pieces_right_eight_to_diag[direction][trackSequence]
            | session->TrackColours[SCHEME_SUPPORTS];
        CoordsXY offset = miniature_railway_track_floor_pieces_right_eight_to_diag_offset[direction][trackSequence];
        CoordsXYZ bounds = miniature_railway_track_floor_pieces_right_eight_to_diag_bounds[direction][trackSequence];
        sub_98197C(session, imageId, 0, 0, bounds.x, bounds.y, (int8_t)bounds.z, height, offset.x, offset.y, height);

        int8_t index = paint_miniature_railway_eighth_to_diag_index[trackSequence];
        if (index >= 0)
        {
            imageId = miniature_railway_track_pieces_right_eight_to_diag[direction][index]
                | session->TrackColours[SCHEME_TRACK];
            offset = miniature_railway_track_pieces_right_eight_to_diag_offset[direction][index];
            bounds = miniature_railway_track_pieces_right_eight_to_diag_bounds[direction][index];
            sub_98199C(session, imageId, 0, 0, bounds.x, bounds.y, (int8_t)bounds.z, height, offset.x, offset.y, height);
        }
    }

    if (direction == 0 && trackSequence == 0)
    {
        paint_util_push_tunnel_left(session, height, TUNNEL_6);
    }
    if (direction == 3 && trackSequence == 0)
    {
        paint_util_push_tunnel_right(session, height, TUNNEL_6);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 32, 0x20);
}

/** rct2: 0x008AD1E0 */
static void paint_miniature_railway_track_left_eighth_to_orthogonal(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    trackSequence = mapLeftEighthTurnToOrthogonal[trackSequence];
    paint_miniature_railway_track_right_eighth_to_diag(
        session, rideIndex, trackSequence, (direction + 2) % 4, height, tileElement);
}

/** rct2: 0x008AD1F0 */
static void paint_miniature_railway_track_right_eighth_to_orthogonal(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    trackSequence = mapLeftEighthTurnToOrthogonal[trackSequence];
    paint_miniature_railway_track_left_eighth_to_diag(
        session, rideIndex, trackSequence, (direction + 3) % 4, height, tileElement);
}

static constexpr const bool miniature_railway_diag_image_segment[][4] = {
    { false, true, false, false },
    { false, false, false, true },
    { false, false, true, false },
    { true, false, false, false },
};

enum
{
    SUPPORT_PLAIN = 0,
    SUPPORT_PLAIN_90_DEG = 1,
    SUPPORT_PLAIN_N = 2,
    SUPPORT_PLAIN_E = 3,
    SUPPORT_PLAIN_S = 4,
    SUPPORT_PLAIN_W = 5,
};

static constexpr const int16_t monorail_diag_support_types[][4] = {
    { -1, SUPPORT_PLAIN_N, SUPPORT_PLAIN_S, SUPPORT_PLAIN },
    { -1, SUPPORT_PLAIN_E, SUPPORT_PLAIN_W, SUPPORT_PLAIN_90_DEG },
    { -1, SUPPORT_PLAIN_S, SUPPORT_PLAIN_N, SUPPORT_PLAIN },
    { -1, SUPPORT_PLAIN_W, SUPPORT_PLAIN_E, SUPPORT_PLAIN_90_DEG },
};

struct floor_desc
{
    uint32_t image_id;
    CoordsXY bound_size;
    CoordsXY bound_offset;
};

static constexpr const floor_desc floors[] = {
    { SPR_FLOOR_PLANKS, { 32, 32 }, { 0, 0 } },
    { SPR_FLOOR_PLANKS_90_DEG, { 32, 32 }, { 0, 0 } }, // -16,-16(1:3)
    { SPR_FLOOR_PLANKS_N_SEGMENT, { 32, 32 }, { -16, -16 } },
    { SPR_FLOOR_PLANKS_E_SEGMENT, { 16, 16 }, { 0, 16 } },
    { SPR_FLOOR_PLANKS_S_SEGMENT, { 16, 16 }, { 16, 16 } },
    { SPR_FLOOR_PLANKS_W_SEGMENT, { 16, 16 }, { 16, 0 } },
};

/** rct2: 0x008AD200 */
static void miniature_railway_track_diag_flat(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    bool isSupported = false;
    int16_t supportType = monorail_diag_support_types[direction][trackSequence];

    uint32_t floorImage = 0;
    CoordsXY floorBoundSize;
    CoordsXY floorBoundOffset;

    if (supportType != -1)
    {
        floorImage = floors[supportType].image_id;
        floorBoundSize = floors[supportType].bound_size;
        floorBoundOffset = floors[supportType].bound_offset;
        isSupported = wooden_a_supports_paint_setup(
            session, supportType, 0, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }

    if (direction == 1 && trackSequence == 3)
    {
        floorBoundOffset = { -16, -16 };
    }

    uint32_t imageId = miniature_railway_track_pieces_diag_flat[direction];
    bool drawRail = miniature_railway_diag_image_segment[direction][trackSequence];

    if (isSupported)
    {
        sub_98197C(
            session, floorImage | session->TrackColours[SCHEME_SUPPORTS], 0, 0, floorBoundSize.x, floorBoundSize.y,
            (drawRail ? 2 : 0), height, floorBoundOffset.x, floorBoundOffset.y, height);
        if (drawRail)
        {
            sub_98199C(session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16, height);
        }
    }
    else if (drawRail)
    {
        sub_98197C(session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16, height);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 32, 0x20);
}

enum
{
    WOOD_A = 0,
    WOOD_B = 1,
};

static bool wooden_supports_paint_setup(
    paint_session* session, int32_t woodType, int32_t supportType, int32_t special, int32_t height, uint32_t imageColourFlags,
    bool* underground)
{
    switch (woodType)
    {
        case WOOD_A:
            return wooden_a_supports_paint_setup(session, supportType, special, height, imageColourFlags, underground);

        case WOOD_B:
            return wooden_b_supports_paint_setup(session, supportType, special, height, imageColourFlags, underground);
    }

    return false;
}

/** rct2: 0x008AD230 */
static void miniature_railway_track_diag_25_deg_up(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    // TODO: The supports are inconsistent for different rotations

    int8_t supportFunction = WOOD_B;
    int16_t supportType = monorail_diag_support_types[direction][trackSequence];
    if (supportType == SUPPORT_PLAIN || supportType == SUPPORT_PLAIN_90_DEG)
    {
        supportFunction = WOOD_A;
    }

    int8_t heightDiffs[] = { +8, -8, +8, -8 };
    int8_t heightDiff = heightDiffs[direction];
    if (trackSequence == 3)
    {
        heightDiff = 8;
    }

    bool hasSupports = false;

    uint32_t floorImage = 0;
    CoordsXY floorBoundSize;
    CoordsXY floorBoundOffset;

    if (supportType != -1)
    {
        floorImage = floors[supportType].image_id;
        floorBoundSize = floors[supportType].bound_size;
        floorBoundOffset = floors[supportType].bound_offset;
        hasSupports = wooden_supports_paint_setup(
            session, supportFunction, supportType, 0, height + heightDiff, session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }

    if (direction == 1 && trackSequence == 3)
    {
        floorBoundOffset = { -16, -16 };
    }

    static constexpr const int8_t offsetsB[4][4][2] = {
        { { 0, 0 }, { +8, +16 }, { +8, +8 }, { +8, +8 } },
        { { 0, 0 }, { -8, -8 }, { -8, -8 }, { +8, +8 } },
        { { 0, 0 }, { +8, +8 }, { +8, +16 }, { +8, +8 } },
        { { 0, 0 }, { -8, -8 }, { -8, -8 }, { +8, +8 } },
    };

    uint32_t imageId = miniature_railway_track_pieces_diag_25_deg_up[direction];
    bool drawRail = miniature_railway_diag_image_segment[direction][trackSequence];
    static constexpr const int8_t offsetB[] = { +8, 0, +8, +8 };
    if (hasSupports)
    {
        sub_98197C(
            session, floorImage | session->TrackColours[SCHEME_SUPPORTS], 0, 0, floorBoundSize.x, floorBoundSize.y,
            (drawRail ? 2 : 0), height + offsetsB[direction][trackSequence][0], floorBoundOffset.x, floorBoundOffset.y,
            height + offsetsB[direction][trackSequence][1]);
        if (drawRail)
        {
            sub_98199C(
                session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16,
                height + offsetB[direction]);
        }
    }
    else if (drawRail)
    {
        sub_98197C(
            session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16,
            height + offsetB[direction]);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 56, 0x20);
}

/** rct2: 0x008AD210 */
static void miniature_railway_track_diag_flat_to_25_deg_up(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    bool hasSupports = false;

    uint32_t floorImage = 0;
    CoordsXY floorBoundSize;
    CoordsXY floorBoundOffset;

    int16_t supportType = monorail_diag_support_types[direction][trackSequence];
    if (supportType != -1)
    {
        floorImage = floors[supportType].image_id;
        floorBoundSize = floors[supportType].bound_size;
        floorBoundOffset = floors[supportType].bound_offset;
        hasSupports = wooden_a_supports_paint_setup(
            session, supportType, 0, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }

    if (direction == 1 && trackSequence == 3)
    {
        floorBoundOffset = CoordsXY(-16, -16);
    }

    uint32_t imageId = miniature_railway_track_pieces_diag_flat_to_25_deg_up[direction];
    bool drawRail = miniature_railway_diag_image_segment[direction][trackSequence];

    if (hasSupports)
    {
        sub_98197C(
            session, floorImage | session->TrackColours[SCHEME_SUPPORTS], 0, 0, floorBoundSize.x, floorBoundSize.y,
            (drawRail ? 2 : 0), height, floorBoundOffset.x, floorBoundOffset.y, height);
        if (drawRail)
        {
            sub_98199C(session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16, height);
        }
    }
    else if (drawRail)
    {
        sub_98197C(session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16, height);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 48, 0x20);
}

/** rct2: 0x008AD220 */
static void miniature_railway_track_diag_25_deg_up_to_flat(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    const int8_t supportOffsets[][4] = {
        { 0, +8, +8, +8 },
        { 0, -8, -8, 0 },
        { 0, +8, +8, +8 },
        { 0, -8, -8, +8 },
    };

    int8_t supportFunction = WOOD_B;
    if (trackSequence == 3)
    {
        supportFunction = WOOD_A;
    }

    bool hasSupports = false;
    int16_t supportType = monorail_diag_support_types[direction][trackSequence];

    uint32_t floorImage = 0;
    CoordsXY floorBoundSize;
    CoordsXY floorBoundOffset;

    if (supportType != -1)
    {
        floorImage = floors[supportType].image_id;
        floorBoundSize = floors[supportType].bound_size;
        floorBoundOffset = floors[supportType].bound_offset;
        hasSupports = wooden_supports_paint_setup(
            session, supportFunction, supportType, 0, height + supportOffsets[direction][trackSequence],
            session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }

    if (direction == 1 && trackSequence == 3)
    {
        floorBoundOffset = CoordsXY(-16, -16);
    }

    const int8_t offsetsB[4][4][2] = {
        { { 0, 0 }, { +8, +16 }, { +8, +8 }, { +8, +8 } },
        { { 0, 0 }, { -8, -8 }, { -8, -8 }, { +8, +8 } },
        { { 0, 0 }, { +8, +8 }, { +8, +16 }, { +8, +8 } },
        { { 0, 0 }, { -8, -8 }, { -8, -8 }, { +8, +8 } },
    };

    uint32_t imageId = miniature_railway_track_pieces_diag_25_deg_up_to_flat[direction];
    bool drawRail = miniature_railway_diag_image_segment[direction][trackSequence];

    const int8_t railOffsets[] = { +8, 0, +8, +8 };

    if (hasSupports)
    {
        sub_98197C(
            session, floorImage | session->TrackColours[SCHEME_SUPPORTS], 0, 0, floorBoundSize.x, floorBoundSize.y,
            (drawRail ? 2 : 0), height + offsetsB[direction][trackSequence][0], floorBoundOffset.x, floorBoundOffset.y,
            height + offsetsB[direction][trackSequence][1]);
        if (drawRail)
        {
            sub_98199C(
                session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16,
                height + railOffsets[direction]);
        }
    }
    else if (drawRail)
    {
        sub_98197C(
            session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16,
            height + railOffsets[direction]);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 56, 0x20);
}

/** rct2: 0x008AD260 */
static void miniature_railway_track_diag_25_deg_down(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    const int8_t supportOffsets[][4] = {
        { 0, +16, +16, 0 },
        { 0, -8, -8, 0 },
        { 0, +16, +16, 0 },
        { 0, -8, -8, -8 },
    };

    int8_t supportFunction = WOOD_B;
    if (trackSequence == 3)
    {
        supportFunction = WOOD_A;
    }

    bool hasSupports = false;

    uint32_t floorImage = 0;
    CoordsXY floorBoundSize;
    CoordsXY floorBoundOffset;
    int16_t supportType = monorail_diag_support_types[direction][trackSequence];

    if (supportType != -1)
    {
        floorImage = floors[supportType].image_id;
        floorBoundSize = floors[supportType].bound_size;
        floorBoundOffset = floors[supportType].bound_offset;
        hasSupports = wooden_supports_paint_setup(
            session, supportFunction, supportType, 0, height + supportOffsets[direction][trackSequence],
            session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }

    if (direction == 1 && trackSequence == 3)
    {
        floorBoundOffset = CoordsXY(-16, -16);
    }

    uint32_t imageId = miniature_railway_track_pieces_diag_25_deg_up[(direction + 2) % 4];
    bool drawRail = miniature_railway_diag_image_segment[direction][trackSequence];
    const int8_t railOffsets[] = { 0, +8, +8, +8 };

    const int8_t offsetsB[4][4][2] = {
        { { 0, 0 }, { +8, +8 }, { +8, +8 }, { -8, -8 } },
        { { 0, 0 }, { -8, -8 }, { -8, -8 }, { -8, 0 } },
        { { 0, 0 }, { +8, +8 }, { +8, +16 }, { -8, -8 } },
        { { 0, 0 }, { -8, -8 }, { -8, -8 }, { -8, -8 } },
    };

    if (hasSupports)
    {
        sub_98197C(
            session, floorImage | session->TrackColours[SCHEME_SUPPORTS], 0, 0, floorBoundSize.x, floorBoundSize.y,
            (drawRail ? 2 : 0), height + offsetsB[direction][trackSequence][0], floorBoundOffset.x, floorBoundOffset.y,
            height + offsetsB[direction][trackSequence][1]);
        if (drawRail)
        {
            sub_98199C(
                session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16,
                height + railOffsets[direction]);
        }
    }
    else if (drawRail)
    {
        sub_98197C(
            session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16,
            height + railOffsets[direction]);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 56, 0x20);
}

/** rct2: 0x008AD240 */
static void miniature_railway_track_diag_flat_to_25_deg_down(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    const int8_t supportOffsets[][4] = {
        { 0, +16, +16, 0 },
        { 0, -8, -8, -8 },
        { 0, +8, +8, 0 },
        { 0, -8, -8, -8 },
    };

    int8_t supportFunction = WOOD_B;
    if (trackSequence == 3)
    {
        supportFunction = WOOD_A;
    }

    bool hasSupports = false;
    uint32_t floorImage = 0;
    CoordsXY floorBoundSize;
    CoordsXY floorBoundOffset;
    int16_t supportType = monorail_diag_support_types[direction][trackSequence];

    if (supportType != -1)
    {
        floorImage = floors[supportType].image_id;
        floorBoundSize = floors[supportType].bound_size;
        floorBoundOffset = floors[supportType].bound_offset;
        hasSupports = wooden_supports_paint_setup(
            session, supportFunction, supportType, 0, height + supportOffsets[direction][trackSequence],
            session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }

    if (direction == 1 && trackSequence == 3)
    {
        floorBoundOffset = CoordsXY(-16, -16);
    }

    const int8_t offsetsB[4][4][2] = {
        { { 0, 0 }, { +8, +8 }, { +8, +8 }, { -8, -8 } },
        { { 0, 0 }, { -8, -8 }, { -8, -8 }, { -8, 0 } },
        { { 0, 0 }, { +8, +8 }, { +8, +16 }, { -8, -8 } },
        { { 0, 0 }, { -8, -8 }, { -8, -8 }, { -8, -8 } },
    };

    uint32_t imageId = miniature_railway_track_pieces_diag_25_deg_up_to_flat[(direction + 2) % 4];
    bool drawRail = miniature_railway_diag_image_segment[direction][trackSequence];
    const int8_t railOffsets[] = { 0, +8, +8, +8 };

    if (hasSupports)
    {
        sub_98197C(
            session, floorImage | session->TrackColours[SCHEME_SUPPORTS], 0, 0, floorBoundSize.x, floorBoundSize.y,
            (drawRail ? 2 : 0), height + offsetsB[direction][trackSequence][0], floorBoundOffset.x, floorBoundOffset.y,
            height + offsetsB[direction][trackSequence][1]);
        if (drawRail)
        {
            sub_98199C(
                session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16,
                height + railOffsets[direction]);
        }
    }
    else if (drawRail)
    {
        sub_98197C(
            session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16,
            height + railOffsets[direction]);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 56, 0x20);
}

static void miniature_railway_track_diag_25_deg_down_to_flat(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    bool hasSupports = false;
    uint32_t floorImage = 0;
    CoordsXY floorBoundSize;
    CoordsXY floorBoundOffset;
    int16_t supportType = monorail_diag_support_types[direction][trackSequence];

    if (supportType != -1)
    {
        floorImage = floors[supportType].image_id;
        floorBoundSize = floors[supportType].bound_size;
        floorBoundOffset = floors[supportType].bound_offset;
        hasSupports = wooden_a_supports_paint_setup(
            session, supportType, 0, height, session->TrackColours[SCHEME_SUPPORTS], nullptr);
    }

    if (direction == 1 && trackSequence == 3)
    {
        floorBoundOffset = CoordsXY(-16, -16);
    }

    uint32_t imageId = miniature_railway_track_pieces_diag_flat_to_25_deg_up[(direction + 2) % 4];
    bool drawRail = miniature_railway_diag_image_segment[direction][trackSequence];

    if (hasSupports)
    {
        sub_98197C(
            session, floorImage | session->TrackColours[SCHEME_SUPPORTS], 0, 0, floorBoundSize.x, floorBoundSize.y,
            (drawRail ? 2 : 0), height, floorBoundOffset.x, floorBoundOffset.y, height);
        if (drawRail)
        {
            sub_98199C(session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16, height);
        }
    }
    else if (drawRail)
    {
        sub_98197C(session, imageId | session->TrackColours[SCHEME_TRACK], -16, -16, 32, 32, 2, height, -16, -16, height);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 48, 0x20);
}

/**
 * rct2: 0x008ACE48
 */
TRACK_PAINT_FUNCTION get_track_paint_function_miniature_railway(int32_t trackType, int32_t direction)
{
    switch (trackType)
    {
        case TRACK_ELEM_FLAT:
            return paint_miniature_railway_track_flat;

        case TRACK_ELEM_END_STATION:
        case TRACK_ELEM_BEGIN_STATION:
        case TRACK_ELEM_MIDDLE_STATION:
            return paint_miniature_railway_station;

        case TRACK_ELEM_25_DEG_UP:
            return paint_miniature_railway_track_25_deg_up;
        case TRACK_ELEM_FLAT_TO_25_DEG_UP:
            return paint_miniature_railway_track_flat_to_25_deg_up;
        case TRACK_ELEM_25_DEG_UP_TO_FLAT:
            return paint_miniature_railway_track_25_deg_up_to_flat;

        case TRACK_ELEM_25_DEG_DOWN:
            return paint_miniature_railway_track_25_deg_down;
        case TRACK_ELEM_FLAT_TO_25_DEG_DOWN:
            return paint_miniature_railway_track_flat_to_25_deg_down;
        case TRACK_ELEM_25_DEG_DOWN_TO_FLAT:
            return paint_miniature_railway_track_25_deg_down_to_flat;

        case TRACK_ELEM_LEFT_QUARTER_TURN_5_TILES:
            return paint_miniature_railway_track_left_quarter_turn_5_tiles;
        case TRACK_ELEM_RIGHT_QUARTER_TURN_5_TILES:
            return paint_miniature_railway_track_right_quarter_turn_5_tiles;

        case TRACK_ELEM_S_BEND_LEFT:
            return paint_miniature_railway_track_s_bend_left;
        case TRACK_ELEM_S_BEND_RIGHT:
            return paint_miniature_railway_track_s_bend_right;

        case TRACK_ELEM_LEFT_QUARTER_TURN_3_TILES:
            return paint_miniature_railway_track_left_quarter_turn_3_tiles;
        case TRACK_ELEM_RIGHT_QUARTER_TURN_3_TILES:
            return paint_miniature_railway_track_right_quarter_turn_3_tiles;

        case TRACK_ELEM_LEFT_EIGHTH_TO_DIAG:
            return paint_miniature_railway_track_left_eighth_to_diag;
        case TRACK_ELEM_RIGHT_EIGHTH_TO_DIAG:
            return paint_miniature_railway_track_right_eighth_to_diag;
        case TRACK_ELEM_LEFT_EIGHTH_TO_ORTHOGONAL:
            return paint_miniature_railway_track_left_eighth_to_orthogonal;
        case TRACK_ELEM_RIGHT_EIGHTH_TO_ORTHOGONAL:
            return paint_miniature_railway_track_right_eighth_to_orthogonal;

        case TRACK_ELEM_DIAG_FLAT:
            return miniature_railway_track_diag_flat;
        case TRACK_ELEM_DIAG_25_DEG_UP:
            return miniature_railway_track_diag_25_deg_up;
        case TRACK_ELEM_DIAG_FLAT_TO_25_DEG_UP:
            return miniature_railway_track_diag_flat_to_25_deg_up;
        case TRACK_ELEM_DIAG_25_DEG_UP_TO_FLAT:
            return miniature_railway_track_diag_25_deg_up_to_flat;
        case TRACK_ELEM_DIAG_25_DEG_DOWN:
            return miniature_railway_track_diag_25_deg_down;
        case TRACK_ELEM_DIAG_FLAT_TO_25_DEG_DOWN:
            return miniature_railway_track_diag_flat_to_25_deg_down;
        case TRACK_ELEM_DIAG_25_DEG_DOWN_TO_FLAT:
            return miniature_railway_track_diag_25_deg_down_to_flat;
    }

    return nullptr;
}
