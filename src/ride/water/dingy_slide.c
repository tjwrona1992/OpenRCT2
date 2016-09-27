#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include "../../interface/viewport.h"
#include "../../paint/paint.h"
#include "../../paint/supports.h"
#include "../../world/sprite.h"
#include "../track.h"
#include "../track_paint.h"

enum {
	SPR_DINGHY_SLIDE_FLAT_SW_NE = 19720,
	SPR_DINGHY_SLIDE_FLAT_NW_SE = 19721,
	SPR_DINGHY_SLIDE_FLAT_FRONT_SW_NE = 19722,
	SPR_DINGHY_SLIDE_FLAT_FRONT_NW_SE = 19723,
	SPR_DINGHY_SLIDE_FLAT_CHAIN_SW_NE = 19724,
	SPR_DINGHY_SLIDE_FLAT_CHAIN_NW_SE = 19725,
	SPR_DINGHY_SLIDE_FLAT_CHAIN_FRONT_SW_NE = 19726,
	SPR_DINGHY_SLIDE_FLAT_CHAIN_FRONT_NW_SE = 19727,
};

static void dinghy_slide_track_flat(uint8 rideIndex, uint8 trackSequence, uint8 direction, int height, rct_map_element * mapElement)
{
	static const uint32 imageIds[2][4][2] = {
		{
			{ SPR_DINGHY_SLIDE_FLAT_SW_NE, SPR_DINGHY_SLIDE_FLAT_FRONT_SW_NE },
			{ SPR_DINGHY_SLIDE_FLAT_NW_SE, SPR_DINGHY_SLIDE_FLAT_FRONT_NW_SE },
			{ SPR_DINGHY_SLIDE_FLAT_SW_NE, SPR_DINGHY_SLIDE_FLAT_FRONT_SW_NE },
			{ SPR_DINGHY_SLIDE_FLAT_NW_SE, SPR_DINGHY_SLIDE_FLAT_FRONT_NW_SE },
		},
		{
			{ SPR_DINGHY_SLIDE_FLAT_CHAIN_SW_NE, SPR_DINGHY_SLIDE_FLAT_CHAIN_FRONT_SW_NE },
			{ SPR_DINGHY_SLIDE_FLAT_CHAIN_NW_SE, SPR_DINGHY_SLIDE_FLAT_CHAIN_FRONT_NW_SE },
			{ SPR_DINGHY_SLIDE_FLAT_CHAIN_SW_NE, SPR_DINGHY_SLIDE_FLAT_CHAIN_FRONT_SW_NE },
			{ SPR_DINGHY_SLIDE_FLAT_CHAIN_NW_SE, SPR_DINGHY_SLIDE_FLAT_CHAIN_FRONT_NW_SE },
		},
	};

	bool isChained = track_element_is_lift_hill(mapElement);
	uint32 imageId = imageIds[isChained][direction][0] | gTrackColours[SCHEME_TRACK];
	sub_98197C_rotated(direction, imageId, 0, 0, 32, 20, 2, height, 0, 6, height);

	imageId = imageIds[isChained][direction][1] | gTrackColours[SCHEME_TRACK];
	sub_98197C_rotated(direction, imageId, 0, 0, 32, 1, 26, height, 0, 27, height);

	if (track_paint_util_should_paint_supports(gPaintMapPosition)) {
		metal_a_supports_paint_setup(0, 4, 0, height, gTrackColours[SCHEME_SUPPORTS]);
	}

	paint_util_push_tunnel_rotated(direction, height, TUNNEL_0);

	paint_util_set_segment_support_height(paint_util_rotate_segments(SEGMENT_D0 | SEGMENT_C4 | SEGMENT_CC, direction), 0xFFFF, 0);
	paint_util_set_general_support_height(height + 32, 0x20);
}

TRACK_PAINT_FUNCTION get_track_paint_function_dinghy_slide(int trackType, int direction)
{
	switch (trackType) {
	case TRACK_ELEM_FLAT:
		return dinghy_slide_track_flat;
	}

	return NULL;
}
