#ifndef FREE_NEHE_H
#define FREE_NEHE_H

#include <map>
#include <vector>

#ifdef IOS
#import <OpenGLES/ES1/gl.h>
#else
#include <GL/gl.h>
#endif

//FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

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

    void make_dlist ( FT_Face face, unsigned short ch, unsigned int h, GlyphData &glyphData);

    class GLFont {

    private:
        FT_Library library;
        FT_Face face;

        float h;			///< Holds the height of the font.
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

        inline GlyphData & getGlyphData(unsigned short ch) {
            //if (ch < 256)
            //    return asciiGlyphes[ch];
            GlyphMap::iterator iter = nonAsciiGlyphes.find(ch);
            if (iter != nonAsciiGlyphes.end())
                return *(iter->second);
            GlyphData *newGlyph = new GlyphData();
            make_dlist(face, ch, h, *newGlyph);
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
        GLFont(const char *fname, unsigned int h, float letter_spacing) { init(fname, h, letter_spacing); }
        GLFont() {}
        ~GLFont() {}

        float letter_spacing;

        //The init function will create a font of
        //of the height h from the file fname.
        void init(const char * fname, unsigned int h, float letter_spacing);

        //Free all the resources assosiated with the font.
        void clean();

        //The flagship function of the library - this thing will print
        //out text at window coordinates x,y, using the font ft_font.
        //The current modelview matrix will also be applied to the text.
        void printUnicode(float x, float y, const unsigned short *text) ;
        void printCenteredUnicode(float x, float y, const unsigned short *text);
        float getWidthUnicode(const unsigned short *text);
        //void print(float x, float y, const wchar_t *fmt, ...) ;
        //void printCentered(float x, float y, const wchar_t *fmt, ...);
        //float getWidth(const wchar_t *fmt, ...);
    };

    extern int GLFONT_SCREEN_HEIGHT; //Hack...
}

#endif
