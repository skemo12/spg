#include "lab_m2/Tema2/tema2.h"

#include <iostream>
#include <vector>

#include "pfd/portable-file-dialogs.h"
#include <chrono>

using namespace std;
using namespace m2;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

Tema2::Tema2()
{
    outputMode = 0;
    gpuProcessing = false;
    saveScreenToImage = false;
    window->SetSize(600, 600);
}

Tema2::~Tema2() {}

void Tema2::Init()
{
    // Load default texture fore imagine processing
    originalImage = TextureManager::LoadTexture(
        PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "tema2", "star.png"),
        nullptr, "image", true, true);
    processedImage = TextureManager::LoadTexture(
        PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "tema2", "star.png"),
        nullptr, "newImage", true, true);
    waterMark = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir,
                                                      RESOURCE_PATH::TEXTURES, "tema2",
                                                      "watermark.png"),
                                            nullptr, "waterMark", true, true);
    processedWaterMark = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir,
                                                               RESOURCE_PATH::TEXTURES,
                                                               "tema2", "watermark.png"),
                                                     nullptr, "newWaterMark", true, true);

    copyImage = TextureManager::LoadTexture(
        PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "tema2", "star.png"),
        nullptr, "copyStar", true, true);

    copyWaterMark = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir,
                                                          RESOURCE_PATH::TEXTURES,
                                                          "tema2", "watermark.png"),
                                                nullptr, "copyWatermark", true, true);
    {
        Mesh *mesh = new Mesh("quad");
        mesh->LoadMesh(
            PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"),
            "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    std::string shaderPath =
        PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema2", "shaders");

    // Create a shader program for particle system
    {
        Shader *shader = new Shader("ImageProcessing");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentShader.glsl"),
                          GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}

void Tema2::FrameStart() {}

void Tema2::Update(float deltaTimeSeconds)
{
    ClearScreen();

    auto shader = shaders["ImageProcessing"];
    shader->Use();

    if (saveScreenToImage)
    {
        window->SetSize(originalImage->GetWidth(), originalImage->GetHeight());
    }

    int flip_loc = shader->GetUniformLocation("flipVertical");
    glUniform1i(flip_loc, saveScreenToImage ? 0 : 1);

    int screenSize_loc = shader->GetUniformLocation("screenSize");
    glm::ivec2 resolution = window->GetResolution();
    glUniform2i(screenSize_loc, resolution.x, resolution.y);

    int outputMode_loc = shader->GetUniformLocation("outputMode");
    glUniform1i(outputMode_loc, outputMode);

    int locTexture = shader->GetUniformLocation("textureImage");
    glUniform1i(locTexture, 0);

    auto textureImage = (gpuProcessing == true) ? originalImage : processedImage;
    textureImage->BindToTextureUnit(GL_TEXTURE0);

    RenderMesh(meshes["quad"], shader, glm::mat4(1));

    if (saveScreenToImage)
    {
        saveScreenToImage = false;

        GLenum format = GL_RGB;
        if (originalImage->GetNrChannels() == 4)
        {
            format = GL_RGBA;
        }

        glReadPixels(0, 0, originalImage->GetWidth(), originalImage->GetHeight(), format,
                     GL_UNSIGNED_BYTE, processedImage->GetImageData());
        processedImage->UploadNewData(processedImage->GetImageData());
        SaveImage("shader_processing_" + std::to_string(outputMode));

        float aspectRatio =
            static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}

void Tema2::FrameEnd() { DrawCoordinateSystem(); }

void Tema2::OnFileSelected(const std::string &fileName)
{
    if (fileName.size())
    {
        std::cout << fileName << endl;
        originalImage =
            TextureManager::LoadTexture(fileName, nullptr, "image", true, true);
        processedImage =
            TextureManager::LoadTexture(fileName, nullptr, "newImage", true, true);
        copyImage =
            TextureManager::LoadTexture(fileName, nullptr, "copyStar", true, true);
        float aspectRatio =
            static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}

void Tema2::GrayScale(Texture2D *oImage, Texture2D *pImage)
{
    unsigned int channels = oImage->GetNrChannels();
    unsigned char *data = oImage->GetImageData();
    unsigned char *newData = pImage->GetImageData();

    if (channels < 3)
    {
        memcpy(newData, data,
               oImage->GetWidth() * oImage->GetHeight() * channels *
                   sizeof(unsigned char));
        return;
    }

    glm::ivec2 imageSize = glm::ivec2(oImage->GetWidth(), oImage->GetHeight());

    for (int i = 0; i < imageSize.y; i++)
    {
        for (int j = 0; j < imageSize.x; j++)
        {
            int offset = channels * (i * imageSize.x + j);

            // Reset save image data
            char value =
                static_cast<char>(data[offset + 0] * 0.2f + data[offset + 1] * 0.71f +
                                  data[offset + 2] * 0.07);
            memset(&newData[offset], value, 3);
        }
    }

    pImage->UploadNewData(newData);
}

void Tema2::Blur(Texture2D *oImage, Texture2D *pImage)
{
    unsigned int channels = oImage->GetNrChannels();
    unsigned char *data = oImage->GetImageData();
    unsigned char *newData = pImage->GetImageData();

    if (channels < 3)
    {
        cout << "Blur implementations requires channel = 3\n";
        memcpy(newData, data,
               oImage->GetWidth() * oImage->GetHeight() * channels *
                   sizeof(unsigned char));
        return;
    }

    glm::ivec2 imageSize = glm::ivec2(oImage->GetWidth(), oImage->GetHeight());
    int radius = 3;
    int samples = radius * radius;

    for (int i = (radius - 1) / 2; i < imageSize.y - radius / 2; i++)
    {
        for (int j = (radius - 1) / 2; j < imageSize.x - radius / 2; j++)
        {
            int r_sum = 0;
            int g_sum = 0;
            int b_sum = 0;
            int offset =
                channels * ((i - (radius - 1) / 2) * imageSize.x + j - (radius - 1) / 2);
            for (int k = 0; k < radius; k++)
            {
                for (int r = 0; r < radius; r++)
                {
                    r_sum += data[offset];
                    g_sum += data[offset + 1];
                    b_sum += data[offset + 2];
                    offset += channels;
                }
                offset -= channels * radius;
                offset += channels * imageSize.x;
            }
            offset = channels * (i * imageSize.x + j);
            newData[offset] = r_sum / samples;
            newData[offset + 1] = g_sum / samples;
            newData[offset + 2] = b_sum / samples;
        }
    }
    pImage->UploadNewData(newData);
}

void Tema2::Sobel(Texture2D *oImage, Texture2D *pImage, Texture2D *copy)
{

    Blur(oImage, copy);

    Blur(oImage, pImage);
    GrayScale(pImage, copy);

    unsigned char *newData = pImage->GetImageData();
    unsigned char *data = copy->GetImageData();

    glm::ivec2 imageSize = glm::ivec2(oImage->GetWidth(), oImage->GetHeight());

    int nrChannels = oImage->GetNrChannels();
    cout << nrChannels << endl;

    for (int i = 1; i < imageSize.y - 1; i++)
    {
        for (int j = 1; j < imageSize.x - 1; j++)
        {
            int a_0_0 = data[nrChannels * ((i - 1) * imageSize.x + j - 1)];
            int a_0_1 = data[nrChannels * ((i - 1) * imageSize.x + j)];
            int a_0_2 = data[nrChannels * ((i - 1) * imageSize.x + j + 1)];
            int a_1_0 = data[nrChannels * (i * imageSize.x + j - 1)];
            int a_1_2 = data[nrChannels * (i * imageSize.x + j + 1)];
            int a_2_0 = data[nrChannels * ((i + 1) * imageSize.x + j - 1)];
            int a_2_1 = data[nrChannels * ((i + 1) * imageSize.x + j)];
            int a_2_2 = data[nrChannels * ((i + 1) * imageSize.x + j + 1)];

            int dx = a_2_2 + 2 * a_1_2 + a_0_2 - a_2_0 - 2 * a_1_0 - a_0_0;
            int dy = a_2_2 + 2 * a_2_1 + a_2_0 - a_0_2 - 2 * a_0_1 - a_0_0;
            int val = abs(dx) + abs(dy);
            if (val < 220)
                val = 0;
            else
                val = 255;
            newData[nrChannels * (i * imageSize.x + j)] = (unsigned char)val;
            newData[nrChannels * (i * imageSize.x + j) + 1] = (unsigned char)val;
            newData[nrChannels * (i * imageSize.x + j) + 2] = (unsigned char)val;
        }
    }

    pImage->UploadNewData(newData);
}

void Tema2::RemoveWatermark(Texture2D *oImage, Texture2D *pImage, Texture2D *copy,
                            Texture2D *oWaterMark, Texture2D *pWaterMark)
{
    cout << "Remove watermark\n";
    Sobel(oImage, pImage, copyImage);
    unsigned char *data = pImage->GetImageData();
    memcpy(copy->GetImageData(), oImage->GetImageData(),
           oImage->GetWidth() * oImage->GetHeight() * oImage->GetNrChannels() *
               sizeof(unsigned char));
    unsigned char *oMark = oWaterMark->GetImageData();
    unsigned char *result = copy->GetImageData();
    Sobel(oWaterMark, pWaterMark, copyWaterMark);
    unsigned char *waterMarkData = pWaterMark->GetImageData();

    glm::ivec2 imageSize = glm::ivec2(oImage->GetWidth(), oImage->GetHeight());

    glm::ivec2 wISize = glm::ivec2(oWaterMark->GetWidth(), oWaterMark->GetHeight());

    int nrChannels = oImage->GetNrChannels();
    int nrChannelsWatermark = oWaterMark->GetNrChannels();
    int y, x, yw, xw, offset, maybe, lineImage, lineWaterMark, offsetWatermark,
        localOffset, line, neg;
    int total = 0;

    for (yw = 0; yw < wISize.y; yw++)
    {
        lineWaterMark = yw * wISize.x * nrChannelsWatermark;
        for (xw = 0; xw < wISize.x; xw++)
        {
            offsetWatermark = nrChannelsWatermark * xw + lineWaterMark;

            if (waterMarkData[offsetWatermark] == 255)
            {
                total += 1;
            }
        }
    }

    total = total / 4;
    cout << "Total:\n";
    cout << total << endl;
    int prag1 = static_cast<int>(total * 0.80);
    int prag2 = static_cast<int>(total * 0.10);

    cout << prag1 << endl;
    cout << prag2 << endl;

    for (y = 0; y < imageSize.y - wISize.y; y++)
    {
        line = y * imageSize.x * nrChannels;
        for (x = 0; x < imageSize.x - wISize.x; x++)
        {
            offset = line + x * nrChannels;
            maybe = 0;
            neg = 0;

            for (yw = 0; yw < wISize.y; yw += 2)
            {
                lineWaterMark = yw * wISize.x * nrChannelsWatermark;
                lineImage = yw * imageSize.x * nrChannels + offset;

                for (xw = 0; xw < wISize.x; xw += 2)
                {
                    localOffset = nrChannels * xw + lineImage;
                    if (data[localOffset] == 0)
                    {
                        continue;
                    }
                    offsetWatermark = nrChannelsWatermark * xw + lineWaterMark;

                    if (data[localOffset] == waterMarkData[offsetWatermark])
                    {
                        maybe += 1;
                    }
                }
            }

            if (maybe >= prag1)
            {
                cout << "match " << maybe << endl;
                for (yw = 0; yw < wISize.y; yw++)
                {
                    lineWaterMark = yw * wISize.x * nrChannelsWatermark;
                    lineImage = yw * imageSize.x * nrChannels + offset;
                    for (xw = 0; xw < wISize.x; xw++)
                    {
                        offsetWatermark = nrChannelsWatermark * xw + lineWaterMark;
                        localOffset = nrChannels * xw + lineImage;

                        result[localOffset] -= oMark[offsetWatermark];
                        result[localOffset + 1] -= oMark[offsetWatermark + 1];
                        result[localOffset + 2] -= oMark[offsetWatermark + 2];
                    }
                }
                x += wISize.x - 2;
            }
            if (maybe < prag2)
            {
                x += wISize.x / 4;
            }
        }
    }

    memcpy(pImage->GetImageData(), copy->GetImageData(),
           copy->GetWidth() * copy->GetHeight() * copy->GetNrChannels() *
               sizeof(unsigned char));
    cout << "End watermark\n";

    unsigned char *newData = copy->GetImageData();
    pImage->UploadNewData(newData);
}

void Tema2::SaveImage(const std::string &fileName)
{
    cout << "Saving image! ";
    processedImage->SaveToFile((fileName + ".png").c_str());
    cout << "[Done]" << endl;
}

void Tema2::OpenDialog()
{
    std::vector<std::string> filters = {"Image Files", "*.png *.jpg *.jpeg *.bmp",
                                        "All Files", "*"};

    auto selection = pfd::open_file("Select a file", ".", filters).result();
    if (!selection.empty())
    {
        std::cout << "User selected file " << selection[0] << "\n";
        OnFileSelected(selection[0]);
    }
}

/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}

void Tema2::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        OpenDialog();
    }

    if (key == GLFW_KEY_E)
    {
        gpuProcessing = !gpuProcessing;
        if (gpuProcessing == false)
        {
            outputMode = 0;
        }
        cout << "Processing on GPU: " << (gpuProcessing ? "true" : "false") << endl;
    }

    if (key - GLFW_KEY_0 >= 0 && key <= GLFW_KEY_5)
    {
        outputMode = key - GLFW_KEY_0;

        if (gpuProcessing == false)
        {
            if (outputMode == 1)
                GrayScale(originalImage, processedImage);
            if (outputMode == 2)
                Blur(originalImage, processedImage);
            if (outputMode == 4)
                Sobel(originalImage, processedImage, copyImage);
            if (outputMode == 5)
            {
                auto start = chrono::high_resolution_clock::now();
                RemoveWatermark(originalImage, processedImage, copyImage, waterMark,
                                processedWaterMark);

                auto stop = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::seconds>(stop - start);
                cout << "Time taken by function: " << duration.count() << " seconds"
                     << endl;
                auto durationms =
                    chrono::duration_cast<chrono::microseconds>(stop - start);

                cout << "Time taken by function: " << durationms.count()
                     << " microseconds" << endl;
            }
            outputMode = 0;
        }
    }

    if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
    {
        if (!gpuProcessing)
        {
            SaveImage("processCPU_" + std::to_string(outputMode));
        }
        else
        {
            saveScreenToImage = true;
        }
    }
}

void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}

void Tema2::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
