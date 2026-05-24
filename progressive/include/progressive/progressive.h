#ifndef PROGRESSIVE_PROGRESSIVE_H
#define PROGRESSIVE_PROGRESSIVE_H

#include <string>

#define PROGRESSIVE_VERSION_MAJOR 1
#define PROGRESSIVE_VERSION_MINOR 0
#define PROGRESSIVE_VERSION_PATCH 0

namespace progressive {

struct RendererConfig;
struct Frame;
class Renderer;
class Window;

std::string version();

} // namespace progressive

#endif // PROGRESSIVE_PROGRESSIVE_H
