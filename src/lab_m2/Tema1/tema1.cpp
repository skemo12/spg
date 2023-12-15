#include "lab_m2/Tema1/tema1.h"
#include "components/transform.h"
#include "stb/stb_image.h"

#include <iostream>
#include <limits>
#include <vector>

using namespace std;
using namespace m2;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

Tema1::Tema1() {}

Tema1::~Tema1() {}

void Tema1::Init()
{
    nrInstances = 0;
    maxInstances = 50;
    shrink = 1.0;
    mirrorPosition = glm::vec3(0, 0, 0);
    mirrorRotationOX = 0;
    mirrorRotationOY = 0;
    mirrorRotationOZ = 0;
    ifParticle = 0;
    ifCountour = 0;
    auto camera = GetSceneCamera();
    camera->SetPositionAndRotation(glm::vec3(0, 5, 4),
                                   glm::quat(glm::vec3(-30 * TO_RADIANS, 0, 0)));
    camera->Update();
    std::string shaderPath =
        PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders");

    // Load a mesh from file into GPU memory
    {
        Mesh *mesh = new Mesh("quad");
        mesh->LoadMesh(
            PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"),
            "screen_quad.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh *mesh = new Mesh("cube");
        mesh->LoadMesh(
            PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"),
            "box.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh *mesh = new Mesh("archer");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS,
                                 "characters", "archer"),
                       "Archer.fbx");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    // Create a shader program for rendering outline
    {
        Shader *shader = new Shader("outline");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1",
                                    "shaders", "OutlineVS.glsl"),
                          GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1",
                                    "shaders", "OutlineGS.glsl"),
                          GL_GEOMETRY_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1",
                                    "shaders", "OutlineFS.glsl"),
                          GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Shader for particles
    {
        Shader *shader = new Shader("particles");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1",
                                    "shaders", "ParticleVS.glsl"),
                          GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1",
                                    "shaders", "ParticleGS.glsl"),
                          GL_GEOMETRY_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1",
                                    "shaders", "ParticleFS.glsl"),
                          GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
    // Create a shader program for rendering cubemap texture
    {
        Shader *shader = new Shader("CubeMap");
        shader->AddShader(PATH_JOIN(shaderPath, "CubeMapVS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "CubeMapFS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Create a shader program for creating a CUBEMAP
    {
        Shader *shader = new Shader("Framebuffer");
        shader->AddShader(PATH_JOIN(shaderPath, "FramebufferVS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FramebufferFS.glsl"),
                          GL_FRAGMENT_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FramebufferGS.glsl"),
                          GL_GEOMETRY_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Create a shader program for standard rendering
    {
        Shader *shader = new Shader("ShaderNormal");
        shader->AddShader(PATH_JOIN(shaderPath, "NormalVS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "NormalFS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        TextureManager::LoadTexture(
            PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "particle2.png");
    }

    generator_position = glm::vec3(0, 0, 0);
    scene = 0;
    offset = 0.05f;
    InitParticles(20, 20, 20);
    color_texture = 0;
    depth_texture = 0;

    angle = 0;

    type = 0;
    std::string texturePath =
        PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube");

    cubeMapTextureID = UploadCubeMapTexture(
        PATH_JOIN(texturePath, "pos_x.png"), PATH_JOIN(texturePath, "pos_y.png"),
        PATH_JOIN(texturePath, "pos_z.png"), PATH_JOIN(texturePath, "neg_x.png"),
        PATH_JOIN(texturePath, "neg_y.png"), PATH_JOIN(texturePath, "neg_z.png"));
    // Create the framebuffer on which the scene is rendered from the perspective of the
    // mesh Texture size must be cubic
    CreateFramebuffer(1024, 1024);
}

void Tema1::CreateFramebuffer(int width, int height)
{
    // TODO(student): In this method, use the attributes
    // 'framebuffer_object', 'color_texture'
    // declared in lab6.h

    // TODO(student): Generate and bind the framebuffer
    glGenFramebuffers(1, &framebuffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
    // TODO(student): Generate and bind the color texture
    glGenTextures(1, &color_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);

    // TODO(student): Initialize the color textures
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, NULL);

    if (color_texture)
    {
        // cubemap params
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (GLEW_EXT_texture_filter_anisotropic)
        {
            float maxAnisotropy;

            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Bind the color textures to the framebuffer as a color attachments
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        std::vector<GLenum> draw_textures;
        draw_textures.push_back(GL_COLOR_ATTACHMENT0);
        glDrawBuffers(draw_textures.size(), &draw_textures[0]);
    }

    // TODO(student): Generate and bind the depth texture
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_texture);

    // TODO(student): Initialize the depth textures
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    if (depth_texture)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
    }

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int
Tema1::UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y,
                            const std::string &pos_z, const std::string &neg_x,
                            const std::string &neg_y, const std::string &neg_z)
{
    int width, height, chn;

    unsigned char *data_pos_x = stbi_load(pos_x.c_str(), &width, &height, &chn, 0);
    unsigned char *data_pos_y = stbi_load(pos_y.c_str(), &width, &height, &chn, 0);
    unsigned char *data_pos_z = stbi_load(pos_z.c_str(), &width, &height, &chn, 0);
    unsigned char *data_neg_x = stbi_load(neg_x.c_str(), &width, &height, &chn, 0);
    unsigned char *data_neg_y = stbi_load(neg_y.c_str(), &width, &height, &chn, 0);
    unsigned char *data_neg_z = stbi_load(neg_z.c_str(), &width, &height, &chn, 0);

    unsigned int textureID = 0;
    // TODO(student): Create the texture
    glGenTextures(1, &textureID);

    // TODO(student): Bind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (GLEW_EXT_texture_filter_anisotropic)
    {
        float maxAnisotropy;

        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // TODO(student): Load texture information for each face
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data_pos_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data_pos_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data_pos_z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data_neg_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data_neg_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data_neg_z);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        cout << "\t[NOTE] : For students : DON'T PANIC! This error should go away when "
                "completing the tasks."
             << std::endl;
    }

    // Free memory
    SAFE_FREE(data_pos_x);
    SAFE_FREE(data_pos_y);
    SAFE_FREE(data_pos_z);
    SAFE_FREE(data_neg_x);
    SAFE_FREE(data_neg_y);
    SAFE_FREE(data_neg_z);

    return textureID;
}

ParticleEffect<Particle> *particleEffect;

void Tema1::InitParticles(int xSize, int ySize, int zSize)
{
    unsigned int nrParticles = 500;

    particleEffect = new ParticleEffect<Particle>();
    particleEffect->Generate(nrParticles, true);

    auto particleSSBO = particleEffect->GetParticleBuffer();
    Particle *data = const_cast<Particle *>(particleSSBO->GetBuffer());

    for (unsigned int i = 0; i < nrParticles; i++)
    {
        glm::vec3 pos(0);
        glm::vec3 speed(0);

        float delay = (rand() % 200) / 20.0f + (rand() % 1000) / 250.0f;
        float lifetime = 2.f + (rand() % 10);
        data[i].SetInitial(glm::vec4(pos, 1), glm::vec4(speed, 0), delay, lifetime);
    }

    particleSSBO->SetBufferData(data);
}

void Tema1::FrameStart() { ClearScreen(); }

void Tema1::Update(float deltaTimeSeconds)
{

    angle += 0.5f * deltaTimeSeconds;

    auto camera = GetSceneCamera();

    // Draw the scene in Framebuffer
    if (framebuffer_object)
    {
        glm::mat4 cubeView[6] = {
                glm::lookAt(mirrorPosition, glm::vec3(1.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, -1.0f, 0.0f)), // +X
                glm::lookAt(mirrorPosition, glm::vec3(-1.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, -1.0f, 0.0f)), // -X
                glm::lookAt(mirrorPosition, glm::vec3(0.0f, 1.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f)), // +Y
                glm::lookAt(mirrorPosition, glm::vec3(0.0f, -1.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, -1.0f)), // -Y
                glm::lookAt(mirrorPosition, glm::vec3(0.0f, 0.0f, 1.0f),
                            glm::vec3(0.0f, -1.0f, 0.0f)), // +Z
                glm::lookAt(mirrorPosition, glm::vec3(0.0f, 0.0f, -1.0f),
                            glm::vec3(0.0f, -1.0f, 0.0f)), // -Z
            };
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
        // Set the clear color for the color buffer
        glClearColor(0, 0, 0, 1);
        // Clears the color buffer (using the previously set color) and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, 1024, 1024);

        Shader *shader = shaders["Framebuffer"];
        if(ifCountour)
        {
            shader = shaders["outline"];
        }
        shader->Use();

        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);

        {
            glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                               glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                               glm::value_ptr(camera->GetViewMatrix()));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                               glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            glUniform1i(glGetUniformLocation(shader->program, "texture_cubemap"), 1);

            glUniform1i(glGetUniformLocation(shader->program, "cube_draw"), 1);

            meshes["cube"]->Render();
        }

        for (int i = 0; i < 5; i++)
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix *= glm::rotate(glm::mat4(1), angle + i * glm::radians(360.0f) / 5,
                                       glm::vec3(0, 1, 0));
            modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(3, -1, 0));
            modelMatrix *=
                glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0));
            modelMatrix *= glm::scale(glm::mat4(1), glm::vec3(0.01f));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                               glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                               glm::value_ptr(camera->GetViewMatrix()));

            if(ifCountour)
            {
                int loc_instances = shader->GetUniformLocation("viewVector");
                glm::vec3 viewVector = GetSceneCamera()->m_transform->GetLocalOZVector();
                glUniform3fv(loc_instances, 1, glm::value_ptr(viewVector));
            }
            glUniformMatrix4fv(
                glGetUniformLocation(shader->GetProgramID(), "viewMatrices"), 6, GL_FALSE,
                glm::value_ptr(cubeView[0]));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                               glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture(
                                             "Akai_E_Espiritu.fbm\\akai_diffuse.png")
                                             ->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

            glUniform1i(glGetUniformLocation(shader->program, "cube_draw"), 0);

            meshes["archer"]->Render();
        }

        // PARTICLES
        if(ifParticle && !ifCountour)
        {
            auto shader = shaders["particles"];
            shader->Use();
            glLineWidth(3);

            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            TextureManager::GetTexture("particle2.png")
                ->BindToTextureUnit(GL_TEXTURE0);
            // TODO(student): Send uniforms generator_position,
            // deltaTime and offset to the shader
            particleEffect->Render(GetSceneCamera(), shader);

            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                               glm::value_ptr(projection));
            GLint position = glGetUniformLocation(shader->program, "deltaTime");
            glUniform1f(position, deltaTimeSeconds);

            position = glGetUniformLocation(shader->program, "offset");
            glUniform1f(position, offset);

            glUniformMatrix4fv(
                glGetUniformLocation(shader->GetProgramID(), "viewMatrices"), 6, GL_FALSE,
                glm::value_ptr(cubeView[0]));


            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);

        }




        glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        // reset drawing to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, window->GetResolution().x, window->GetResolution().y);

    // Draw the cubemap
    {
        Shader *shader = shaders["ShaderNormal"];
        shader->Use();

        glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                           glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                           glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                           glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
        int loc_texture = shader->GetUniformLocation("texture_cubemap");
        glUniform1i(loc_texture, 0);

        meshes["cube"]->Render();
    }


    // Draw five archers around the mesh
    for (int i = 0; i < 5; i++)
    {
        Shader *shader = shaders["Simple"];
        shader->Use();

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= glm::rotate(glm::mat4(1), angle + i * glm::radians(360.0f) / 5,
                                   glm::vec3(0, 1, 0));
        modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(3, -1, 0));
        modelMatrix *=
            glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0));
        modelMatrix *= glm::scale(glm::mat4(1), glm::vec3(0.01f));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                           glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                           glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                           glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,
                      TextureManager::GetTexture("Akai_E_Espiritu.fbm\\akai_diffuse.png")
                          ->GetTextureID());
        glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

        meshes["archer"]->Render();
    }

    // QUAD
    {
        Shader *shader = shaders["CubeMap"];
        shader->Use();

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, mirrorPosition);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f));

        modelMatrix = glm::rotate(modelMatrix, mirrorRotationOY, glm::vec3(0,1,0));
        modelMatrix = glm::rotate(modelMatrix, mirrorRotationOX, glm::vec3(1,0,0));
        modelMatrix = glm::rotate(modelMatrix, mirrorRotationOZ, glm::vec3(0,0,1));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE,
                           glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE,
                           glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE,
                           glm::value_ptr(camera->GetProjectionMatrix()));

        auto cameraPosition = camera->m_transform->GetWorldPosition();

        if (!color_texture)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            int loc_texture = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture, 0);
        }

        if (color_texture)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
            int loc_texture2 = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture2, 1);
        }

        int loc_camera = shader->GetUniformLocation("camera_position");
        glUniform3f(loc_camera, cameraPosition.x, cameraPosition.y, cameraPosition.z);

        glUniform1i(shader->GetUniformLocation("type"), type);


        meshes["quad"]->Render();
    }

}

void Tema1::FrameEnd()
{
    // DrawCoordinateSystem();
}

/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input with window->KeyHold()

    // TODO(student): Add events for modifying the shrinking parameter


    if (window->KeyHold(GLFW_KEY_U))
    {
        mirrorPosition.z -= 1.25 * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_J))
    {
        mirrorPosition.z += 1.25 * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_H))
    {
        mirrorPosition.x -= 1.25 * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_K))
    {
        mirrorPosition.x += 1.25 * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_I))
    {
        mirrorPosition.y -= 1.25 * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_O))
    {
        mirrorPosition.y += 1.25 * deltaTime;
    }

    if (window->KeyHold(GLFW_KEY_1))
    {
        mirrorRotationOY += glm::radians(10 * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_2))
    {
        mirrorRotationOY -= glm::radians(10 * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_3))
    {
        mirrorRotationOX += glm::radians(10 * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_4))
    {
        mirrorRotationOX -= glm::radians(10 * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_5))
    {
        mirrorRotationOZ += glm::radians(10 * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_6))
    {
        mirrorRotationOZ -= glm::radians(10 * deltaTime);
    }


}

void Tema1::OnKeyPress(int key, int mods)
{
    // Add key press event

    if (key == GLFW_KEY_M)
    {
        ifParticle = ifParticle ^ 1;
    }

    if (key == GLFW_KEY_N)
    {
        ifCountour = ifCountour ^ 1;
    }

}

void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}

void Tema1::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
