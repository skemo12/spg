#pragma once

#include "components/simple_scene.h"
#include "core/gpu/particle_effect.h"

#include "components/transform.h"
#include <string>

namespace m2
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        Tema1();
        ~Tema1();

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

    protected:
        glm::mat4 modelMatrix;
        glm::vec3 generator_position;
        GLenum polygonMode;
        int scene;
        float offset;

        unsigned int nrInstances;
        unsigned int maxInstances;
        int cubeMapTextureID;
        float angle;
        unsigned int framebuffer_object;
        float shrink;
        unsigned int color_texture;
        unsigned int depth_texture;
        unsigned int type;
        glm::vec3 mirrorPosition;
        float mirrorRotationOX;
        float mirrorRotationOY;
        float mirrorRotationOZ;
        unsigned int ifParticle;
        unsigned int ifCountour;
        void InitParticles(int xSize, int ySize, int zSize);
        unsigned int UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string &neg_x, const std::string &neg_y, const std::string &neg_z);
        void CreateFramebuffer(int width, int height);

    };

    struct Particle
    {
        glm::vec4 position;
        glm::vec4 speed;
        glm::vec4 initialPos;
        glm::vec4 initialSpeed;
        float delay;
        float initialDelay;
        float lifetime;
        float initialLifetime;

        Particle() {}

        Particle(const glm::vec4 &pos, const glm::vec4 &speed) { SetInitial(pos, speed); }

        void SetInitial(const glm::vec4 &pos, const glm::vec4 &speed, float delay = 0,
                        float lifetime = 0)
        {
            position = pos;
            initialPos = pos;

            this->speed = speed;
            initialSpeed = speed;

            this->delay = delay;
            initialDelay = delay;

            this->lifetime = lifetime;
            initialLifetime = lifetime;
        }
    };
} // namespace m2
