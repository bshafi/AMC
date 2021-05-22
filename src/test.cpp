#include "standard.hpp"
#include "chunk.hpp"

int main() {
    Chunk chunk;
    for (int i = 0; i < 10; ++i) {
        chunk.SetBlock(glm::ivec3(i, 0, 0), static_cast<uint32_t>(BlockType::Stone));
    }
    Ray ray = { glm::vec3(0, -10, 0) , glm::normalize(glm::vec3(0, 1, 0)) };
    auto hit = ray.cast(chunk, 10);
    if (hit.has_value()) {
        std::cout << *hit << std::endl;
        const glm::vec3 thing = (*hit) * ray.direction + ray.endpoint;
        std::cout << thing.x << ", " << thing.y << ", " << thing.z << std::endl;
        std::cout << chunk.GetBlock(glm::ivec3((*hit) * ray.direction + ray.endpoint));
    } else {
        std::cout << "nullopt" << std::endl;
    }
}