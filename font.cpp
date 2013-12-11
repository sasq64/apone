#include "GL_Header.h"

#include "font.h"
#include "shader.h"
#include "color.h"
#include "staticfont.h"
#include "render_target.h"

#include <coreutils/log.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>
using namespace std;

uint8_t *make_distance_map(uint8_t *img, int width, int height);

Font::Font() : font(nullptr), atlas(nullptr) {
	atlas = new texture_atlas_t();
	atlas->width = static_font.tex_width;
	atlas->height = static_font.tex_height;
	atlas->id = 0;
	atlas->data = static_font.tex_data;
    texture_atlas_upload(atlas);
    LOGD("Static font created");
}

Font::Font(const string &ttfName, int size, int flags) {

	atlas = texture_atlas_new(256, 256, 1 );

	const wchar_t *text = L"@!ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ";
	font = texture_font_new(atlas, ttfName.c_str(), size);

	LOGD("Result %p", font);

	texture_font_load_glyphs(font, text);

	if(flags & DISTANCE_MAP) {
		uint8_t *data = make_distance_map(atlas->data, atlas->width, atlas->height);
		LOGD("Distance map created");
		free(atlas->data);
		atlas->data = data;
	}
    texture_atlas_upload(atlas);
}

//static float scale = 1.0;

TextBuf Font::make_text2(const string &text) {

	LOGD("Make text2");

	//auto tl = text.length();
	//vector<GLfloat> p;
	vector<GLfloat> verts;
	vector<GLushort> indexes;

	//char lastChar = 0;
	int i = 0;
	float x = 0;
	float y = 0;
	for(auto c : text) {

        texture_glyph2_t *glyph = 0;
        for(int j=0; j<static_font.glyphs_count; ++j) {
            if(static_font.glyphs[j].charcode == c) {
                glyph = &static_font.glyphs[j];
                break;
            }
        }
        if(!glyph)
            continue;

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
	glGenBuffers(2, &tbuf.vbuf[0]);
	glBindBuffer(GL_ARRAY_BUFFER, tbuf.vbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbuf.vbuf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * 2, &indexes[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, &verts[0], GL_STATIC_DRAW);

	return tbuf;
}

TextBuf Font::make_text(const string &text) {

	if(!font)
		return make_text2(text);

	LOGD("Make text");

	//auto tl = text.length();
	//vector<GLfloat> p;
	vector<GLfloat> verts;
	vector<GLushort> indexes;

	char lastChar = 0;
	int i = 0;
	float x = 0;
	float y = 0;
	for(auto c : text) {
		texture_glyph_t *glyph = texture_font_get_glyph(font, c);
		//LOGD("Glyph %p", glyph);
		//if( glyph == NULL )
		if(lastChar)
			x += texture_glyph_get_kerning(glyph, lastChar);
		lastChar = c;

		float x0  = x + glyph->offset_x;
		float y0  = y + font->height;//gl;//+ glyph->offset_y;
		float x1  = x0 + glyph->width;
		float y1  = y0 - glyph->offset_y;
		//LOGD("%d %d", glyph->height, glyph->offset_y);

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
	glGenBuffers(2, &tbuf.vbuf[0]);
	glBindBuffer(GL_ARRAY_BUFFER, tbuf.vbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbuf.vbuf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * 2, &indexes[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, &verts[0], GL_STATIC_DRAW);

	return tbuf;
}

void Font::render_text(RenderTarget &target, const TextBuf &text, int x, int y, uint32_t color, float scale) {

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

	glBindTexture( GL_TEXTURE_2D, atlas->id );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	int tl = text.text.length();
	glDrawElements(GL_TRIANGLES, 6*tl, GL_UNSIGNED_SHORT, 0);

	//LOGD("Drew %d\n", tl);

	glDisableVertexAttribArray(uvHandle);
	glDisableVertexAttribArray(vertHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Font::render_text(RenderTarget &target, const std::string &text, int x, int y, uint32_t col, float scale) {

	auto buf = cache.get(text);
	if(buf.text == "") {
		buf = make_text(text);
		cache.put(text, buf);
	}
	render_text(target, buf, x, y, col, scale);
}



