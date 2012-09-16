extern "C" {
#include <png.h>
#include <jpeglib.h>
}
#include "StandardTextureLoader.h"
#include "GTLog.h"

static __inline__ int power_of_2(int input)
{
    int value = 1;
    while (value < input) {
        value <<= 1;
    }
    return value;
}

static inline int utf16_to_utf8(int src, char *dest)
{
    char byte[4];
    int i, nbytes;

    if (src < 0x80) {
        nbytes = 1;
        byte[0] = src;
    } else if (src < 0x800) {
        nbytes = 2;
        byte[1] = (src & 0x3f) | 0x80;
        byte[0] = ((src << 2) & 0xcf00 | 0xc000) >> 8;
    } else {
        nbytes = 3;
        byte[2] = (src & 0x3f) | 0x80;
        byte[1] = ((src << 2) & 0x3f00 | 0x8000) >> 8;
        byte[0] = ((src << 4) & 0x3f0000 | 0xe00000) >> 16;
    }

    for (i = nbytes; i < 4; i++)
        byte[i] = 0;

    strcpy(dest, byte);
    return nbytes;
}

static inline int utf8_to_utf16(const unsigned char * &src)
{
    const unsigned char *utf8 = src;
    if (utf8[0] < 0x80) {
        src++;
        return utf8[0];
    }
    else if (utf8[0] < 0xC2) {
        GTLogTrace("Bad Unicode!");
        src++;
        return '.';
    }
    else if (utf8[0] < 0xE0) {
        src += 2;
        return (((int)utf8[0] & 0x1F) << 6) | ((int)utf8[1] & 0x3F);
    }
    else if (utf8[0] < 0xF0) {
        src += 3;
        return (((int)utf8[0] & 0x1F) << 12) | (((int)utf8[1] & 0x3F) << 6) | ((int)utf8[2] & 0x3F);
    }
}

StandardTextureLoader::StandardTextureLoader(DataPathManager *dataPathManager)
    : m_dataPathManager(dataPathManager)
{
}

unsigned short * StandardTextureLoader::utf8ToUnicode(const char *utf8Text,
                                                        unsigned short *unicodeTextBuffer,
                                                        size_t unicodeTextBufferSize)
{
    const unsigned char *wb = (const unsigned char *)utf8Text;
    unsigned short *rb = unicodeTextBuffer;
    while (*wb != 0) {
        *rb = (unsigned short)(utf8_to_utf16(wb));
        ++rb;
    }
    *rb = 0;
    return unicodeTextBuffer;
}

OpenGLRawImage * StandardTextureLoader::loadImage(ImageType type, const char *path)
{
    const char *extension = path + strlen(path) - 4;
    if (strcmp(extension, ".png") == 0) {
        OpenGLRawImage * result = loadImagePNG(type, path);
        if (result != NULL)
            return result;
    }
    else if (strcmp(extension, ".jpg") == 0) {
        OpenGLRawImage * result = loadImageJPG(type, path);
        if (result != NULL)
            return result;
    }
    return NULL;
}

static void libpngUserReadData(png_structp pngPtr, png_bytep data, png_size_t length)
{
    DataInputStream *f = (DataInputStream *)(png_get_io_ptr(pngPtr));
    f->streamRead(data, length);
}

OpenGLRawImage * StandardTextureLoader::loadImagePNG(ImageType type, const char *path)
{
    std::auto_ptr<DataInputStream> pngfile(m_dataPathManager->openDataInputStream(path));
    if (pngfile.get() == NULL)
        return NULL;
    png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngPtr) {
        GTLogTrace("ERROR: Couldn't initialize png read struct");
        return NULL;
    }
    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr) {
        GTLogTrace("ERROR: Couldn't initialize png info struct");
        png_destroy_read_struct(&pngPtr, (png_infopp)0, (png_infopp)0);
        return NULL;
    }
    png_set_read_fn(pngPtr,(void *)(pngfile.get()), libpngUserReadData);
    png_read_info(pngPtr, infoPtr);

    png_uint_32 imgWidth =  png_get_image_width(pngPtr, infoPtr);
    png_uint_32 imgHeight = png_get_image_height(pngPtr, infoPtr);
    //bits per CHANNEL! note: not per pixel!
    png_uint_32 bitdepth   = png_get_bit_depth(pngPtr, infoPtr);
    //Number of channels
    png_uint_32 channels   = png_get_channels(pngPtr, infoPtr);
    //Color type. (RGB, RGBA, Luminance, luminance alpha... palette... etc)
    png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);

    // GTLogTrace("Image dimensions: %ux%u chan:%u bitdepth:%u", imgWidth, imgHeight, channels, bitdepth);
    switch (color_type) {
    case PNG_COLOR_TYPE_PALETTE:
        png_set_palette_to_rgb(pngPtr);
        channels = 3;
        break;
    case PNG_COLOR_MASK_PALETTE:
        png_set_palette_to_rgb(pngPtr);
        channels = 4;
        break;
    default:
        break;
    }
    if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(pngPtr);
        channels = 4;
    }
    if (bitdepth < 8) {
        png_set_expand(pngPtr);
        bitdepth = 8;
    }
    if (bitdepth == 16) {
        png_set_strip_16(pngPtr);
        bitdepth = 8;
    }
    png_uint_32 po2Width =  power_of_2(imgWidth);
    png_uint_32 po2Height = power_of_2(imgHeight);
    int rowsize = (po2Width*channels*bitdepth)/8;
    char *imgData = (char *)calloc(rowsize * po2Height, 1);
    png_bytep *rowPtrs = new png_bytep[imgHeight];
    char *dataPtr = imgData;
    for (int i = 0 ; i < imgHeight ; i++) {
        rowPtrs[i] = (png_bytep)dataPtr;
        dataPtr += rowsize;
    }
    png_read_image(pngPtr, rowPtrs);
    delete[] rowPtrs;
#ifdef DISABLED
PNG_COLOR_TYPE_GRAY
                        (bit depths 1, 2, 4, 8, 16)
                     PNG_COLOR_TYPE_GRAY_ALPHA
                        (bit depths 8, 16)
                     PNG_COLOR_TYPE_PALETTE
                        (bit depths 1, 2, 4, 8)
                     PNG_COLOR_TYPE_RGB
                        (bit_depths 8, 16)
                     PNG_COLOR_TYPE_RGB_ALPHA
                        (bit_depths 8, 16)

                     PNG_COLOR_MASK_PALETTE
                     PNG_COLOR_MASK_COLOR
                     PNG_COLOR_MASK_ALPHA
#endif
    png_destroy_read_struct(&pngPtr, (png_infopp)0, (png_infopp)0);
    return new MallocedOpenGLRawImage(channels == 4 ? GL_RGBA : GL_RGB, imgWidth, imgHeight, po2Width, po2Height,
                                      imgData);
}

typedef struct {
  struct jpeg_source_mgr pub;	/* public fields */
  DataInputStream * infile;		/* source stream */
  JOCTET * buffer;		/* start of buffer */
  bool start_of_file;	/* have we gotten any data yet? */
} my_source_mgr;

#define INPUT_BUF_SIZE  4096	/* choose an efficiently fread'able size */

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */
static void init_source (j_decompress_ptr cinfo)
{
    my_source_mgr *src = (my_source_mgr *)cinfo->src;
    /* We reset the empty-input-file flag for each image,
     * but we don't clear the input buffer.
     * This is correct behavior for reading a series of images from one source.
     */
    src->start_of_file = true;
}

static boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    my_source_mgr *src = (my_source_mgr *) cinfo->src;
    size_t nbytes;
    nbytes = src->infile->streamRead(src->buffer, INPUT_BUF_SIZE);
    if (nbytes <= 0) {
        if (src->start_of_file)	/* Treat empty input file as fatal error */
            return FALSE;//ERREXIT(cinfo, JERR_INPUT_EMPTY);
        //WARNMS(cinfo, JWRN_JPEG_EOF);
        /* Insert a fake EOI marker */
        src->buffer[0] = (JOCTET) 0xFF;
        src->buffer[1] = (JOCTET) JPEG_EOI;
        nbytes = 2;
    }
    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_file = FALSE;
    return TRUE;
}

static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    my_source_mgr *src = (my_source_mgr *) cinfo->src;

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (num_bytes > 0) {
        while (num_bytes > (long) src->pub.bytes_in_buffer) {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            (void) fill_input_buffer(cinfo);
            /* note we assume that fill_input_buffer will never return FALSE,
             * so suspension need not be handled.
             */
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

static void term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}

OpenGLRawImage * StandardTextureLoader::loadImageJPG(ImageType type, const char *path)
{
    std::auto_ptr<DataInputStream> jpgfile(m_dataPathManager->openDataInputStream(path));
    if (jpgfile.get() == NULL)
        return NULL;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    // File IO redirection
    my_source_mgr src;
    src.buffer = (JOCTET *)
      (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
                                 INPUT_BUF_SIZE * sizeof(JOCTET));
    src.pub.init_source = init_source;
    src.pub.fill_input_buffer = fill_input_buffer;
    src.pub.skip_input_data = skip_input_data;
    src.pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src.pub.term_source = term_source;
    src.infile = jpgfile.get();
    src.pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    src.pub.next_input_byte = NULL; /* until buffer loaded */
    cinfo.src = (struct jpeg_source_mgr *)(&src);

    jpeg_read_header(&cinfo, TRUE);
    //GTLogTrace("jpeg dimensions:%ux%u", cinfo.image_width, cinfo.image_height);
    png_uint_32 po2Width =  power_of_2(cinfo.image_width);
    png_uint_32 po2Height = power_of_2(cinfo.image_height);
    int rowsize = (po2Width*3);
    char *imgData = (char *)malloc(rowsize * po2Height);
    char *dataPtr = imgData;
    jpeg_start_decompress(&cinfo);
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, (JSAMPARRAY)(&dataPtr), 1);
        dataPtr += rowsize;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return new MallocedOpenGLRawImage(GL_RGB, cinfo.image_width, cinfo.image_height, po2Width, po2Height,
                                      imgData);
}


