#include "GL_Header.h"

#include "texture-font.h"
#include "texture-atlas.h"

#include "font.h"
#include "shader.h"
#include "color.h"
#include "staticfont.h"
#include "render_target.h"

#include <coreutils/utils.h>
#include <coreutils/log.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>
using namespace std;

namespace grappix {

uint8_t *make_distance_map(uint8_t *img, int width, int height);

Font::Font(bool stfont) {
	ref = make_shared<FontRef>(0, 0, "", 0.0);
	texture_atlas_t *atlas = new texture_atlas_t();
	ref->atlas = atlas;
	atlas->width = static_font.tex_width;
	atlas->height = static_font.tex_height;
	atlas->id = 0;
	atlas->data = static_font.tex_data;
    texture_atlas_upload(atlas);
    LOGD("Static font created");
}



const static wchar_t *fontLetters = L"@!ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ";
const static wchar_t *fontLettersUpper = L"@!ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";

Font::Font(const string &ttfName, int size, int flags) {

	int tsize = flags & 0xffffc0;
	if(tsize == 0) tsize = 128;
	flags &= 0x3f;

	ref = make_shared<FontRef>(tsize, tsize, ttfName, size);

	auto text = fontLetters;
	if(flags & UPPER_CASE)
		text = fontLettersUpper;

	texture_font_load_glyphs((texture_font_t*)ref->font, text);

	texture_atlas_t *atlas = (texture_atlas_t*)ref->atlas;
	if(flags & DISTANCE_MAP) {
		uint8_t *data = make_distance_map(atlas->data, atlas->width, atlas->height);
		LOGD("Distance map created");
		free(atlas->data);
		atlas->data = data;
	}
    texture_atlas_upload(atlas);
}

//static float scale = 1.0;

TextBuf Font::make_text2(const string &text) const {

	//LOGD("Make text2");

	//auto tl = text.length();
	//vector<GLfloat> p;
	vector<GLfloat> verts;
	vector<GLushort> indexes;

	//char lastChar = 0;
	int i = 0;
	float x = 0;
	float y = 0;

	auto t2 = utils::utf8_decode(text);

	for(auto c : t2) {

        texture_glyph2_t *glyph = 0;
        for(unsigned int j=0; j<static_font.glyphs_count; ++j) {
            if(static_font.glyphs[j].charcode == c) {
                glyph = &static_font.glyphs[j];
                break;
            }
        }
        if(!glyph) { 
   			x += 8.0;
            continue;
        }

		//texture_glyph_t *glyph = texture_font_get_glyph(font, c);
		//LOGD("Glyph %p", glyph);
		//if( glyph == NULL )
		//if(lastChar)
		//	x += texture_glyph_get_kerning(glyph, lastChar);
		float x0  = x + glyph->offset_x;
		float y0  = y + static_font.height;//gl;//+ glyph->offset_y;
		float x1  = x0 + glyph->width;
		float y1  = y0 - glyph->offset_y;
		//LOGD("%d %d", glyph->height, glyph->offset_y);

		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;

		x += glyph->advance_x;

		verts.push_back(x0);
		verts.push_back(y1);
		verts.push_back(s0);
		verts.push_back(t0);
		verts.push_back(x1);
		verts.push_back(y1);
		verts.push_back(s1);
		verts.push_back(t0);
		verts.push_back(x0);
		verts.push_back(y0);
		verts.push_back(s0);
		verts.push_back(t1);
		verts.push_back(x1);
		verts.push_back(y0);
		verts.push_back(s1);
		verts.push_back(t1);

		indexes.push_back(i);
		indexes.push_back(i+1);
		indexes.push_back(i+2);
		indexes.push_back(i+1);
		indexes.push_back(i+3);
		indexes.push_back(i+2);
		i += 4;
		//break;
	}

	TextBuf tbuf;
	//vector<GLuint> vbuf(2);
	tbuf.text = text;
	tbuf.size = i/4;
	glGenBuffers(2, &tbuf.vbuf[0]);
	glBindBuffer(GL_ARRAY_BUFFER, tbuf.vbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbuf.vbuf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * 2, &indexes[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, &verts[0], GL_STATIC_DRAW);

	return tbuf;
}

TextBuf Font::make_text(const string &text) const {

	if(!ref->font)
		return make_text2(text);

	//LOGD("Make text");

	//auto tl = text.length();
	//vector<GLfloat> p;
	vector<GLfloat> verts;
	vector<GLushort> indexes;

	char lastChar = 0;
	int i = 0;
	float x = 0;
	texture_font_t *font = (texture_font_t*)ref->font;

	float y = font->ascender;

	//auto t2 = utils::utf8_decode(text);

	for(auto c : text) {

		texture_glyph_t *glyph = texture_font_get_glyph(font, c);
		//LOGD("Glyph %p", glyph);
		//if( glyph == NULL )
		if(lastChar)
			x += texture_glyph_get_kerning(glyph, lastChar);
		lastChar = c;

		//LOGD("%d vs %d", glyph->height, font->height);

		float x0  = x + glyph->offset_x;
		float x1  = x0 + glyph->width;

		float y1  = y - glyph->offset_y;
		float y0  = y1 + glyph->height;
		//LOGD("%f %f %f", glyph->height, glyph->offset_y, font->height);

		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;

		verts.push_back(x0);
		verts.push_back(y1);
		verts.push_back(s0);
		verts.push_back(t0);
		verts.push_back(x1);
		verts.push_back(y1);
		verts.push_back(s1);
		verts.push_back(t0);
		verts.push_back(x0);
		verts.push_back(y0);
		verts.push_back(s0);
		verts.push_back(t1);
		verts.push_back(x1);
		verts.push_back(y0);
		verts.push_back(s1);
		verts.push_back(t1);

		indexes.push_back(i);
		indexes.push_back(i+1);
		indexes.push_back(i+2);
		indexes.push_back(i+1);
		indexes.push_back(i+3);
		indexes.push_back(i+2);
		i += 4;

		x += glyph->advance_x;

		//break;
	}



	TextBuf tbuf;
	//vector<GLuint> vbuf(2);
	tbuf.text = text;
	tbuf.size = i/4;
	tbuf.rec[0] = verts[0];
	tbuf.rec[1] = verts[1];
	tbuf.rec[2] = verts[verts.size()-4];
	tbuf.rec[3] = verts[verts.size()-3];

	//LOGD("Text %s covers %f to %f", text, tbuf.rec[0], tbuf.rec[2]);

	glGenBuffers(2, &tbuf.vbuf[0]);
	glBindBuffer(GL_ARRAY_BUFFER, tbuf.vbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbuf.vbuf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * 2, &indexes[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, &verts[0], GL_STATIC_DRAW);

	return tbuf;
}


void Font::render_text(const RenderTarget &target, const TextBuf &text, int x, int y, uint32_t color, float scale) const {

	//LOGD("[%f]", uvs);
	//auto _width = screen.size().first;
	//auto _height = screen.size().second;

	//LOGD("Render text %d %d", vbuf[0], vbuf[1]);

	glBindBuffer(GL_ARRAY_BUFFER, text.vbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text.vbuf[1]);


	auto program = get_program(FONT_PROGRAM_DF).id();
	//LOGD("Program %d", program);
	glUseProgram(program);

	GLuint vertHandle = glGetAttribLocation(program, "vertex");
	GLuint uvHandle = glGetAttribLocation(program, "uv");

	//uint32_t color = 0x40ff80;

	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform4f(whHandle, 2.0 / target.width(), 2.0 / target.height(), 0, 1);

	GLuint posHandle = glGetUniformLocation(program, "vPosition");
	GLuint scaleHandle = glGetUniformLocation(program, "vScale");
	GLuint colorHandle = glGetUniformLocation(program, "vColor");
	float red = ((color>>16)&0xff) / 255.0;
	float green = ((color>>8)&0xff) / 255.0;
	float blue = (color&0xff) / 255.0;
	float alpha = ((color>>24)&0xff) / 255.0;
	glUniform4f(colorHandle, red, green, blue, alpha);
	glUniform4f(scaleHandle, scale, scale, 0, 1);
	glUniform4f(posHandle, x, y, 0, 1);
	//scale *= 1.001;


	glVertexAttribPointer(vertHandle, 2, GL_FLOAT, GL_FALSE, 16, 0);
	glEnableVertexAttribArray(vertHandle);
	glVertexAttribPointer(uvHandle, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);
	glEnableVertexAttribArray(uvHandle);

	texture_atlas_t *atlas = (texture_atlas_t*)ref->atlas;
	glBindTexture( GL_TEXTURE_2D, atlas->id );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//int tl = text.text.length();
	glDrawElements(GL_TRIANGLES, 6*text.size, GL_UNSIGNED_SHORT, 0);

	//LOGD("Drew %d\n", tl);

	glDisableVertexAttribArray(uvHandle);
	glDisableVertexAttribArray(vertHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Font::render_text(const RenderTarget &target, const std::string &text, int x, int y, uint32_t col, float scale) const {
	if(text == "")
		return;
	auto buf = cache.get(text);
	if(buf.text == "") {
		buf = make_text(text);
		cache.put(text, buf);
	}
	render_text(target, buf, x, y, col, scale);
}

void clean_cache() {
}


int Font::get_width(const string &text, float scale) {
	if(text == "")
		return 0;
	auto buf = cache.get(text);
	if(buf.text == "") {
		buf = make_text(text);
		cache.put(text, buf);
	}
	return (buf.rec[2] - buf.rec[0]) * scale;
}


Font::FontRef::FontRef(int w, int h, const std::string &ttfName, int size) : atlas(nullptr), font(nullptr) {
	LOGD("FONTREF CONSTRUCT");
	texture_atlas_t *a = nullptr;
	if(w > 0 && h > 0)
		a = texture_atlas_new(w, h, 1);
	if(a && size > 0) {
		font = (texture_font_t*)texture_font_new(a, ttfName.c_str(), size);
		LOGD("FONTREF DONE");
	}
	atlas = a;
}
Font::FontRef::~FontRef() {
	//LOGD("FONTREF DESTROY");
	//if(font)
	//	texture_font_delete((texture_font_t*)font);
	//if(atlas)
	//	texture_atlas_delete((texture_atlas_t*)atlas);
	font = nullptr;
	atlas = nullptr;
}


}

