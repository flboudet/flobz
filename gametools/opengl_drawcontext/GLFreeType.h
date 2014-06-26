#ifndef FREE_NEHE_H
#define FREE_NEHE_H

#include <map>
#include <vector>
#include "config.h"

#ifdef HAVE_GL_GL_H
#include <GL/gl.h>
#elif defined (HAVE_OPENGL_GL_H)
#include <OpenGL/gl.h>
#elif defined (IOS)
#include <OpenGLES/ES1/gl.h>
#elif defined (ANDROID)
#include <GLES/gl.h>
#endif

//FreeType Headers
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H

namespace flobopop {

    struct GlyphData {
        GlyphData();
        ~GlyphData();
        GLuint texture;
        float letter_width;
        float texcoord[8];
        float vertices[8];
        unsigned short faces[4];
    };

    class GLFont {

    private:
        FT_Library library;
        FT_Face face;

        float m_resMultiplier;
        float m_glyphH;     ///< Holds the height of the font for the glyphes.
        float m_renderH;    ///< Holds the height of the font for the rendering.

        //GLuint *textures;	///< Holds the texture id's
        GLuint  list_base;	///< Holds the first display list id
        //float letter_width[256];

        //float texcoord[2048]; // 256*8
        //float vertices[2048];
        //unsigned short faces[1024]; // 256*4
        GlyphData asciiGlyphes[256];
        typedef std::map<unsigned int, GlyphData *> GlyphMap;
        GlyphMap nonAsciiGlyphes;
        typedef std::vector<unsigned short> Line;
        typedef std::vector<Line> MultiLine;

        // Font file data
        void *m_data;

        void make_dlist ( FT_Face face, unsigned short ch, unsigned int h, GlyphData &glyphData);

        inline GlyphData & getGlyphData(unsigned short ch) {
            //if (ch < 256)
            //    return asciiGlyphes[ch];
            GlyphMap::iterator iter = nonAsciiGlyphes.find(ch);
            if (iter != nonAsciiGlyphes.end())
                return *(iter->second);
            GlyphData *newGlyph = new GlyphData();
            make_dlist(face, ch, m_glyphH, *newGlyph);
            nonAsciiGlyphes[ch] = newGlyph;
            return *newGlyph;
        }

        inline void textToLines(const unsigned short *text, MultiLine &lines) {
            //Here is some code to split the text that we have been
            //given into a set of lines.
            //This could be made much neater by using
            //a regular expression library such as the one avliable from
            //boost.org (I've only done it out by hand to avoid complicating
            //this tutorial with unnecessary library dependencies).
            const unsigned short *start_line=text;
            const unsigned short *c;
            for(c=text;*c;c++) {
                if(*c=='\n') {
                    Line line;
                    for(const unsigned short *n=start_line;n<c;n++) line.push_back(*n);
                    lines.push_back(line);
                    start_line=c+1;
                }
            }
            if(start_line) {
                Line line;
                for(const unsigned short *n=start_line;n<c;n++) line.push_back(*n);
                lines.push_back(line);
            }
        }

    public:
        /**
         * Create a GLFont from in-memory data.
         * Note: data must be malloced, and GLFont takes ownership of this pointer.
         * Don't free() data on your own!
         */
        GLFont(void *data, int size, unsigned int h, float resMultiplier = 1., float letter_spacing = 0.) {
            init(data, size, h, resMultiplier, letter_spacing);
        }
        GLFont() {}
        ~GLFont() {}

        float letter_spacing;

        //The init function will create a font of
        //of the height h from the file fname.
        void init(void *data, int size, unsigned int h, float resMultiplier, float letter_spacing);

        //Free all the resources assosiated with the font.
        void clean();

        //The flagship function of the library - this thing will print
        //out text at window coordinates x,y, using the font ft_font.
        //The current modelview matrix will also be applied to the text.
        void printUnicode(float x, float y, const unsigned short *text) ;
        void printCenteredUnicode(float x, float y, const unsigned short *text);
        float getWidthUnicode(const unsigned short *text);
    };

    extern int GLFONT_SCREEN_HEIGHT; //Hack...
}

#endif
