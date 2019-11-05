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
#include "../Track.h"
#include "../TrackPaint.h"

enum
{
    SPR_BOAT_HIRE_FLAT_BACK_SW_NE = 28523,
    SPR_BOAT_HIRE_FLAT_FRONT_SW_NE = 28524,
    SPR_BOAT_HIRE_FLAT_BACK_NW_SE = 28525,
    SPR_BOAT_HIRE_FLAT_FRONT_NW_SE = 28526,
    SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_BACK_SW_NW = 28527,
    SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_FRONT_SW_NW = 28528,
    SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_BACK_NW_NE = 28529,
    SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_FRONT_NW_NE = 28530,
    SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_BACK_NE_SE = 28531,
    SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_FRONT_NE_SE = 28532,
    SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_FRONT_SE_SW = 28533,
    SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_BACK_SE_SW = 28534,
};

/** rct2: 0x008B0E40 */
static void paint_boat_hire_track_flat(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    uint32_t imageId;

    if (direction & 1)
    {
        imageId = SPR_BOAT_HIRE_FLAT_BACK_NW_SE | session->TrackColours[SCHEME_TRACK];
        sub_98197C(session, imageId, 0, 0, 1, 32, 3, height, 4, 0, height);

        imageId = SPR_BOAT_HIRE_FLAT_FRONT_NW_SE | session->TrackColours[SCHEME_TRACK];
        sub_98197C(session, imageId, 0, 0, 1, 32, 3, height, 28, 0, height);
    }
    else
    {
        imageId = SPR_BOAT_HIRE_FLAT_BACK_SW_NE | session->TrackColours[SCHEME_TRACK];
        sub_98197C(session, imageId, 0, 0, 32, 1, 3, height, 0, 4, height);

        imageId = SPR_BOAT_HIRE_FLAT_FRONT_SW_NE | session->TrackColours[SCHEME_TRACK];
        sub_98197C(session, imageId, 0, 0, 32, 1, 3, height, 0, 28, height);
    }

    paint_util_set_segment_support_height(
        session, paint_util_rotate_segments(SEGMENT_D0 | SEGMENT_C4 | SEGMENT_CC, direction), 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 16, 0x20);
}

/** rct2: 0x008B0E50 */
static void paint_boat_hire_station(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    auto ride = get_ride(rideIndex);
    if (ride == nullptr)
        return;

    CoordsXY position = session->MapPosition;
    auto stationObj = ride_get_station_object(ride);

    if (direction & 1)
    {
        paint_util_push_tunnel_right(session, height, TUNNEL_6);
        track_paint_util_draw_pier(
            session, ride, stationObj, position, direction, height, tileElement, session->CurrentRotation);
    }
    else
    {
        paint_util_push_tunnel_left(session, height, TUNNEL_6);
        track_paint_util_draw_pier(
            session, ride, stationObj, position, direction, height, tileElement, session->CurrentRotation);
    }

    paint_util_set_segment_support_height(session, SEGMENTS_ALL, 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 32, 0x20);
}

/** rct2: 0x008B0E80 */
static void paint_boat_hire_track_left_quarter_turn_1_tile(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    uint32_t imageId;
    switch (direction)
    {
        case 0:
            imageId = SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_BACK_SW_NW | session->TrackColours[SCHEME_TRACK];
            sub_98197C(session, imageId, 0, 0, 32, 32, 0, height, 0, 0, height);

            imageId = SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_FRONT_SW_NW | session->TrackColours[SCHEME_TRACK];
            sub_98197C(session, imageId, 0, 0, 3, 3, 3, height, 28, 28, height + 2);
            break;
        case 1:
            imageId = SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_BACK_NW_NE | session->TrackColours[SCHEME_TRACK];
            sub_98197C(session, imageId, 0, 0, 32, 32, 0, height, 0, 0, height);

            imageId = SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_FRONT_NW_NE | session->TrackColours[SCHEME_TRACK];
            sub_98197C(session, imageId, 0, 0, 3, 3, 3, height, 28, 28, height + 2);
            break;
        case 2:
            imageId = SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_BACK_NE_SE | session->TrackColours[SCHEME_TRACK];
            sub_98197C(session, imageId, 0, 0, 32, 32, 0, height, 0, 0, height);

            imageId = SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_FRONT_NE_SE | session->TrackColours[SCHEME_TRACK];
            sub_98197C(session, imageId, 0, 0, 3, 3, 3, height, 28, 28, height + 2);
            break;
        case 3:
            imageId = SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_FRONT_SE_SW | session->TrackColours[SCHEME_TRACK];
            sub_98197C(session, imageId, 0, 0, 3, 3, 3, height, 28, 28, height + 2);

            imageId = SPR_BOAT_HIRE_FLAT_QUARTER_TURN_1_TILE_BACK_SE_SW | session->TrackColours[SCHEME_TRACK];
            sub_98197C(session, imageId, 0, 0, 32, 32, 0, height, 0, 0, height);
            break;
    }

    paint_util_set_segment_support_height(
        session, paint_util_rotate_segments(SEGMENT_D0 | SEGMENT_C4 | SEGMENT_C8, direction), 0xFFFF, 0);
    paint_util_set_general_support_height(session, height + 16, 0x20);
}

/** rct2: 0x008B0E90 */
static void paint_boat_hire_track_right_quarter_turn_1_tile(
    paint_session* session, ride_id_t rideIndex, uint8_t trackSequence, uint8_t direction, int32_t height,
    const TileElement* tileElement)
{
    paint_boat_hire_track_left_quarter_turn_1_tile(session, rideIndex, trackSequence, (direction + 3) % 4, height, tileElement);
}

/**
 * rct2: 0x008B0D60
 */
TRACK_PAINT_FUNCTION get_track_paint_function_boat_hire(int32_t trackType, int32_t direction)
{
    switch (trackType)
    {
        case TRACK_ELEM_FLAT:
            return paint_boat_hire_track_flat;

        case TRACK_ELEM_END_STATION:
        case TRACK_ELEM_BEGIN_STATION:
        case TRACK_ELEM_MIDDLE_STATION:
            return paint_boat_hire_station;

        case TRACK_ELEM_LEFT_QUARTER_TURN_1_TILE:
            return paint_boat_hire_track_left_quarter_turn_1_tile;
        case TRACK_ELEM_RIGHT_QUARTER_TURN_1_TILE:
            return paint_boat_hire_track_right_quarter_turn_1_tile;
    }

    return nullptr;
}
