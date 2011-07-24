/*
   A quick and simple opengl font library that uses GNU freetype2, written
   and distributed as part of a tutorial for nehe.gamedev.net.
   Sven Olsen, 2003
   */

//Include our header file.
#include "GLFreeType.h"

//Some STL headers
#include <vector>
#include <string>

//Using the STL exception library increases the
//chances that someone else using our code will corretly
//catch any exceptions that we throw.
#include <stdexcept>

#define GL_GET_ERROR() \
while(1) {\
    GLenum err = glGetError(); \
    if (err != GL_NO_ERROR) \
       fprintf(stderr, "OpenGL Error: %s:%d, %d\n", __FILE__, __LINE__, err); \
    break; \
}

namespace flobopop {

int GLFONT_SCREEN_HEIGHT;
float GLFONT_letter_spacing;

//Inside of this namespace, give ourselves the ability
//to write just "vector" instead of "std::vector"
using std::vector;

//Ditto for string.
using std::string;

    ///This function gets the first power of 2 >= the
    ///int that we pass it.
    inline int next_p2 ( int a )
    {
        int rval=1;
        while(rval<a) rval<<=1;
        return rval;
    }

    GlyphData::GlyphData() {
        glGenTextures(1, &texture);
        GL_GET_ERROR();
    }
    GlyphData::~GlyphData() {
        glDeleteTextures(1, &texture);
        GL_GET_ERROR();
    }

    ///Create a display list coresponding to the give character.
    void make_dlist ( FT_Face face, unsigned short ch, unsigned int h, GlyphData &glyphData) {

        //The first thing we do is get FreeType to render our character
        //into a bitmap.  This actually requires a couple of FreeType commands:

        //Load the Glyph for our character.
        if(FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT ))
            throw std::runtime_error("FT_Load_Glyph failed");

        //Move the face's glyph into a Glyph object.
        FT_Glyph glyph;
        if(FT_Get_Glyph( face->glyph, &glyph ))
            throw std::runtime_error("FT_Get_Glyph failed");

        //Convert the glyph to a bitmap.
        FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
        FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

        //This reference will make accessing the bitmap easier
        FT_Bitmap& bitmap=bitmap_glyph->bitmap;

        //Use our helper function to get the widths of
        //the bitmap data that we will need in order to create
        //our texture.
        int width = next_p2( bitmap.width );
        int height = next_p2( bitmap.rows );

        //Allocate memory for the texture data.
        GLubyte* expanded_data = new GLubyte[2 * width * height];

        // Here We Fill In The Data For The Expanded Bitmap.
        // Notice That We Are Using A Two Channel Bitmap (One
        // Channel For Luminosity And One For Alpha Values).
        //
        // We Make All The Luminosity Values White, And Use The Freeytpe Generated Bitmap
        // To Set Up The Alpha Values. Given The Blend Function That We're Going To Use,
        // This Will Make OpenGL Render The Font Properly.
        //
        // We Use The ?: Operator To Say That The Alpha Value Which We Use
        // Will Be 0 If We Are In The Padding Zone, And Whatever
        // Is The FreeType Bitmap Otherwise.
        for(int j=0; j <height;j++) for(int i=0; i < width; i++) {
            expanded_data[2*(i+j*width)] = 255;
            expanded_data[2*(i+j*width)+1] = (i>=bitmap.width || j>=bitmap.rows) ?
              0 : bitmap.buffer[i + bitmap.width*j];
        }
        GL_GET_ERROR();
        //Now we just setup some texture paramaters.
        glBindTexture( GL_TEXTURE_2D, glyphData.texture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        GL_GET_ERROR();
        //Here we actually create the texture itself, notice
        //that we are using GL_LUMINANCE_ALPHA to indicate that
        //we are using 2 channel data.
        glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA/*GL_RGBA*/, width, height,
                      0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );
        GL_GET_ERROR();
        //With the texture created, we don't need to expanded data anymore
        delete [] expanded_data;

        float fx = bitmap_glyph->left;
        float fy = (float)h-bitmap_glyph->top;

        //Now we need to account for the fact that many of
        //our textures are filled with empty padding space.
        //We figure what portion of the texture is used by
        //the actual character and store that information in
        //the x and y variables, then when we draw the
        //quad, we will only reference the parts of the texture
        //that we contain the character itself.
        float	x=(float)bitmap.width / (float)width,
                y=(float)bitmap.rows / (float)height;

        //Here we draw the texturemaped quads.
        //The bitmap that we got from FreeType was not
        //oriented quite like we would like it to be,
        //so we need to link the texture to the quad
        //so that the result will be properly aligned.

        glyphData.texcoord[0] = 0;
        glyphData.texcoord[1] = 0;
        glyphData.texcoord[2] = 0;
        glyphData.texcoord[3] = y;
        glyphData.texcoord[4] = x;
        glyphData.texcoord[5] = y;
        glyphData.texcoord[6] = x;
        glyphData.texcoord[7] = 0;
        glyphData.vertices[0] = fx + 0;
        glyphData.vertices[1] = fy;
        glyphData.vertices[2] = fx + 0;
        glyphData.vertices[3] = fy + bitmap.rows;
        glyphData.vertices[4] = fx + bitmap.width;
        glyphData.vertices[5] = fy + bitmap.rows;
        glyphData.vertices[6] = fx + bitmap.width;
        glyphData.vertices[7] = fy;
        glyphData.faces[0] = 0;
        glyphData.faces[1] = 1;
        glyphData.faces[2] = 3;
        glyphData.faces[3] = 2;

        glyphData.letter_width = /*bitmap_glyph->left + bitmap.width*/ (face->glyph->advance.x >> 6);
        // We are done with the glyph, let's release it
        FT_Done_Glyph(glyph);
    }



    void GLFont::init(void *data, int size, unsigned int h, float letter_spacing) {
        this->letter_spacing = letter_spacing;
        GLFONT_letter_spacing = letter_spacing;

        this->h=h;

        //Create and initilize a freetype font library.
        if (FT_Init_FreeType( &library ))
            throw std::runtime_error("FT_Init_FreeType failed");

        //The object in which Freetype holds information on a given
        //font is called a "face".

        //This is where we load in the font information from the file.
        //Of all the places where the code might die, this is the most likely,
        //as FT_New_Face will die if the font file does not exist or is somehow broken.
#ifdef DISABLED
        if (FT_New_Face( library, fname, 0, &face ))
            throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");
#endif
        if (FT_New_Memory_Face(library, (FT_Byte *)data,
                               size, 0, &face ))
            throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");
        m_data = data;

        //For some twisted reason, Freetype measures font size
        //in terms of 1/64ths of pixels.  Thus, to make a font
        //h pixels high, we need to request a size of h*64.
        //(h << 6 is just a prettier way of writting h*64)
        FT_Set_Char_Size( face, h << 6, h << 6, 96, 96);
    }

    void GLFont::clean() {
        //We don't need the face information now that the display
        //lists have been created, so we free the assosiated resources.
        FT_Done_Face(face);
        free(m_data);
        //Ditto for the library.
        FT_Done_FreeType(library);

        //We have to clean the map of non-ascii characters
        for (GlyphMap::iterator iter = nonAsciiGlyphes.begin() ;
             iter != nonAsciiGlyphes.end() ; iter++) {
            delete (iter->second);
        }
    }

	float GLFont::getWidthUnicode(const unsigned short *text)  {
        if (text == NULL)
            return 0.;
        MultiLine lines;
        textToLines(text, lines);

        float width = 0.0f;
        for(unsigned int i=0;i<lines.size();i++) {
            float w = 0.0f;
            for (unsigned int j=0;j<lines[i].size();++j) {
                w += getGlyphData((unsigned short)lines[i][j]).letter_width;
            }
            if (w > width) width = w;
        }
		return width;
    }

    void GLFont::printCenteredUnicode(float x, float y, const unsigned short *text)  {
        if (text == NULL)
            return;
        MultiLine lines;
        textToLines(text, lines);

        float width = 0.0f;
        for(unsigned int i=0;i<lines.size();i++) {
            float w = 0.0f;
            for (unsigned int j=0;j<lines[i].size();++j) {
                w += getGlyphData((unsigned short)lines[i][j]).letter_width;
            }
            if (w > width) width = w;
        }
        // offset x,y
        // print
        printUnicode(x - width/2.0, y/* - GLFONT_SCREEN_HEIGHT / 2.0f*/, text);
    }
#ifdef DISABLED
    ///Much like Nehe's glPrint function, but modified to work
    ///with freetype fonts.
    void GLFont::print(float x, float y, const wchar_t *fmt, ...)  {
        // GLuint font=this->list_base;
        float h=this->h/.63f;						//We make the height about 1.5* that of

        wchar_t text[256];								// Holds Our String
        va_list		ap;										// Pointer To List Of Arguments

        if (fmt == NULL)									// If There's No Text
            *text=0;											// Do Nothing

        else {
            va_start(ap, fmt);									// Parses The String For Variables
#ifdef WIN32
            vswprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
#else
            vswprintf(text, 256, fmt, ap);						// And Converts Symbols To Actual Numbers
#endif
            va_end(ap);											// Results Are Stored In Text
        }
        printUnicode(x, y, (unsigned short *)text);
    }
#endif
    void GLFont::printUnicode(float x, float y, const unsigned short *text) {
        if (text == NULL)
            return;
        MultiLine lines;
        textToLines(text, lines);

        //glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
        //glMatrixMode(GL_MODELVIEW);

        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        //glTexCoordPointer(2, GL_FLOAT, 0, getGlyphData(0).texcoord);
        //glVertexPointer(2, GL_FLOAT, 0, getGlyphData(0).vertices);

        for(unsigned int i=0;i<lines.size();i++) {
            glPushMatrix();
            glTranslatef(x,y+h*i,0);
            const Line &line = lines[i];
            for (unsigned int j=0; j<line.size(); ++j) {
                unsigned short ch = line[j];
                GlyphData &glyphData = getGlyphData(ch);
                glBindTexture(GL_TEXTURE_2D, glyphData.texture);
                glTexCoordPointer(2, GL_FLOAT, 0, glyphData.texcoord);
                glVertexPointer(2, GL_FLOAT, 0, glyphData.vertices);
                glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, glyphData.faces);
                glTranslatef(glyphData.letter_width,0,0);
            }
            glPopMatrix();
        }
        //glDisableClientState(GL_VERTEX_ARRAY);
        //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_BLEND);
    }

}
