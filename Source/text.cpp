#include "text.h"
#include "shader.h"
#include "texture.h"
#include "fstream"
#include "sstream"

Font font;
Shader fontShader;
Texture fontTexture;
GLuint textVAO;
GLuint textVBO;
vector<vec2> quads;

void drawLine(const string &text, vec2 position, float scale)
{
	float curX = position.x;

	for (unsigned i = 0, l = text.size(); i < l; i++)
	{
		const CharInfo &ci = font.chars[text[i]];

		if (ci.w == 0) continue;

		vec2 posNW = vec2(curX + ci.offsetX * scale,
						  position.y + ci.offsetY * scale);
		vec2 posSE = vec2(curX + (ci.offsetX + ci.w) * scale,
						  position.y + (ci.offsetY + ci.h) * scale);
		vec2 texNW = vec2(ci.x, ci.y);
		vec2 texSE = vec2(ci.x + ci.w, ci.y + ci.h);

		quads.push_back(posNW);
		quads.push_back(texNW);

		quads.push_back(vec2(posSE.x, posNW.y));
		quads.push_back(vec2(texSE.x, texNW.y));

		quads.push_back(posSE);
		quads.push_back(texSE);

		quads.push_back(vec2(posNW.x, posSE.y));
		quads.push_back(vec2(texNW.x, texSE.y));

		curX += ci.advance * scale;
	}
}

void drawText(const string &text, vec2 position, float scale, Anchor anchor)
{
	float curY = 0;
	std::stringstream textStream = std::stringstream(text);
	string line;

	if (anchor == ANCHOR_BOTTOM)
	{
		int lineCount = std::count(text.begin(), text.end(), '\n') + 1;
		curY = - lineCount * font.lineHeight * scale;
	}

	while (std::getline(textStream, line))
	{
		drawLine(line, vec2(position.x, position.y + curY), scale);
		curY += font.lineHeight * scale * 1;
	}
}

void beginRenderText()
{
	quads.clear();
}

void endRenderText()
{
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, quads.size() * sizeof(vec2), &quads[0], GL_STREAM_DRAW);

	glUseProgram(fontShader.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fontTexture.tex);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_QUADS, 0, quads.size());

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	glBindVertexArray(0);
}

void initFont(int screenWidth, int screenHeight)
{
	fontShader = getShader("Shaders/font");
	fontTexture = getTexture("font_0.png");

	glUseProgram(fontShader.program);
	fontShader.setUniform("texture", 0);
	fontShader.setUniform("screenSize", vec2(screenWidth, screenHeight));
	fontShader.setUniform("texSize", vec2(fontTexture.width, fontTexture.height));
	glUseProgram(0);

	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);

	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(vec2), (GLvoid*)0);

	// Vertex Texture Coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(vec2), (GLvoid*)sizeof(vec2));

	glBindVertexArray(0);

	font.face = "Arial";
	font.lineHeight = 26;

	font.chars[32] = {53, 221, 5, 3, -2, 24, 6};
	font.chars[33] = {62, 174, 6, 19, 1, 3, 8};
	font.chars[34] = {141, 207, 10, 8, -1, 3, 8};
	font.chars[35] = {71, 113, 16, 19, -2, 3, 13};
	font.chars[36] = {140, 49, 15, 22, -1, 2, 13};
	font.chars[37] = {226, 69, 20, 19, 0, 3, 20};
	font.chars[38] = {159, 92, 17, 19, -1, 3, 15};
	font.chars[39] = {162, 207, 6, 8, -1, 3, 4};
	font.chars[40] = {108, 0, 9, 24, 0, 3, 8};
	font.chars[41] = {118, 0, 9, 24, 0, 3, 8};
	font.chars[42] = {129, 207, 11, 9, -1, 3, 9};
	font.chars[43] = {165, 190, 15, 14, -1, 6, 13};
	font.chars[44] = {169, 205, 6, 7, 0, 18, 6};
	font.chars[45] = {11, 223, 10, 4, -1, 13, 8};
	font.chars[46] = {40, 221, 6, 4, 0, 18, 6};
	font.chars[47] = {22, 174, 10, 19, -2, 3, 6};
	font.chars[48] = {80, 153, 14, 19, -1, 3, 13};
	font.chars[49] = {44, 174, 9, 19, 1, 3, 13};
	font.chars[50] = {207, 110, 15, 19, -1, 3, 13};
	font.chars[51] = {223, 109, 15, 19, -1, 3, 13};
	font.chars[52] = {239, 109, 15, 19, -1, 3, 13};
	font.chars[53] = {0, 134, 15, 19, -1, 3, 13};
	font.chars[54] = {16, 134, 15, 19, -1, 3, 13};
	font.chars[55] = {96, 132, 15, 19, -1, 3, 13};
	font.chars[56] = {32, 134, 15, 19, -1, 3, 13};
	font.chars[57] = {48, 134, 15, 19, -1, 3, 13};
	font.chars[58] = {158, 191, 6, 15, 0, 7, 6};
	font.chars[59] = {154, 172, 6, 18, 0, 7, 6};
	font.chars[60] = {197, 188, 14, 13, -1, 6, 13};
	font.chars[61] = {114, 208, 14, 9, -1, 8, 13};
	font.chars[62] = {212, 188, 14, 13, 0, 6, 13};
	font.chars[63] = {64, 134, 15, 19, -1, 3, 13};
	font.chars[64] = {0, 0, 25, 24, -1, 3, 23};
	font.chars[65] = {21, 94, 19, 19, -2, 3, 15};
	font.chars[66] = {122, 112, 16, 19, 0, 3, 15};
	font.chars[67] = {101, 92, 19, 19, -1, 3, 17};
	font.chars[68] = {140, 92, 18, 19, 0, 3, 17};
	font.chars[69] = {105, 112, 16, 19, 0, 3, 15};
	font.chars[70] = {80, 133, 15, 19, 0, 3, 14};
	font.chars[71] = {61, 93, 19, 19, -1, 3, 18};
	font.chars[72] = {231, 89, 17, 19, 0, 3, 17};
	font.chars[73] = {249, 89, 6, 19, 0, 3, 6};
	font.chars[74] = {124, 152, 13, 19, -1, 3, 12};
	font.chars[75] = {18, 114, 17, 19, 0, 3, 15};
	font.chars[76] = {95, 153, 14, 19, 0, 3, 13};
	font.chars[77] = {81, 92, 19, 19, 0, 3, 19};
	font.chars[78] = {36, 114, 17, 19, 0, 3, 17};
	font.chars[79] = {0, 94, 20, 19, -1, 3, 18};
	font.chars[80] = {88, 112, 16, 19, 0, 3, 15};
	font.chars[81] = {167, 49, 20, 20, -1, 3, 18};
	font.chars[82] = {121, 92, 18, 19, 0, 3, 17};
	font.chars[83] = {177, 92, 17, 19, -1, 3, 15};
	font.chars[84] = {54, 114, 16, 19, -1, 3, 14};
	font.chars[85] = {195, 90, 17, 19, 0, 3, 17};
	font.chars[86] = {213, 89, 17, 19, -1, 3, 15};
	font.chars[87] = {89, 72, 25, 19, -1, 3, 23};
	font.chars[88] = {0, 114, 17, 19, -1, 3, 15};
	font.chars[89] = {139, 112, 16, 19, -1, 3, 14};
	font.chars[90] = {156, 112, 16, 19, -1, 3, 14};
	font.chars[91] = {137, 0, 7, 24, 0, 3, 6};
	font.chars[92] = {11, 174, 10, 19, -2, 3, 6};
	font.chars[93] = {145, 0, 7, 24, -1, 3, 6};
	font.chars[94] = {14, 210, 13, 11, -1, 3, 12};
	font.chars[95] = {217, 202, 17, 4, -2, 23, 13};
	font.chars[96] = {209, 202, 7, 5, 0, 3, 8};
	font.chars[97] = {32, 194, 15, 15, -1, 7, 13};
	font.chars[98] = {152, 152, 13, 19, 0, 3, 12};
	font.chars[99] = {93, 192, 13, 15, -1, 7, 12};
	font.chars[100] = {138, 152, 13, 19, -1, 3, 12};
	font.chars[101] = {16, 194, 15, 15, -1, 7, 13};
	font.chars[102] = {244, 149, 10, 19, -1, 3, 7};
	font.chars[103] = {0, 73, 13, 20, -1, 7, 12};
	font.chars[104] = {179, 152, 12, 19, 0, 3, 12};
	font.chars[105] = {76, 174, 5, 19, 0, 3, 5};
	font.chars[106] = {128, 0, 8, 24, -3, 3, 4};
	font.chars[107] = {166, 152, 12, 19, 0, 3, 11};
	font.chars[108] = {69, 174, 6, 19, -1, 3, 4};
	font.chars[109] = {215, 172, 19, 15, 0, 7, 18};
	font.chars[110] = {134, 191, 12, 15, 0, 7, 12};
	font.chars[111] = {48, 194, 15, 15, -1, 7, 13};
	font.chars[112] = {14, 73, 13, 20, 0, 7, 12};
	font.chars[113] = {28, 73, 13, 20, -1, 7, 12};
	font.chars[114] = {147, 191, 10, 15, 0, 7, 8};
	font.chars[115] = {64, 194, 14, 15, -1, 7, 12};
	font.chars[116] = {0, 174, 10, 19, -2, 3, 6};
	font.chars[117] = {121, 191, 12, 15, 0, 7, 12};
	font.chars[118] = {0, 194, 15, 15, -2, 7, 11};
	font.chars[119] = {235, 169, 18, 15, -2, 7, 15};
	font.chars[120] = {107, 192, 13, 15, -1, 7, 11};
	font.chars[121] = {240, 48, 13, 20, -1, 7, 11};
	font.chars[122] = {79, 194, 13, 15, -1, 7, 11};
	font.chars[123] = {97, 0, 10, 24, -1, 3, 8};
	font.chars[124] = {153, 0, 6, 24, 0, 3, 6};
	font.chars[125] = {86, 0, 10, 24, -1, 3, 8};
	font.chars[126] = {176, 205, 15, 6, -1, 10, 13};
	font.chars[160] = {47, 221, 5, 3, -2, 24, 6};
	font.chars[161] = {55, 72, 6, 20, 1, 7, 8};
	font.chars[162] = {57, 0, 14, 24, -1, 3, 13};
	font.chars[163] = {112, 132, 15, 19, -1, 3, 13};
	font.chars[164] = {181, 189, 15, 13, -1, 6, 13};
	font.chars[165] = {190, 112, 16, 19, -2, 3, 13};
	font.chars[166] = {160, 0, 6, 24, 0, 3, 6};
	font.chars[167] = {26, 0, 15, 24, -1, 3, 13};
	font.chars[168] = {0, 223, 10, 4, -1, 3, 8};
	font.chars[169] = {204, 69, 21, 19, -2, 3, 17};
	font.chars[170] = {44, 210, 11, 10, -1, 3, 9};
	font.chars[171] = {241, 185, 13, 12, 0, 9, 13};
	font.chars[172] = {98, 208, 15, 9, -1, 8, 13};
	font.chars[173] = {22, 222, 10, 4, -1, 13, 8};
	font.chars[174] = {182, 70, 21, 19, -2, 3, 17};
	font.chars[175] = {235, 201, 17, 4, -2, 0, 13};
	font.chars[176] = {152, 207, 9, 8, 0, 3, 9};
	font.chars[177] = {176, 172, 15, 16, -1, 6, 13};
	font.chars[178] = {79, 210, 10, 10, -1, 3, 8};
	font.chars[179] = {68, 210, 10, 10, -1, 3, 8};
	font.chars[180] = {201, 202, 7, 5, 1, 3, 8};
	font.chars[181] = {42, 73, 12, 20, 0, 7, 12};
	font.chars[182] = {0, 49, 16, 23, -2, 3, 12};
	font.chars[183] = {33, 221, 6, 4, 1, 11, 8};
	font.chars[184] = {192, 203, 8, 6, 0, 20, 8};
	font.chars[185] = {90, 210, 7, 10, 0, 3, 8};
	font.chars[186] = {56, 210, 11, 10, -1, 3, 9};
	font.chars[187] = {227, 188, 13, 12, 0, 9, 13};
	font.chars[188] = {115, 72, 22, 19, -1, 3, 19};
	font.chars[189] = {160, 72, 21, 19, -1, 3, 19};
	font.chars[190] = {138, 72, 21, 19, -1, 3, 19};
	font.chars[191] = {225, 48, 14, 20, 0, 7, 14};
	font.chars[192] = {0, 25, 19, 23, -2, -1, 15};
	font.chars[193] = {40, 25, 19, 23, -2, -1, 15};
	font.chars[194] = {80, 25, 19, 23, -2, -1, 15};
	font.chars[195] = {60, 25, 19, 23, -2, -1, 15};
	font.chars[196] = {85, 49, 19, 22, -2, 0, 15};
	font.chars[197] = {65, 49, 19, 22, -2, 0, 15};
	font.chars[198] = {62, 72, 26, 19, -2, 3, 23};
	font.chars[199] = {20, 25, 19, 23, -1, 3, 17};
	font.chars[200] = {190, 24, 16, 23, 0, -1, 15};
	font.chars[201] = {224, 24, 16, 23, 0, -1, 15};
	font.chars[202] = {207, 24, 16, 23, 0, -1, 15};
	font.chars[203] = {123, 49, 16, 22, 0, 0, 15};
	font.chars[204] = {28, 49, 7, 23, -1, -1, 6};
	font.chars[205] = {36, 49, 7, 23, 0, -1, 6};
	font.chars[206] = {17, 49, 10, 23, -2, -1, 6};
	font.chars[207] = {156, 49, 10, 22, -2, 0, 6};
	font.chars[208] = {41, 94, 19, 19, -2, 3, 17};
	font.chars[209] = {100, 25, 17, 23, 0, -1, 17};
	font.chars[210] = {167, 0, 20, 23, -1, -1, 18};
	font.chars[211] = {209, 0, 20, 23, -1, -1, 18};
	font.chars[212] = {188, 0, 20, 23, -1, -1, 18};
	font.chars[213] = {230, 0, 20, 23, -1, -1, 18};
	font.chars[214] = {44, 49, 20, 22, -1, 0, 18};
	font.chars[215] = {0, 210, 13, 12, 0, 7, 13};
	font.chars[216] = {188, 48, 20, 20, -1, 3, 18};
	font.chars[217] = {154, 25, 17, 23, 0, -1, 17};
	font.chars[218] = {172, 24, 17, 23, 0, -1, 17};
	font.chars[219] = {136, 25, 17, 23, 0, -1, 17};
	font.chars[220] = {105, 49, 17, 22, 0, 0, 17};
	font.chars[221] = {118, 25, 17, 23, -2, -1, 15};
	font.chars[222] = {173, 112, 16, 19, 0, 3, 15};
	font.chars[223] = {128, 132, 15, 19, 0, 3, 14};
	font.chars[224] = {144, 132, 15, 19, -1, 3, 13};
	font.chars[225] = {160, 132, 15, 19, -1, 3, 13};
	font.chars[226] = {176, 132, 15, 19, -1, 3, 13};
	font.chars[227] = {192, 132, 15, 19, -1, 3, 13};
	font.chars[228] = {82, 173, 15, 18, -1, 4, 13};
	font.chars[229] = {209, 48, 15, 20, -1, 2, 13};
	font.chars[230] = {192, 172, 22, 15, -1, 7, 20};
	font.chars[231] = {110, 152, 13, 19, -1, 7, 12};
	font.chars[232] = {64, 154, 15, 19, -1, 3, 13};
	font.chars[233] = {208, 130, 15, 19, -1, 3, 13};
	font.chars[234] = {224, 129, 15, 19, -1, 3, 13};
	font.chars[235] = {98, 173, 15, 18, -1, 4, 13};
	font.chars[236] = {247, 69, 7, 19, -1, 3, 6};
	font.chars[237] = {54, 174, 7, 19, 0, 3, 6};
	font.chars[238] = {33, 174, 10, 19, -2, 3, 6};
	font.chars[239] = {143, 172, 10, 18, -2, 4, 6};
	font.chars[240] = {240, 129, 15, 19, -1, 3, 13};
	font.chars[241] = {231, 149, 12, 19, 0, 3, 13};
	font.chars[242] = {0, 154, 15, 19, -1, 3, 13};
	font.chars[243] = {16, 154, 15, 19, -1, 3, 13};
	font.chars[244] = {32, 154, 15, 19, -1, 3, 13};
	font.chars[245] = {48, 154, 15, 19, -1, 3, 13};
	font.chars[246] = {114, 172, 15, 18, -1, 4, 13};
	font.chars[247] = {28, 210, 15, 10, -1, 8, 13};
	font.chars[248] = {161, 172, 14, 17, 0, 6, 13};
	font.chars[249] = {218, 150, 12, 19, 0, 3, 13};
	font.chars[250] = {205, 152, 12, 19, 0, 3, 13};
	font.chars[251] = {192, 152, 12, 19, 0, 3, 13};
	font.chars[252] = {130, 172, 12, 18, 0, 4, 13};
	font.chars[253] = {72, 0, 13, 24, -1, 3, 12};
	font.chars[254] = {42, 0, 14, 24, 0, 3, 13};
	font.chars[255] = {241, 24, 13, 23, -1, 4, 12};
}
