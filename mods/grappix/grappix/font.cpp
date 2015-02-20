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
#include <coreutils/file.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>
using namespace std;
using namespace utils;

namespace grappix {

std::vector<std::weak_ptr<Font::FontRef>> Font::fontRefs;

uint8_t *make_distance_map(uint8_t *img, int width, int height);

Font::Font(bool stfont) : size(32) {

	program = get_program(FONT_PROGRAM_DF);

	ref = make_shared<FontRef>(0, 0, "", 0.0, 0);
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

Font::Font(const string &ttfName, int size, int flags) : size(size) {

	program = flags & DISTANCE_MAP ? get_program(FONT_PROGRAM_DF) : get_program(FONT_PROGRAM);

	int tsize = flags & 0xffffc0;
	if(tsize == 0) tsize = 128;
	flags &= 0x3f;

	for(auto &fr : fontRefs) {
		auto r = fr.lock();
		if(r) {
			LOGD("%s(%d) vs %s(%d)",r->ttfName, r->w, ttfName, tsize);
			if(r->ttfName == ttfName && r->w == tsize && r->h == tsize && r->flags == flags) {
				ref = r;
				LOGD("Reusing %s (%d)", ttfName, size);
				return;
			}
		}
	}

	ref = make_shared<FontRef>(tsize, tsize, ttfName, size, flags);
	fontRefs.push_back(ref);

	auto text = fontLetters;
	if(flags & UPPER_CASE)
		text = fontLettersUpper;

	texture_font_load_glyphs((texture_font_t*)ref->font, text);

	texture_atlas_t *atlas = (texture_atlas_t*)ref->atlas;
	if(flags & DISTANCE_MAP) {
		auto fn = path_filename(ttfName);
		File f { format("%s%s.%d.%d.dfield", File::getCacheDir(), fn, size, tsize) };
		if(f.exists()) {
			f.read(atlas->data, atlas->width*atlas->height);
			LOGD("Distance map loaded");
		} else {
			uint8_t *data = make_distance_map(atlas->data, atlas->width, atlas->height);
			LOGD("Distance map created");
			free(atlas->data);
			atlas->data = data;
			f.write(atlas->data, atlas->width*atlas->height);
		}
		f.close();
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

	auto t2 = utf8_decode(text);

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

	char lastChar = 0;
	int i = 0;
	float x = 0;
	texture_font_t *font = (texture_font_t*)ref->font;

	float y = font->ascender;

	//auto t2 = utf8_decode(text);

	int tl = text.length();

	vector<GLfloat> verts;
	vector<GLushort> indexes;

	verts.reserve(16*tl);
	indexes.reserve(4*tl);

	for(auto c : text) {

		texture_glyph_t *glyph = texture_font_get_glyph(font, c);
		if(!glyph)
			continue;
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
	tbuf.rec[1] = 0;//verts[1];
	tbuf.rec[2] = verts[verts.size()-4];
	tbuf.rec[3] = font->height;//verts[verts.size()-3];

	//LOGD("Text %s covers %f to %f", text, tbuf.rec[0], tbuf.rec[2]);

	glGenBuffers(2, &tbuf.vbuf[0]);
	glBindBuffer(GL_ARRAY_BUFFER, tbuf.vbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbuf.vbuf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * 2, &indexes[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, &verts[0], GL_STATIC_DRAW);

	return tbuf;
}


void Font::render_text(const RenderTarget &target, const TextBuf &text, float x, float y, uint32_t color, float scale) const {

	scale = scale * 32.0 / (float)size;

	glBindFramebuffer(GL_FRAMEBUFFER, target.buffer());
	glViewport(0,0,target.width(), target.height());

	program.use();

	glBindBuffer(GL_ARRAY_BUFFER, text.vbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text.vbuf[1]);

	mat4f matrix = make_scale(scale, scale);
	matrix = make_translate(x, y) * matrix;
	matrix = target.get_view_matrix() * matrix;
	program.setUniform("matrix", matrix.transpose());

	// Needed for DF shader
	program.setUniform("vScale", scale);

	//auto c = make_color(color);
	program.setUniform("color", Color(color));

	program.vertexAttribPointer("vertex", 2, GL_FLOAT, GL_FALSE, 16, 0);
	program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 16, 8);

	texture_atlas_t *atlas = (texture_atlas_t*)ref->atlas;
	glBindTexture( GL_TEXTURE_2D, atlas->id );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//int tl = text.text.length();
	glDrawElements(GL_TRIANGLES, 6*text.size, GL_UNSIGNED_SHORT, 0);

	//LOGD("Drew %d\n", tl);

	//glDisableVertexAttribArray(uvHandle);
	//glDisableVertexAttribArray(vertHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Font::render_text(const RenderTarget &target, const std::string &text, float x, float y, uint32_t col, float scale) const {
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


int Font::get_width(const string &text, float scale) const {
	return get_size(text, scale).x;
	// if(text == "")
	// 	return 0;
	// auto buf = cache.get(text);
	// if(buf.text == "") {
	// 	buf = make_text(text);
	// 	cache.put(text, buf);
	// }
	// scale = scale * 32.0 / (float)size;
	// return (buf.rec[2] - buf.rec[0]) * scale;
}

vec2i Font::get_size(const string &text, float scale) const {
	if(text == "")
		return vec2i(0,0);
	auto buf = cache.get(text);
	if(buf.text == "") {
		buf = make_text(text);
		cache.put(text, buf);
	}
	scale = scale * 32.0 / (float)size;
	return vec2i(buf.rec[2] - buf.rec[0], buf.rec[3] - buf.rec[1]) * scale;
}


Font::FontRef::FontRef(int w, int h, const std::string &ttfName, int fsize, int flags) : w(w), h(h), flags(flags), ttfName(ttfName), atlas(nullptr), font(nullptr) {
	//LOGD("FONTREF CONSTRUCT");
	texture_atlas_t *a = nullptr;
	if(w > 0 && h > 0)
		a = texture_atlas_new(w, h, 1);
	if(a && fsize > 0) {
		font = (texture_font_t*)texture_font_new(a, ttfName.c_str(), fsize);
		//LOGD("FONTREF DONE");
	}
	atlas = a;
}
Font::FontRef::~FontRef() {
	//LOGD("FONTREF DESTROY");
	if(font)
		texture_font_delete((texture_font_t*)font);
	if(atlas)
		texture_atlas_delete((texture_atlas_t*)atlas);
	font = nullptr;
	atlas = nullptr;
}


void TextBuf::destroy() {
	glDeleteBuffers(2, &vbuf[0]);
}

}

