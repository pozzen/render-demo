#ifndef _MAIN_H_INCLUDED_
#define _MAIN_H_INCLUDED_

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstdio>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

using std::string;
using std::vector;
using std::unordered_map;
using std::cout;
using std::cerr;
using std::endl;
using std::shared_ptr;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;
using glm::mat3;
using glm::mat4;
using glm::abs;
using glm::clamp;
using glm::distance;
using glm::distance2;
using glm::radians;
using glm::normalize;
using glm::perspective;
using glm::cross;

#endif // _MAIN_H_INCLUDED_
