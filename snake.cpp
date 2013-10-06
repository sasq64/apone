#include <freetype-gl.h>
//#include <mat4.h>
//#include <shader.h>
//#include <vertex-buffer.h>

#include <grappix.h>
#include <vector>
#include <cstdlib>

using namespace std;
using namespace utils;

uint32_t blend(uint32_t col0, uint32_t col1, float alpha) {
	auto a0 = (col0>>24)&0xff;
	auto r0 = (col0>>16)&0xff;
	auto g0 = (col0>>8)&0xff;
	auto b0 = col0&0xff;

	auto a1 = (col1>>24)&0xff;
	auto r1 = (col1>>16)&0xff;
	auto g1 = (col1>>8)&0xff;
	auto b1 = col1&0xff;

	a0 = a0*alpha+a1*(1.0-alpha);
	r0 = r0*alpha+r1*(1.0-alpha);
	g0 = g0*alpha+g1*(1.0-alpha);
	b0 = b0*alpha+b1*(1.0-alpha);
	return (a0<<24) | (r0<<16) | (g0<<8) | b0;
}


static texture_font_t *font;
static texture_atlas_t *atlas;

void make_text() {
	atlas = texture_atlas_new( 512, 512, 1 );
	const wchar_t *text = L"@ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ";
	font = texture_font_new(atlas, "fonts/Vera.ttf", 50);
	texture_font_load_glyphs(font, text);
	//texture_font_delete(font);
}

static float scale = 1.0;

void render_text(int x, int y, const string &text) {

	auto tl = text.length();
	vector<GLfloat> p;
	vector<GLfloat> uvs;
	vector<GLushort> indexes;

	auto _width = screen.size().first;
	auto _height = screen.size().second;
	char lastChar = 0;
	int i = 0;
	for(auto c : text) {
		texture_glyph_t *glyph = texture_font_get_glyph(font, c);
		//if( glyph == NULL )
		if(lastChar)
			x += texture_glyph_get_kerning(glyph, lastChar);
		lastChar = c;

		float x0  = x + glyph->offset_x;
		float y0  = y + glyph->offset_y;
		float x1  = x0 + glyph->width;
		float y1  = y0 - glyph->height;

		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;

		p.push_back(x0);
		p.push_back(y1);
		p.push_back(x1);
		p.push_back(y1);
		p.push_back(x0);
		p.push_back(y0);
		p.push_back(x1);
		p.push_back(y0);

		uvs.push_back(s0); uvs.push_back(t1);
		uvs.push_back(s1); uvs.push_back(t1);
		uvs.push_back(s0); uvs.push_back(t0);
		uvs.push_back(s1); uvs.push_back(t0);

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

	//LOGD("[%f]", uvs);

	auto program = get_program(FONT_PROGRAM);
	glUseProgram(program);

	GLuint posHandle = glGetAttribLocation(program, "vPosition");
	GLuint uvHandle = glGetAttribLocation(program, "vUV");

	uint32_t color = 0x40ff80;

	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform2f(whHandle, 2.0 / _width, 2.0 / _height);

	GLuint scaleHandle = glGetUniformLocation(program, "fScale");
	GLuint colorHandle = glGetUniformLocation(program, "fColor");
	float red = ((color>>16)&0xff) / 255.0;
	float green = ((color>>8)&0xff) / 255.0;
	float blue = (color&0xff) / 255.0;
	glUniform4f(colorHandle, red, green, blue, 1.0);
	glUniform1f(scaleHandle, scale);
	scale *= 1.001;

	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, &p[0]);
	glEnableVertexAttribArray(posHandle);
	glVertexAttribPointer(uvHandle, 2, GL_FLOAT, GL_FALSE, 0, &uvs[0]);
	glEnableVertexAttribArray(uvHandle);
	glBindTexture( GL_TEXTURE_2D, atlas->id );

	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawElements(GL_TRIANGLES, 6*tl, GL_UNSIGNED_SHORT, &indexes[0]);
}

int main() {

	vec2f screenSize = vec2f(screen.size());
	vec2f playFieldSize {32, 18};
	vec2f border { 100, 100 };
	vec2f tileSize = (screenSize - border) / playFieldSize;
	tileSize.x = (int)tileSize.x;
	tileSize.y = (int)tileSize.y;

	srand(0);

	renderbuffer sprite(tileSize);
	sprite.clear();
	sprite.circle(tileSize/2, tileSize[0]/2, 0x00c000); 

	int w = playFieldSize.x;
	int h = playFieldSize.y;

	vector<int> playField(w*h);

	playField[rand() % (w*h)] = -1;

	vec2f pos = {5,5};
	int score = 0;

	int d = 2;
	vector<vec2f> adds { {0,-1}, {1,0}, {0,1}, {-1,0} };
	bool noKey = true;
	int speed = 10;
	int delay = speed;

	int snakeLen = 100;

	make_text();
	bool gameOver = false;
	float endScale = 1.0;
	while(screen.is_open()) {

		if(!gameOver) {
			if(screen.key_pressed(window::LEFT)) {
				if(noKey)
					d--;
				noKey = false;
			} else if(screen.key_pressed(window::RIGHT)) {
				if(noKey)
					d++;
				noKey = false;
			} else
				noKey = true;

			d = (d+4) % 4;

			if(delay-- == 0) {
				pos += adds[d];
				if(pos.x < 0 || pos.y < 0 || pos.x >= w || pos.y >= h || playField[pos.x + pos.y * w] > 0) {
					gameOver = true;
				}

				snakeLen += 4;

				if(playField[pos.x + pos.y * w] == -1) {
					snakeLen += 24;
					score++;
					while(true) {
						auto applePos = rand() % (w*h);
						if(playField[applePos] == 0) {
							playField[applePos] = -1;
							break;
						}
					}
				}

				playField[pos.x + pos.y * w] = 1;
				delay = speed;
			}
		}
		screen.clear();

		vec2f pfpos { 50, 50 };
		screen.scale(1.0);
		screen.rectangle(pfpos - 10, tileSize.x * w + 20, tileSize.y * h + 20, 0x00a000);
		screen.rectangle(pfpos, tileSize.x * w, tileSize.y * h, 0x000000);
		int i = 0;
		for(auto &p : playField) {
			if(p) {
				vec2f pos { (float)(i%w), (float)(i/w) };
				if(p == -1) {
					//screen.draw(pos * tileSize + pfpos, sprite);
					float r = tileSize.x/3;
					screen.circle(pos * tileSize + pfpos + r, r, 0x00ff00);
				} else {
					if(endScale < 10.0) {
						if(gameOver) {
							screen.scale(endScale);
							screen.rectangle(pos * tileSize + pfpos, tileSize, blend(0x00ff00, 0x000000, 1.0/endScale));
						} else  {
							screen.scale(1.0 - (float)p/snakeLen);
							screen.rectangle(pos * tileSize + pfpos, tileSize, blend(0x004000, 0x00ff00, (float)p/snakeLen));
							p++;
							if(p == snakeLen)
								p = 0;
						}
					}
				}
			}
			i++;
		}
		if(gameOver)
			endScale *= 1.05;

		render_text(10, 5, format("Score:%d", score));
		screen.flip();
	}

	return 0;
}