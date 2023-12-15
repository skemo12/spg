#pragma once

#include <string>

#include "components/simple_scene.h"
#include "core/gpu/frame_buffer.h"


namespace m2
{
    class Lab8 : public gfxc::SimpleScene
    {
     public:
        Lab8();
        ~Lab8();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void OpenDialog();
        void OnFileSelected(const std::string &fileName);

        // Processing effects
        void GrayScale();
        void BitToBit(int size);
        void Blur(unsigned char *data);
        void GrayScale(unsigned char *data);
        void Median();
        void Sobel();
        void SaveImage(const std::string &fileName);

     private:
        Texture2D *originalImage;
        Texture2D *processedImage;

        int outputMode;
        bool gpuProcessing;
        bool saveScreenToImage;
    };
}   // namespace m2
