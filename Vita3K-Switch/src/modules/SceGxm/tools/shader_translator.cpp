#include "../SceGxm.h"
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_shader> <output_shader>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1], std::ios::binary);
    std::ofstream output(argv[2], std::ios::binary);

    if (!input || !output) {
        std::cerr << "Error opening files" << std::endl;
        return 1;
    }

    // [Do not instantiate SceGxm directly. Use static/utility functions or provide a minimal mock if needed.]
    
    // Read shader data
    std::vector<uint8_t> shaderData((std::istreambuf_iterator<char>(input)),
                                  std::istreambuf_iterator<char>());

    // Translate shader
    std::string glslCode = SceGxm::translateShader(shaderData.data(), shaderData.size(), false);
    
    if (glslCode.empty()) {
        std::cerr << "Shader translation failed" << std::endl;
        return 1;
    }

    // Write GLSL code
    output << glslCode;
    
    return 0;
}
