#ifndef MODEL_H
#define MODEL_H
#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "mesh.h"
#include "stb_image.h"
#include <unordered_map>
#include "stb_image_loader.h"

class Model {
public:
    std::vector<Mesh> meshes;
    std::string directory;

    Model(const std::string& path) {
        loadModel(path);
    }

    void Draw(Shader& shader) {
        for (auto& mesh : meshes) {
            mesh.Draw(shader);
        }
    }

private:
    void loadModel(const std::string& path) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        directory = path.substr(0, path.find_last_of("/\\"));

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), directory.c_str());

        if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
        if (!err.empty()) std::cerr << "ERR: " << err << std::endl;
        if (!ret) throw std::runtime_error("Failed to load model: " + path);

        std::unordered_map<std::string, Texture> loaded_textures;

        for (const auto& shape : shapes) {
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            for (const auto& index : shape.mesh.indices) {
                Vertex vertex = {};
                vertex.Position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                vertex.Normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
                if (!attrib.texcoords.empty()) {
                    vertex.TexCoords = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }
                else {
                    vertex.TexCoords = glm::vec2(0.0f);
                }

                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }

            std::vector<Texture> mesh_textures;
            int mat_id = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[0];
            if (mat_id >= 0 && mat_id < materials.size()) {
                std::string tex_filename = materials[mat_id].diffuse_texname;
                if (!tex_filename.empty()) {
                    std::string full_path = directory + "/" + tex_filename;
                    if (loaded_textures.find(full_path) == loaded_textures.end()) {
                        Texture tex;
                        tex.id = loadTexture(full_path.c_str());
                        tex.type = "texture_diffuse1";
                        tex.path = full_path;
                        loaded_textures[full_path] = tex;
                    }
                    mesh_textures.push_back(loaded_textures[full_path]);
                }
            }

            meshes.emplace_back(vertices, indices, mesh_textures);
        }
    }

    unsigned int loadTexture(const char* path) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format = (nrComponents == 1) ? GL_RED :
                (nrComponents == 3) ? GL_RGB : GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else {
            std::cerr << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }
        return textureID;
    }
};

#endif
