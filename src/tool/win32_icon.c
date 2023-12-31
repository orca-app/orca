#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb_image_write.h"

typedef struct
{
    u16 reserved_zero; // must always be zero
    u16 image_type;    // 1 for icon (.ICO) image, 2 for cursor (.CUR) image. Other values are invalid.
    u16 num_images;    // Specifies number of images in the file.
} IcoHeader;

typedef struct
{
    u8 image_width;
    u8 image_height;
    u8 num_colors_in_palette; // should be 0 if the image does not use a color palette
    u8 reserved_zero;         // must be 0
    u16 color_planes;         // should be 0 or 1
    u16 bits_per_pixel;
    u32 image_size_in_bytes;
    u32 offset; // offset of the bmp or png data from the beginning of the ico file
} IcoEntry;

bool icon_from_image(oc_arena* a, oc_str8 image_path, oc_str8 ico_path)
{
    bool result = true;
    oc_arena_scope scratch = oc_scratch_begin_next(a);

    i32 og_width, og_height, og_channels;
    char* filename = oc_str8_to_cstring(scratch.arena, image_path);
    u8* og_image_data = stbi_load(filename, &og_width, &og_height, &og_channels, STBI_rgb_alpha);

    u32 sizes[] = { 16, 32, 48, 256 };
    i32 num_sizes = oc_array_size(sizes);
    u8* images[oc_array_size(sizes)] = { 0 };
    i32 image_sizes_bytes[oc_array_size(sizes)] = { 0 };

    IcoHeader header = { 0 };
    header.image_type = 1;
    header.num_images = (u16)num_sizes;

    oc_str8 input_dir = oc_path_slice_directory(image_path);
    oc_file ico_file = oc_file_open(ico_path, OC_FILE_ACCESS_WRITE, OC_FILE_OPEN_CREATE);
    if(oc_file_last_error(ico_file) != OC_IO_OK)
    {
        result = false;
        goto cleanup;
    }

    //-----------------------------------------------------------------------------
    // Write .ico header
    //-----------------------------------------------------------------------------
    oc_file_write(ico_file, sizeof(header), (u8*)&header);
    if(oc_file_last_error(ico_file) != OC_IO_OK)
    {
        result = false;
        goto cleanup;
    }

    //-----------------------------------------------------------------------------
    // Generate images of each size
    //-----------------------------------------------------------------------------
    for(i32 i = 0; i < num_sizes; ++i)
    {
        u32 size = sizes[i];
        u8* image_data = stbir_resize_uint8_linear(
            og_image_data, og_width, og_height, 0,
            0, size, size, 0,
            STBIR_RGBA);
        if(!image_data)
        {
            result = false;
            goto cleanup;
        }
        images[i] = stbi_write_png_to_mem(image_data,
                                          size * STBI_rgb_alpha,
                                          size, size, STBI_rgb_alpha,
                                          &image_sizes_bytes[i]);
        free(image_data);
        if(!images[i])
        {
            result = false;
            goto cleanup;
        }
    }

    //-----------------------------------------------------------------------------
    // Write icon directory entries
    //-----------------------------------------------------------------------------
    u32 data_offset = sizeof(IcoHeader) + (num_sizes * sizeof(IcoEntry));
    for(i32 i = 0; i < num_sizes; ++i)
    {
        u32 size = sizes[i];
        u8 entry_size = size == 256 ? 0 : (u8)size; // NOTE(shaw): 0 means 256 pixels in IcoEntry
        IcoEntry entry = {
            .image_width = entry_size,
            .image_height = entry_size,
            .num_colors_in_palette = 0,
            .color_planes = 1,
            .bits_per_pixel = 32,
            .image_size_in_bytes = image_sizes_bytes[i],
            .offset = data_offset,
        };
        oc_file_write(ico_file, sizeof(entry), (u8*)&entry);
        if(oc_file_last_error(ico_file) != OC_IO_OK)
        {
            result = false;
            goto cleanup;
        }
        data_offset += image_sizes_bytes[i];
    }

    //-----------------------------------------------------------------------------
    // Write image data
    //-----------------------------------------------------------------------------
    for(i32 i = 0; i < num_sizes; ++i)
    {
        oc_file_write(ico_file, image_sizes_bytes[i], images[i]);
        if(oc_file_last_error(ico_file) != OC_IO_OK)
        {
            result = false;
            goto cleanup;
        }
    }

cleanup:
    for(i32 i = 0; i < num_sizes; ++i)
    {
        STBIW_FREE(images[i]);
    }
    oc_scratch_end(scratch);
    oc_file_close(ico_file);
    return result;
}

bool resource_file_from_icon(oc_arena* a, oc_str8 ico_path, oc_str8 res_path)
{
    bool result = true;
    oc_file rc_file = { 0 }, res_file = { 0 };
    oc_arena_scope scratch = oc_scratch_begin_next(a);

    oc_str8 input_dir = oc_path_slice_directory(ico_path);
    oc_str8 rc_path = oc_path_append(scratch.arena, input_dir, OC_STR8("icon.rc"));
    rc_file = oc_file_open(rc_path, OC_FILE_ACCESS_WRITE, OC_FILE_OPEN_CREATE);
    if(oc_file_last_error(rc_file) != OC_IO_OK)
    {
        result = false;
        goto cleanup;
    }

    oc_str8 rc_contents = OC_STR8("orca_application_icon ICON icon.ico");
    oc_file_write(rc_file, rc_contents.len, rc_contents.ptr);
    if(oc_file_last_error(rc_file) != OC_IO_OK)
    {
        result = false;
        goto cleanup;
    }

    oc_str8 cmd = oc_str8_pushf(scratch.arena, "rc.exe /nologo /fo %.*s %s",
                                oc_str8_ip(res_path), rc_path.ptr);
    int return_code = system(cmd.ptr);
    if(return_code)
    {
        result = false;
        goto cleanup;
    }

cleanup:
    oc_file_close(rc_file);
    oc_file_close(res_file);
    remove(rc_path.ptr);
    oc_scratch_end(scratch);
    return result;
}
