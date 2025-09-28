#include "marching_cubes.hpp"
#include "glm/gtc/type_ptr.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

namespace Lab {

namespace {
struct GridCell {
    std::array<glm::vec3, 8> vertices;
    std::array<float, 8>     sdf_values;
};

using Triangle      = std::array<glm::vec3, 3>;
using TriangleIndex = glm::uvec3;
} // namespace

/// edgeTable[i] is a 12 bit number; i is a cubeIndex
/// edgeTable[i][j] = 1 if isosurface intersects edge j for cubeIndex i
static constexpr auto edge_table = std::array<int, 256>{
    0x0, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
    0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
    0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
    0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
    0x230, 0x339, 0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
    0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
    0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
    0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
    0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c,
    0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
    0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff, 0x3f5, 0x2fc,
    0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
    0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
    0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
    0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc,
    0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
    0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
    0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
    0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
    0x15c, 0x55, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
    0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
    0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
    0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
    0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460,
    0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
    0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa, 0x1a3, 0x2a9, 0x3a0,
    0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
    0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
    0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
    0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190,
    0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
    0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
};

/// triangleTable[i] is a list of edges forming triangles for cubeIndex i
/// Indices into vertex buffer (0 - 11)
/// To have it on the heap we can't use std::vector // TODO(Meshing) I don't understand this comment
// clang-format off
static constexpr auto triangle_table = std::array<std::array<std::optional<TriangleIndex>, 5>, 256> {{
    { {std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt}},
    { { TriangleIndex{0, 8, 3}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{0, 1, 9}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{1, 8, 3}, TriangleIndex{9, 8, 1} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 2, 10}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{0, 8, 3}, TriangleIndex{1, 2, 10} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 2, 10}, TriangleIndex{0, 2, 9} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{2, 8, 3}, TriangleIndex{2, 10, 8}, TriangleIndex{10, 9, 8} , std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 11, 2}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{0, 11, 2}, TriangleIndex{8, 11, 0} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 9, 0}, TriangleIndex{2, 3, 11} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 11, 2}, TriangleIndex{1, 9, 11}, TriangleIndex{9, 8, 11} , std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 10, 1}, TriangleIndex{11, 10, 3} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 10, 1}, TriangleIndex{0, 8, 10}, TriangleIndex{8, 11, 10} , std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 9, 0}, TriangleIndex{3, 11, 9}, TriangleIndex{11, 10, 9} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 8, 10}, TriangleIndex{10, 8, 11} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 7, 8}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{4, 3, 0}, TriangleIndex{7, 3, 4} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 1, 9}, TriangleIndex{8, 4, 7} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 1, 9}, TriangleIndex{4, 7, 1}, TriangleIndex{7, 3, 1} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 2, 10}, TriangleIndex{8, 4, 7} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 4, 7}, TriangleIndex{3, 0, 4}, TriangleIndex{1, 2, 10} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 2, 10}, TriangleIndex{9, 0, 2}, TriangleIndex{8, 4, 7} , std::nullopt, std::nullopt} },
    { { TriangleIndex{2, 10, 9}, TriangleIndex{2, 9, 7}, TriangleIndex{2, 7, 3}, TriangleIndex{7, 9, 4} , std::nullopt} },
    { { TriangleIndex{8, 4, 7}, TriangleIndex{3, 11, 2} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{11, 4, 7}, TriangleIndex{11, 2, 4}, TriangleIndex{2, 0, 4} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 0, 1}, TriangleIndex{8, 4, 7}, TriangleIndex{2, 3, 11} , std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 7, 11}, TriangleIndex{9, 4, 11}, TriangleIndex{9, 11, 2}, TriangleIndex{9, 2, 1} , std::nullopt} },
    { { TriangleIndex{3, 10, 1}, TriangleIndex{3, 11, 10}, TriangleIndex{7, 8, 4} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 11, 10}, TriangleIndex{1, 4, 11}, TriangleIndex{1, 0, 4}, TriangleIndex{7, 11, 4} , std::nullopt} },
    { { TriangleIndex{4, 7, 8}, TriangleIndex{9, 0, 11}, TriangleIndex{9, 11, 10}, TriangleIndex{11, 0, 3} , std::nullopt} },
    { { TriangleIndex{4, 7, 11}, TriangleIndex{4, 11, 9}, TriangleIndex{9, 11, 10} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 5, 4}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{9, 5, 4}, TriangleIndex{0, 8, 3} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 5, 4}, TriangleIndex{1, 5, 0} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{8, 5, 4}, TriangleIndex{8, 3, 5}, TriangleIndex{3, 1, 5} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 2, 10}, TriangleIndex{9, 5, 4} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 0, 8}, TriangleIndex{1, 2, 10}, TriangleIndex{4, 9, 5} , std::nullopt, std::nullopt} },
    { { TriangleIndex{5, 2, 10}, TriangleIndex{5, 4, 2}, TriangleIndex{4, 0, 2} , std::nullopt, std::nullopt} },
    { { TriangleIndex{2, 10, 5}, TriangleIndex{3, 2, 5}, TriangleIndex{3, 5, 4}, TriangleIndex{3, 4, 8} , std::nullopt} },
    { { TriangleIndex{9, 5, 4}, TriangleIndex{2, 3, 11} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 11, 2}, TriangleIndex{0, 8, 11}, TriangleIndex{4, 9, 5} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 5, 4}, TriangleIndex{0, 1, 5}, TriangleIndex{2, 3, 11} , std::nullopt, std::nullopt} },
    { { TriangleIndex{2, 1, 5}, TriangleIndex{2, 5, 8}, TriangleIndex{2, 8, 11}, TriangleIndex{4, 8, 5} , std::nullopt} },
    { { TriangleIndex{10, 3, 11}, TriangleIndex{10, 1, 3}, TriangleIndex{9, 5, 4} , std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 9, 5}, TriangleIndex{0, 8, 1}, TriangleIndex{8, 10, 1}, TriangleIndex{8, 11, 10} , std::nullopt} },
    { { TriangleIndex{5, 4, 0}, TriangleIndex{5, 0, 11}, TriangleIndex{5, 11, 10}, TriangleIndex{11, 0, 3} , std::nullopt} },
    { { TriangleIndex{5, 4, 8}, TriangleIndex{5, 8, 10}, TriangleIndex{10, 8, 11} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 7, 8}, TriangleIndex{5, 7, 9} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 3, 0}, TriangleIndex{9, 5, 3}, TriangleIndex{5, 7, 3} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 7, 8}, TriangleIndex{0, 1, 7}, TriangleIndex{1, 5, 7} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 5, 3}, TriangleIndex{3, 5, 7} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 7, 8}, TriangleIndex{9, 5, 7}, TriangleIndex{10, 1, 2} , std::nullopt, std::nullopt} },
    { { TriangleIndex{10, 1, 2}, TriangleIndex{9, 5, 0}, TriangleIndex{5, 3, 0}, TriangleIndex{5, 7, 3} , std::nullopt} },
    { { TriangleIndex{8, 0, 2}, TriangleIndex{8, 2, 5}, TriangleIndex{8, 5, 7}, TriangleIndex{10, 5, 2} , std::nullopt} },
    { { TriangleIndex{2, 10, 5}, TriangleIndex{2, 5, 3}, TriangleIndex{3, 5, 7} , std::nullopt, std::nullopt} },
    { { TriangleIndex{7, 9, 5}, TriangleIndex{7, 8, 9}, TriangleIndex{3, 11, 2} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 5, 7}, TriangleIndex{9, 7, 2}, TriangleIndex{9, 2, 0}, TriangleIndex{2, 7, 11} , std::nullopt} },
    { { TriangleIndex{2, 3, 11}, TriangleIndex{0, 1, 8}, TriangleIndex{1, 7, 8}, TriangleIndex{1, 5, 7} , std::nullopt} },
    { { TriangleIndex{11, 2, 1}, TriangleIndex{11, 1, 7}, TriangleIndex{7, 1, 5} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 5, 8}, TriangleIndex{8, 5, 7}, TriangleIndex{10, 1, 3}, TriangleIndex{10, 3, 11} , std::nullopt} },
    { { TriangleIndex{5, 7, 0}, TriangleIndex{5, 0, 9}, TriangleIndex{7, 11, 0}, TriangleIndex{1, 0, 10}, TriangleIndex{11, 10, 0} } },
    { { TriangleIndex{11, 10, 0}, TriangleIndex{11, 0, 3}, TriangleIndex{10, 5, 0}, TriangleIndex{8, 0, 7}, TriangleIndex{5, 7, 0} } },
    { { TriangleIndex{11, 10, 5}, TriangleIndex{7, 11, 5} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{10, 6, 5}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{0, 8, 3}, TriangleIndex{5, 10, 6} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 0, 1}, TriangleIndex{5, 10, 6} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 8, 3}, TriangleIndex{1, 9, 8}, TriangleIndex{5, 10, 6} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 6, 5}, TriangleIndex{2, 6, 1} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 6, 5}, TriangleIndex{1, 2, 6}, TriangleIndex{3, 0, 8} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 6, 5}, TriangleIndex{9, 0, 6}, TriangleIndex{0, 2, 6} , std::nullopt, std::nullopt} },
    { { TriangleIndex{5, 9, 8}, TriangleIndex{5, 8, 2}, TriangleIndex{5, 2, 6}, TriangleIndex{3, 2, 8} , std::nullopt} },
    { { TriangleIndex{2, 3, 11}, TriangleIndex{10, 6, 5} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{11, 0, 8}, TriangleIndex{11, 2, 0}, TriangleIndex{10, 6, 5} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 1, 9}, TriangleIndex{2, 3, 11}, TriangleIndex{5, 10, 6} , std::nullopt, std::nullopt} },
    { { TriangleIndex{5, 10, 6}, TriangleIndex{1, 9, 2}, TriangleIndex{9, 11, 2}, TriangleIndex{9, 8, 11} , std::nullopt} },
    { { TriangleIndex{6, 3, 11}, TriangleIndex{6, 5, 3}, TriangleIndex{5, 1, 3} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 8, 11}, TriangleIndex{0, 11, 5}, TriangleIndex{0, 5, 1}, TriangleIndex{5, 11, 6} , std::nullopt} },
    { { TriangleIndex{3, 11, 6}, TriangleIndex{0, 3, 6}, TriangleIndex{0, 6, 5}, TriangleIndex{0, 5, 9} , std::nullopt} },
    { { TriangleIndex{6, 5, 9}, TriangleIndex{6, 9, 11}, TriangleIndex{11, 9, 8} , std::nullopt, std::nullopt} },
    { { TriangleIndex{5, 10, 6}, TriangleIndex{4, 7, 8} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 3, 0}, TriangleIndex{4, 7, 3}, TriangleIndex{6, 5, 10} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 9, 0}, TriangleIndex{5, 10, 6}, TriangleIndex{8, 4, 7} , std::nullopt, std::nullopt} },
    { { TriangleIndex{10, 6, 5}, TriangleIndex{1, 9, 7}, TriangleIndex{1, 7, 3}, TriangleIndex{7, 9, 4} , std::nullopt} },
    { { TriangleIndex{6, 1, 2}, TriangleIndex{6, 5, 1}, TriangleIndex{4, 7, 8} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 2, 5}, TriangleIndex{5, 2, 6}, TriangleIndex{3, 0, 4}, TriangleIndex{3, 4, 7} , std::nullopt} },
    { { TriangleIndex{8, 4, 7}, TriangleIndex{9, 0, 5}, TriangleIndex{0, 6, 5}, TriangleIndex{0, 2, 6} , std::nullopt} },
    { { TriangleIndex{7, 3, 9}, TriangleIndex{7, 9, 4}, TriangleIndex{3, 2, 9}, TriangleIndex{5, 9, 6}, TriangleIndex{2, 6, 9} } },
    { { TriangleIndex{3, 11, 2}, TriangleIndex{7, 8, 4}, TriangleIndex{10, 6, 5} , std::nullopt, std::nullopt} },
    { { TriangleIndex{5, 10, 6}, TriangleIndex{4, 7, 2}, TriangleIndex{4, 2, 0}, TriangleIndex{2, 7, 11} , std::nullopt} },
    { { TriangleIndex{0, 1, 9}, TriangleIndex{4, 7, 8}, TriangleIndex{2, 3, 11}, TriangleIndex{5, 10, 6} , std::nullopt} },
    { { TriangleIndex{9, 2, 1}, TriangleIndex{9, 11, 2}, TriangleIndex{9, 4, 11}, TriangleIndex{7, 11, 4}, TriangleIndex{5, 10, 6} } },
    { { TriangleIndex{8, 4, 7}, TriangleIndex{3, 11, 5}, TriangleIndex{3, 5, 1}, TriangleIndex{5, 11, 6} , std::nullopt} },
    { { TriangleIndex{5, 1, 11}, TriangleIndex{5, 11, 6}, TriangleIndex{1, 0, 11}, TriangleIndex{7, 11, 4}, TriangleIndex{0, 4, 11} } },
    { { TriangleIndex{0, 5, 9}, TriangleIndex{0, 6, 5}, TriangleIndex{0, 3, 6}, TriangleIndex{11, 6, 3}, TriangleIndex{8, 4, 7} } },
    { { TriangleIndex{6, 5, 9}, TriangleIndex{6, 9, 11}, TriangleIndex{4, 7, 9}, TriangleIndex{7, 11, 9} , std::nullopt} },
    { { TriangleIndex{10, 4, 9}, TriangleIndex{6, 4, 10} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 10, 6}, TriangleIndex{4, 9, 10}, TriangleIndex{0, 8, 3} , std::nullopt, std::nullopt} },
    { { TriangleIndex{10, 0, 1}, TriangleIndex{10, 6, 0}, TriangleIndex{6, 4, 0} , std::nullopt, std::nullopt} },
    { { TriangleIndex{8, 3, 1}, TriangleIndex{8, 1, 6}, TriangleIndex{8, 6, 4}, TriangleIndex{6, 1, 10} , std::nullopt} },
    { { TriangleIndex{1, 4, 9}, TriangleIndex{1, 2, 4}, TriangleIndex{2, 6, 4} , std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 0, 8}, TriangleIndex{1, 2, 9}, TriangleIndex{2, 4, 9}, TriangleIndex{2, 6, 4} , std::nullopt} },
    { { TriangleIndex{0, 2, 4}, TriangleIndex{4, 2, 6} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{8, 3, 2}, TriangleIndex{8, 2, 4}, TriangleIndex{4, 2, 6} , std::nullopt, std::nullopt} },
    { { TriangleIndex{10, 4, 9}, TriangleIndex{10, 6, 4}, TriangleIndex{11, 2, 3} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 8, 2}, TriangleIndex{2, 8, 11}, TriangleIndex{4, 9, 10}, TriangleIndex{4, 10, 6} , std::nullopt} },
    { { TriangleIndex{3, 11, 2}, TriangleIndex{0, 1, 6}, TriangleIndex{0, 6, 4}, TriangleIndex{6, 1, 10} , std::nullopt} },
    { { TriangleIndex{6, 4, 1}, TriangleIndex{6, 1, 10}, TriangleIndex{4, 8, 1}, TriangleIndex{2, 1, 11}, TriangleIndex{8, 11, 1} } },
    { { TriangleIndex{9, 6, 4}, TriangleIndex{9, 3, 6}, TriangleIndex{9, 1, 3}, TriangleIndex{11, 6, 3} , std::nullopt} },
    { { TriangleIndex{8, 11, 1}, TriangleIndex{8, 1, 0}, TriangleIndex{11, 6, 1}, TriangleIndex{9, 1, 4}, TriangleIndex{6, 4, 1} } },
    { { TriangleIndex{3, 11, 6}, TriangleIndex{3, 6, 0}, TriangleIndex{0, 6, 4} , std::nullopt, std::nullopt} },
    { { TriangleIndex{6, 4, 8}, TriangleIndex{11, 6, 8} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{7, 10, 6}, TriangleIndex{7, 8, 10}, TriangleIndex{8, 9, 10} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 7, 3}, TriangleIndex{0, 10, 7}, TriangleIndex{0, 9, 10}, TriangleIndex{6, 7, 10} , std::nullopt} },
    { { TriangleIndex{10, 6, 7}, TriangleIndex{1, 10, 7}, TriangleIndex{1, 7, 8}, TriangleIndex{1, 8, 0} , std::nullopt} },
    { { TriangleIndex{10, 6, 7}, TriangleIndex{10, 7, 1}, TriangleIndex{1, 7, 3} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 2, 6}, TriangleIndex{1, 6, 8}, TriangleIndex{1, 8, 9}, TriangleIndex{8, 6, 7} , std::nullopt} },
    { { TriangleIndex{2, 6, 9}, TriangleIndex{2, 9, 1}, TriangleIndex{6, 7, 9}, TriangleIndex{0, 9, 3}, TriangleIndex{7, 3, 9} } },
    { { TriangleIndex{7, 8, 0}, TriangleIndex{7, 0, 6}, TriangleIndex{6, 0, 2} , std::nullopt, std::nullopt} },
    { { TriangleIndex{7, 3, 2}, TriangleIndex{6, 7, 2} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{2, 3, 11}, TriangleIndex{10, 6, 8}, TriangleIndex{10, 8, 9}, TriangleIndex{8, 6, 7} , std::nullopt} },
    { { TriangleIndex{2, 0, 7}, TriangleIndex{2, 7, 11}, TriangleIndex{0, 9, 7}, TriangleIndex{6, 7, 10}, TriangleIndex{9, 10, 7} } },
    { { TriangleIndex{1, 8, 0}, TriangleIndex{1, 7, 8}, TriangleIndex{1, 10, 7}, TriangleIndex{6, 7, 10}, TriangleIndex{2, 3, 11} } },
    { { TriangleIndex{11, 2, 1}, TriangleIndex{11, 1, 7}, TriangleIndex{10, 6, 1}, TriangleIndex{6, 7, 1} , std::nullopt} },
    { { TriangleIndex{8, 9, 6}, TriangleIndex{8, 6, 7}, TriangleIndex{9, 1, 6}, TriangleIndex{11, 6, 3}, TriangleIndex{1, 3, 6} } },
    { { TriangleIndex{0, 9, 1}, TriangleIndex{11, 6, 7} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{7, 8, 0}, TriangleIndex{7, 0, 6}, TriangleIndex{3, 11, 0}, TriangleIndex{11, 6, 0} , std::nullopt} },
    { { TriangleIndex{7, 11, 6}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{7, 6, 11}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{3, 0, 8}, TriangleIndex{11, 7, 6} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 1, 9}, TriangleIndex{11, 7, 6} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{8, 1, 9}, TriangleIndex{8, 3, 1}, TriangleIndex{11, 7, 6} , std::nullopt, std::nullopt} },
    { { TriangleIndex{10, 1, 2}, TriangleIndex{6, 11, 7} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 2, 10}, TriangleIndex{3, 0, 8}, TriangleIndex{6, 11, 7} , std::nullopt, std::nullopt} },
    { { TriangleIndex{2, 9, 0}, TriangleIndex{2, 10, 9}, TriangleIndex{6, 11, 7} , std::nullopt, std::nullopt} },
    { { TriangleIndex{6, 11, 7}, TriangleIndex{2, 10, 3}, TriangleIndex{10, 8, 3}, TriangleIndex{10, 9, 8} , std::nullopt} },
    { { TriangleIndex{7, 2, 3}, TriangleIndex{6, 2, 7} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{7, 0, 8}, TriangleIndex{7, 6, 0}, TriangleIndex{6, 2, 0} , std::nullopt, std::nullopt} },
    { { TriangleIndex{2, 7, 6}, TriangleIndex{2, 3, 7}, TriangleIndex{0, 1, 9} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 6, 2}, TriangleIndex{1, 8, 6}, TriangleIndex{1, 9, 8}, TriangleIndex{8, 7, 6} , std::nullopt} },
    { { TriangleIndex{10, 7, 6}, TriangleIndex{10, 1, 7}, TriangleIndex{1, 3, 7} , std::nullopt, std::nullopt} },
    { { TriangleIndex{10, 7, 6}, TriangleIndex{1, 7, 10}, TriangleIndex{1, 8, 7}, TriangleIndex{1, 0, 8} , std::nullopt} },
    { { TriangleIndex{0, 3, 7}, TriangleIndex{0, 7, 10}, TriangleIndex{0, 10, 9}, TriangleIndex{6, 10, 7} , std::nullopt} },
    { { TriangleIndex{7, 6, 10}, TriangleIndex{7, 10, 8}, TriangleIndex{8, 10, 9} , std::nullopt, std::nullopt} },
    { { TriangleIndex{6, 8, 4}, TriangleIndex{11, 8, 6} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 6, 11}, TriangleIndex{3, 0, 6}, TriangleIndex{0, 4, 6} , std::nullopt, std::nullopt} },
    { { TriangleIndex{8, 6, 11}, TriangleIndex{8, 4, 6}, TriangleIndex{9, 0, 1} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 4, 6}, TriangleIndex{9, 6, 3}, TriangleIndex{9, 3, 1}, TriangleIndex{11, 3, 6} , std::nullopt} },
    { { TriangleIndex{6, 8, 4}, TriangleIndex{6, 11, 8}, TriangleIndex{2, 10, 1} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 2, 10}, TriangleIndex{3, 0, 11}, TriangleIndex{0, 6, 11}, TriangleIndex{0, 4, 6} , std::nullopt} },
    { { TriangleIndex{4, 11, 8}, TriangleIndex{4, 6, 11}, TriangleIndex{0, 2, 9}, TriangleIndex{2, 10, 9} , std::nullopt} },
    { { TriangleIndex{10, 9, 3}, TriangleIndex{10, 3, 2}, TriangleIndex{9, 4, 3}, TriangleIndex{11, 3, 6}, TriangleIndex{4, 6, 3} } },
    { { TriangleIndex{8, 2, 3}, TriangleIndex{8, 4, 2}, TriangleIndex{4, 6, 2} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 4, 2}, TriangleIndex{4, 6, 2} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 9, 0}, TriangleIndex{2, 3, 4}, TriangleIndex{2, 4, 6}, TriangleIndex{4, 3, 8} , std::nullopt} },
    { { TriangleIndex{1, 9, 4}, TriangleIndex{1, 4, 2}, TriangleIndex{2, 4, 6} , std::nullopt, std::nullopt} },
    { { TriangleIndex{8, 1, 3}, TriangleIndex{8, 6, 1}, TriangleIndex{8, 4, 6}, TriangleIndex{6, 10, 1} , std::nullopt} },
    { { TriangleIndex{10, 1, 0}, TriangleIndex{10, 0, 6}, TriangleIndex{6, 0, 4} , std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 6, 3}, TriangleIndex{4, 3, 8}, TriangleIndex{6, 10, 3}, TriangleIndex{0, 3, 9}, TriangleIndex{10, 9, 3} } },
    { { TriangleIndex{10, 9, 4}, TriangleIndex{6, 10, 4} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 9, 5}, TriangleIndex{7, 6, 11} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 8, 3}, TriangleIndex{4, 9, 5}, TriangleIndex{11, 7, 6} , std::nullopt, std::nullopt} },
    { { TriangleIndex{5, 0, 1}, TriangleIndex{5, 4, 0}, TriangleIndex{7, 6, 11} , std::nullopt, std::nullopt} },
    { { TriangleIndex{11, 7, 6}, TriangleIndex{8, 3, 4}, TriangleIndex{3, 5, 4}, TriangleIndex{3, 1, 5} , std::nullopt} },
    { { TriangleIndex{9, 5, 4}, TriangleIndex{10, 1, 2}, TriangleIndex{7, 6, 11} , std::nullopt, std::nullopt} },
    { { TriangleIndex{6, 11, 7}, TriangleIndex{1, 2, 10}, TriangleIndex{0, 8, 3}, TriangleIndex{4, 9, 5} , std::nullopt} },
    { { TriangleIndex{7, 6, 11}, TriangleIndex{5, 4, 10}, TriangleIndex{4, 2, 10}, TriangleIndex{4, 0, 2} , std::nullopt} },
    { { TriangleIndex{3, 4, 8}, TriangleIndex{3, 5, 4}, TriangleIndex{3, 2, 5}, TriangleIndex{10, 5, 2}, TriangleIndex{11, 7, 6} } },
    { { TriangleIndex{7, 2, 3}, TriangleIndex{7, 6, 2}, TriangleIndex{5, 4, 9} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 5, 4}, TriangleIndex{0, 8, 6}, TriangleIndex{0, 6, 2}, TriangleIndex{6, 8, 7} , std::nullopt} },
    { { TriangleIndex{3, 6, 2}, TriangleIndex{3, 7, 6}, TriangleIndex{1, 5, 0}, TriangleIndex{5, 4, 0} , std::nullopt} },
    { { TriangleIndex{6, 2, 8}, TriangleIndex{6, 8, 7}, TriangleIndex{2, 1, 8}, TriangleIndex{4, 8, 5}, TriangleIndex{1, 5, 8} } },
    { { TriangleIndex{9, 5, 4}, TriangleIndex{10, 1, 6}, TriangleIndex{1, 7, 6}, TriangleIndex{1, 3, 7} , std::nullopt} },
    { { TriangleIndex{1, 6, 10}, TriangleIndex{1, 7, 6}, TriangleIndex{1, 0, 7}, TriangleIndex{8, 7, 0}, TriangleIndex{9, 5, 4} } },
    { { TriangleIndex{4, 0, 10}, TriangleIndex{4, 10, 5}, TriangleIndex{0, 3, 10}, TriangleIndex{6, 10, 7}, TriangleIndex{3, 7, 10} } },
    { { TriangleIndex{7, 6, 10}, TriangleIndex{7, 10, 8}, TriangleIndex{5, 4, 10}, TriangleIndex{4, 8, 10} , std::nullopt} },
    { { TriangleIndex{6, 9, 5}, TriangleIndex{6, 11, 9}, TriangleIndex{11, 8, 9} , std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 6, 11}, TriangleIndex{0, 6, 3}, TriangleIndex{0, 5, 6}, TriangleIndex{0, 9, 5} , std::nullopt} },
    { { TriangleIndex{0, 11, 8}, TriangleIndex{0, 5, 11}, TriangleIndex{0, 1, 5}, TriangleIndex{5, 6, 11} , std::nullopt} },
    { { TriangleIndex{6, 11, 3}, TriangleIndex{6, 3, 5}, TriangleIndex{5, 3, 1} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 2, 10}, TriangleIndex{9, 5, 11}, TriangleIndex{9, 11, 8}, TriangleIndex{11, 5, 6} , std::nullopt} },
    { { TriangleIndex{0, 11, 3}, TriangleIndex{0, 6, 11}, TriangleIndex{0, 9, 6}, TriangleIndex{5, 6, 9}, TriangleIndex{1, 2, 10} } },
    { { TriangleIndex{11, 8, 5}, TriangleIndex{11, 5, 6}, TriangleIndex{8, 0, 5}, TriangleIndex{10, 5, 2}, TriangleIndex{0, 2, 5} } },
    { { TriangleIndex{6, 11, 3}, TriangleIndex{6, 3, 5}, TriangleIndex{2, 10, 3}, TriangleIndex{10, 5, 3} , std::nullopt} },
    { { TriangleIndex{5, 8, 9}, TriangleIndex{5, 2, 8}, TriangleIndex{5, 6, 2}, TriangleIndex{3, 8, 2} , std::nullopt} },
    { { TriangleIndex{9, 5, 6}, TriangleIndex{9, 6, 0}, TriangleIndex{0, 6, 2} , std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 5, 8}, TriangleIndex{1, 8, 0}, TriangleIndex{5, 6, 8}, TriangleIndex{3, 8, 2}, TriangleIndex{6, 2, 8} } },
    { { TriangleIndex{1, 5, 6}, TriangleIndex{2, 1, 6} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 3, 6}, TriangleIndex{1, 6, 10}, TriangleIndex{3, 8, 6}, TriangleIndex{5, 6, 9}, TriangleIndex{8, 9, 6} } },
    { { TriangleIndex{10, 1, 0}, TriangleIndex{10, 0, 6}, TriangleIndex{9, 5, 0}, TriangleIndex{5, 6, 0} , std::nullopt} },
    { { TriangleIndex{0, 3, 8}, TriangleIndex{5, 6, 10} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{10, 5, 6}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{11, 5, 10}, TriangleIndex{7, 5, 11} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{11, 5, 10}, TriangleIndex{11, 7, 5}, TriangleIndex{8, 3, 0} , std::nullopt, std::nullopt} },
    { { TriangleIndex{5, 11, 7}, TriangleIndex{5, 10, 11}, TriangleIndex{1, 9, 0} , std::nullopt, std::nullopt} },
    { { TriangleIndex{10, 7, 5}, TriangleIndex{10, 11, 7}, TriangleIndex{9, 8, 1}, TriangleIndex{8, 3, 1} , std::nullopt} },
    { { TriangleIndex{11, 1, 2}, TriangleIndex{11, 7, 1}, TriangleIndex{7, 5, 1} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 8, 3}, TriangleIndex{1, 2, 7}, TriangleIndex{1, 7, 5}, TriangleIndex{7, 2, 11} , std::nullopt} },
    { { TriangleIndex{9, 7, 5}, TriangleIndex{9, 2, 7}, TriangleIndex{9, 0, 2}, TriangleIndex{2, 11, 7} , std::nullopt} },
    { { TriangleIndex{7, 5, 2}, TriangleIndex{7, 2, 11}, TriangleIndex{5, 9, 2}, TriangleIndex{3, 2, 8}, TriangleIndex{9, 8, 2} } },
    { { TriangleIndex{2, 5, 10}, TriangleIndex{2, 3, 5}, TriangleIndex{3, 7, 5} , std::nullopt, std::nullopt} },
    { { TriangleIndex{8, 2, 0}, TriangleIndex{8, 5, 2}, TriangleIndex{8, 7, 5}, TriangleIndex{10, 2, 5} , std::nullopt} },
    { { TriangleIndex{9, 0, 1}, TriangleIndex{5, 10, 3}, TriangleIndex{5, 3, 7}, TriangleIndex{3, 10, 2} , std::nullopt} },
    { { TriangleIndex{9, 8, 2}, TriangleIndex{9, 2, 1}, TriangleIndex{8, 7, 2}, TriangleIndex{10, 2, 5}, TriangleIndex{7, 5, 2} } },
    { { TriangleIndex{1, 3, 5}, TriangleIndex{3, 7, 5} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 8, 7}, TriangleIndex{0, 7, 1}, TriangleIndex{1, 7, 5} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 0, 3}, TriangleIndex{9, 3, 5}, TriangleIndex{5, 3, 7} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 8, 7}, TriangleIndex{5, 9, 7} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{5, 8, 4}, TriangleIndex{5, 10, 8}, TriangleIndex{10, 11, 8} , std::nullopt, std::nullopt} },
    { { TriangleIndex{5, 0, 4}, TriangleIndex{5, 11, 0}, TriangleIndex{5, 10, 11}, TriangleIndex{11, 3, 0} , std::nullopt} },
    { { TriangleIndex{0, 1, 9}, TriangleIndex{8, 4, 10}, TriangleIndex{8, 10, 11}, TriangleIndex{10, 4, 5} , std::nullopt} },
    { { TriangleIndex{10, 11, 4}, TriangleIndex{10, 4, 5}, TriangleIndex{11, 3, 4}, TriangleIndex{9, 4, 1}, TriangleIndex{3, 1, 4} } },
    { { TriangleIndex{2, 5, 1}, TriangleIndex{2, 8, 5}, TriangleIndex{2, 11, 8}, TriangleIndex{4, 5, 8} , std::nullopt} },
    { { TriangleIndex{0, 4, 11}, TriangleIndex{0, 11, 3}, TriangleIndex{4, 5, 11}, TriangleIndex{2, 11, 1}, TriangleIndex{5, 1, 11} } },
    { { TriangleIndex{0, 2, 5}, TriangleIndex{0, 5, 9}, TriangleIndex{2, 11, 5}, TriangleIndex{4, 5, 8}, TriangleIndex{11, 8, 5} } },
    { { TriangleIndex{9, 4, 5}, TriangleIndex{2, 11, 3} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{2, 5, 10}, TriangleIndex{3, 5, 2}, TriangleIndex{3, 4, 5}, TriangleIndex{3, 8, 4} , std::nullopt} },
    { { TriangleIndex{5, 10, 2}, TriangleIndex{5, 2, 4}, TriangleIndex{4, 2, 0} , std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 10, 2}, TriangleIndex{3, 5, 10}, TriangleIndex{3, 8, 5}, TriangleIndex{4, 5, 8}, TriangleIndex{0, 1, 9} } },
    { { TriangleIndex{5, 10, 2}, TriangleIndex{5, 2, 4}, TriangleIndex{1, 9, 2}, TriangleIndex{9, 4, 2} , std::nullopt} },
    { { TriangleIndex{8, 4, 5}, TriangleIndex{8, 5, 3}, TriangleIndex{3, 5, 1} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 4, 5}, TriangleIndex{1, 0, 5} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{8, 4, 5}, TriangleIndex{8, 5, 3}, TriangleIndex{9, 0, 5}, TriangleIndex{0, 3, 5} , std::nullopt} },
    { { TriangleIndex{9, 4, 5}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{4, 11, 7}, TriangleIndex{4, 9, 11}, TriangleIndex{9, 10, 11} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 8, 3}, TriangleIndex{4, 9, 7}, TriangleIndex{9, 11, 7}, TriangleIndex{9, 10, 11} , std::nullopt} },
    { { TriangleIndex{1, 10, 11}, TriangleIndex{1, 11, 4}, TriangleIndex{1, 4, 0}, TriangleIndex{7, 4, 11} , std::nullopt} },
    { { TriangleIndex{3, 1, 4}, TriangleIndex{3, 4, 8}, TriangleIndex{1, 10, 4}, TriangleIndex{7, 4, 11}, TriangleIndex{10, 11, 4} } },
    { { TriangleIndex{4, 11, 7}, TriangleIndex{9, 11, 4}, TriangleIndex{9, 2, 11}, TriangleIndex{9, 1, 2} , std::nullopt} },
    { { TriangleIndex{9, 7, 4}, TriangleIndex{9, 11, 7}, TriangleIndex{9, 1, 11}, TriangleIndex{2, 11, 1}, TriangleIndex{0, 8, 3} } },
    { { TriangleIndex{11, 7, 4}, TriangleIndex{11, 4, 2}, TriangleIndex{2, 4, 0} , std::nullopt, std::nullopt} },
    { { TriangleIndex{11, 7, 4}, TriangleIndex{11, 4, 2}, TriangleIndex{8, 3, 4}, TriangleIndex{3, 2, 4} , std::nullopt} },
    { { TriangleIndex{2, 9, 10}, TriangleIndex{2, 7, 9}, TriangleIndex{2, 3, 7}, TriangleIndex{7, 4, 9} , std::nullopt} },
    { { TriangleIndex{9, 10, 7}, TriangleIndex{9, 7, 4}, TriangleIndex{10, 2, 7}, TriangleIndex{8, 7, 0}, TriangleIndex{2, 0, 7} } },
    { { TriangleIndex{3, 7, 10}, TriangleIndex{3, 10, 2}, TriangleIndex{7, 4, 10}, TriangleIndex{1, 10, 0}, TriangleIndex{4, 0, 10} } },
    { { TriangleIndex{1, 10, 2}, TriangleIndex{8, 7, 4} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 9, 1}, TriangleIndex{4, 1, 7}, TriangleIndex{7, 1, 3} , std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 9, 1}, TriangleIndex{4, 1, 7}, TriangleIndex{0, 8, 1}, TriangleIndex{8, 7, 1} , std::nullopt} },
    { { TriangleIndex{4, 0, 3}, TriangleIndex{7, 4, 3} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{4, 8, 7}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{9, 10, 8}, TriangleIndex{10, 11, 8} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 0, 9}, TriangleIndex{3, 9, 11}, TriangleIndex{11, 9, 10} , std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 1, 10}, TriangleIndex{0, 10, 8}, TriangleIndex{8, 10, 11} , std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 1, 10}, TriangleIndex{11, 3, 10} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{1, 2, 11}, TriangleIndex{1, 11, 9}, TriangleIndex{9, 11, 8} , std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 0, 9}, TriangleIndex{3, 9, 11}, TriangleIndex{1, 2, 9}, TriangleIndex{2, 11, 9} , std::nullopt} },
    { { TriangleIndex{0, 2, 11}, TriangleIndex{8, 0, 11} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{3, 2, 11}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{2, 3, 8}, TriangleIndex{2, 8, 10}, TriangleIndex{10, 8, 9} , std::nullopt, std::nullopt} },
    { { TriangleIndex{9, 10, 2}, TriangleIndex{0, 9, 2} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{2, 3, 8}, TriangleIndex{2, 8, 10}, TriangleIndex{0, 1, 8}, TriangleIndex{1, 10, 8} , std::nullopt} },
    { { TriangleIndex{1, 10, 2}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{1, 3, 8}, TriangleIndex{9, 1, 8} , std::nullopt, std::nullopt, std::nullopt} },
    { { TriangleIndex{0, 9, 1}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { TriangleIndex{0, 3, 8}, std::nullopt, std::nullopt, std::nullopt, std::nullopt } },
    { { std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt }}
}};
// clang-format on

/// edge_to_vertices[i] = {a, b} => edge i joins vertices a and b
static constexpr auto edge_to_vertices = std::array<std::pair<unsigned int, unsigned int>, 12>{{{0, 1}, {1, 2}, {2, 3}, {0, 3}, {4, 5}, {5, 6}, {6, 7}, {4, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}}};

/// Given a `cell`, calculate its cube index
/// The cube index is an 8-bit encoding. Each bit represents a vertex. `index[i]` is the i-th bit
/// If the value at the i-th vertex is < sdf_value, `index[i]` = 1. Else, `index[i]` = 0
static auto calculate_cube_index(GridCell const& cell, float const sdf_value)
{
    unsigned int cubeIndex{0};
    for (unsigned int i{0}; i < 8; i++)
    {
        if (cell.sdf_values[i] < sdf_value)
        {
            cubeIndex |= (1 << i);
        }
    }

    return cubeIndex;
}

auto interpolate(glm::vec3 const& lhs_pos, float lhs_sdf, glm::vec3 const& rhs_pos, float rhs_sdf, float sdf_value)
{
    static constexpr auto epsilon{std::numeric_limits<float>::epsilon()};
    if (std::fabs(sdf_value - lhs_sdf) < epsilon)
        return lhs_pos;
    if (std::fabs(sdf_value - rhs_sdf) < epsilon)
        return rhs_pos;
    if (std::fabs(lhs_sdf - rhs_sdf) < epsilon)
        return lhs_pos;

    return ((sdf_value - lhs_sdf) / (rhs_sdf - lhs_sdf)) * (rhs_pos - lhs_pos) + lhs_pos;
}

auto get_intersection_coordinates(GridCell const& cell, int intersections_key, float sdf_value)
{
    std::array<glm::vec3, 12> intersections{};

    unsigned int idx{0};
    while (intersections_key)
    {
        if (intersections_key & 1)
        {
            auto const& [v1, v2]        = edge_to_vertices[idx];
            glm::vec3 intersectionPoint = interpolate(cell.vertices[v1], cell.sdf_values[v1], cell.vertices[v2], cell.sdf_values[v2], sdf_value);
            intersections[idx]          = intersectionPoint;
        }
        idx++;
        intersections_key >>= 1;
    }

    return intersections;
}

auto get_triangles(std::array<glm::vec3, 12> const& intersections, unsigned int cubeIndex)
{
    std::vector<Triangle> triangles{};

    for (auto const& optional_triangle_indices : triangle_table[cubeIndex])
    {
        if (optional_triangle_indices.has_value())
        {
            auto const& triangle_indices = optional_triangle_indices.value();
            triangles.push_back(Triangle{intersections[triangle_indices.x], intersections[triangle_indices.y], intersections[triangle_indices.z]});
        }
    }

    return triangles;
}

static auto triangulate_cell(GridCell const& cell, float sdf_value)
{
    auto const cube_index{calculate_cube_index(cell, sdf_value)};
    int const  intersections_key{edge_table[cube_index]};
    auto const intersection_coordinates{get_intersection_coordinates(cell, intersections_key, sdf_value)};
    return get_triangles(intersection_coordinates, cube_index);
}

static auto hash_vertices_to_indices(std::vector<Triangle> const& triangles)
{
    std::unordered_map<glm::vec3, unsigned int> vertices_map{};
    std::vector<glm::vec3>                      vertices{};
    std::vector<glm::uvec3>                     indices{};

    for (auto const& triangle : triangles)
    {
        // Add vertices to the vertices vector if not already present
        for (auto const& vertex : triangle)
        {
            if (!vertices_map.contains(vertex))
            {
                vertices.push_back(vertex);
                vertices_map[vertex] = static_cast<unsigned int>(vertices.size()) - 1;
            }
        }
        indices.emplace_back(vertices_map[triangle[0]], vertices_map[triangle[1]], vertices_map[triangle[2]]);
    }

    return std::make_pair(vertices, indices);
}

auto gen_mesh_with_marching_cubes(std::vector<float> const& sdf_samples, MeshingSettings const& settings) -> Cool::Mesh
{
    std::vector<Triangle> triangles{};

    float sdf_value{0};

    auto const sampling_count = settings.samples_count.x; // TODO(Meshing) support different sample counts on each axis
    auto const step_size      = settings.step_size().x;   // TODO(Meshing) same
    auto const box_size       = settings.box_size.x;      // TODO(Meshing) same

    unsigned int sampling_count_squared{sampling_count * sampling_count};

    for (unsigned int i{0}; i < sampling_count - 1; i++)
    {
        for (unsigned int j{0}; j < sampling_count - 1; j++)
        {
            for (unsigned int k{0}; k < sampling_count - 1; k++)
            {
                float x{static_cast<float>(i) * step_size - box_size / 2.f};
                float y{static_cast<float>(j) * step_size - box_size / 2.f};
                float z{static_cast<float>(k) * step_size - box_size / 2.f};

                GridCell cell{
                    // clang-format off
                    {{
                    {x, y, z}, {x + step_size, y, z},
                    {x + step_size, y, z + step_size}, {x, y, z + step_size},
                    {x, y + step_size, z}, {x + step_size, y + step_size, z},
                    {x + step_size, y + step_size, z + step_size}, {x, y + step_size, z + step_size}
                    }},
                    {
                    sdf_samples[i + j * sampling_count + k * sampling_count_squared],
                    sdf_samples[(i + 1) + j * sampling_count + k * sampling_count_squared],
                    sdf_samples[(i + 1) + j * sampling_count + (k + 1) * sampling_count_squared],
                    sdf_samples[i + j * sampling_count + (k + 1) * sampling_count_squared],
                    sdf_samples[i + (j + 1) * sampling_count + k * sampling_count_squared],
                    sdf_samples[(i + 1) + (j + 1) * sampling_count + k * sampling_count_squared],
                    sdf_samples[(i + 1) + (j + 1) * sampling_count + (k + 1) * sampling_count_squared],
                    sdf_samples[i + (j + 1) * sampling_count + (k + 1) * sampling_count_squared]
                    }
                    // clang-format on
                };

                auto cellTriangles{triangulate_cell(cell, sdf_value)};
                for (const auto& cellTriangle : cellTriangles)
                    triangles.push_back(cellTriangle);
            }
        }
    }

    auto const [vertices, indices] = hash_vertices_to_indices(triangles);

    // TODO(Meshing) Can we remove this dead code?
    // std::vector<glm::vec3>  vertices{};
    // std::vector<glm::uvec3> indices{};

    // for (size_t i{0}; i < triangles.size(); i++)
    // {
    //     vertices.push_back(triangles[i][0]);
    //     vertices.push_back(triangles[i][1]);
    //     vertices.push_back(triangles[i][2]);

    //     indices.emplace_back(3 * i, 3 * i + 1, 3 * i + 2);
    // }

    return Cool::Mesh{vertices, indices};
}

} // namespace Lab