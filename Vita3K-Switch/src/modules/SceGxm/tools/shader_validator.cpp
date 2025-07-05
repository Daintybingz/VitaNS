#include "../SceGxm.h"
#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <shader_file>" << std::endl;
        return 1;
    }

    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "Error opening shader file" << std::endl;
        return 1;
    }

    // [Do not instantiate SceGxm directly. Use static/utility functions or provide a minimal mock if needed.]
    
    // Read shader data
    std::vector<uint8_t> shaderData((std::istreambuf_iterator<char>(input)),
                                  std::istreambuf_iterator<char>());

    // Create shader program
    SceGxmProgram program;
    program.data = shaderData.data();
    program.size = shaderData.size();
    program.type = SCE_GXM_PROGRAM_TYPE_VERTEX; // Default to vertex shader

    // Validate shader
    if (SceGxm::validateShaderProgram(&program)) {
        std::cout << "Shader validation passed" << std::endl;
        std::cout << "Validation flags: " << program.validationFlags << std::endl;
    } else {
        std::cerr << "Shader validation failed: " << program.lastValidationError << std::endl;
        std::cerr << "Validation errors: " << program.validationErrors << std::endl;
        return 1;
    }

    return 0;
}
