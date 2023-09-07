/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

//------------------------------------------------------------------------
// image length checks
//------------------------------------------------------------------------

u64 orca_image_upload_region_rgba8_length(IM3Runtime runtime, oc_rect rect)
{
    u64 pixelFormatWidth = sizeof(u8) * 4;
    u64 len = rect.w * rect.h * pixelFormatWidth;
    return len;
}
