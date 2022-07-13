
class ChunkLoader {
public:
    void request(glm::ivec2);
    void unload(std::unique_ptr<Chunk> &&);
    std::unique_ptr<Chunk> pop();
private:
    static void chunk_thread_fn(const std::string);

    static std::mutex lock;
    static std::vector<std::unique_ptr<Chunk>> full_chunks;
    static std::vector<std::unique_ptr<Chunk>> unloading_chunks;
    static std::vector<glm::ivec2> requested_chunks;
    static std::thread chunk_thread;
};

std::mutex ChunkLoader::lock;
std::vector<std::unique_ptr<Chunk>> ChunkLoader::full_chunks = {};
std::vector<std::unique_ptr<Chunk>> ChunkLoader::unloading_chunks = {};
std::vector<glm::ivec2> ChunkLoader::requested_chunks = {};
std::thread ChunkLoader::chunk_thread = std::thread(/*ChunkLoader::chunk_thread_fn, std::string("saves/save0.hex")*/);

void ChunkLoader::request(glm::ivec2 pos) {
    std::scoped_lock<std::mutex> l(lock);

    requested_chunks.push_back(pos);
}
void ChunkLoader::unload(std::unique_ptr<Chunk> &&chunk) {
    std::scoped_lock<std::mutex> l(lock);

    unloading_chunks.emplace_back(std::move(chunk));
}
std::unique_ptr<Chunk> ChunkLoader::pop() {
    std::scoped_lock<std::mutex> l(lock);

    std::unique_ptr<Chunk> last;
    if (full_chunks.size() > 0) {
        last = std::move(full_chunks.back());
    }
    return last;
}
void ChunkLoader::chunk_thread_fn(const std::string s) {
    std::vector<glm::ivec2> requested_chunks;
    std::vector<std::unique_ptr<Chunk>> empty_chunks;
    std::vector<std::unique_ptr<Chunk>> unloading_chunks;

    std::fstream save_file(s);

    assert(save_file.good());

    while (true) {
        {
            std::scoped_lock<std::mutex> l(ChunkLoader::lock);
            while (ChunkLoader::requested_chunks.size() > 0) {
                requested_chunks.emplace_back(std::move(ChunkLoader::requested_chunks.back()));
                ChunkLoader::requested_chunks.pop_back();
            }
            while (ChunkLoader::unloading_chunks.size() > 0) {
                unloading_chunks.emplace_back(std::move(ChunkLoader::unloading_chunks.back()));
                ChunkLoader::unloading_chunks.pop_back();
            }
        }

        for (const auto &chunk_pos : requested_chunks) {
            std::unique_ptr<Chunk> chunk;
            if (empty_chunks.size() > 0) {
                chunk = std::move(empty_chunks.back());
                empty_chunks.pop_back();
            } else {
                chunk = std::make_unique<Chunk>();
            }


        }
    }
}
