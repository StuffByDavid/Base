#include "test.hpp"
#include "resource/shader.hpp"
#include "resource/model.hpp"
#include "resource/testscene.hpp"
#include "input/mousefunc.hpp"


const string testScene = "scenes/checker.testscene";
const string testShader = "shaders/shadows.glsl";

// TODO: move somewhere
Base::Light* sceneLight;
GLuint depthFramebuffer, depthTexture;
Base::Mat4f biasMat, depthVP, depthBiasVP;
Base::Size2Di bufSize = { 2048, 2048 };

void Base::TestApp::testSceneInit()
{
    // Setup camera
    camAngleXZ = camGoalAngleXZ = 90.f;
    camAngleY  = camGoalAngleY  = 20.f;
    camZoom    = camGoalZoom    = 100.f;
    camMove    = false;

    // SETUP SHADOWS!

    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &depthFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer);

    // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, bufSize.width, bufSize.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    Color borderColor(1.f);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (float*)&borderColor);  
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

    glDrawBuffer(GL_NONE); // No color buffer is drawn to.
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ((Shader*)resHandler->get("shaders/shadows.glsl"))->setSetupFunc([&](GLuint glProgram, const Mat4f& matM)
    {
        GLint uDepthSampler = glGetUniformLocation(glProgram, "uDepthSampler");
        GLint uLightDir = glGetUniformLocation(glProgram, "uLightDir");
        GLint uMatDepthBiasMVP = glGetUniformLocation(glProgram, "uMatDepthBiasMVP");

        // Send in depth matrix
        glUniformMatrix4fv(uMatDepthBiasMVP, 1, GL_FALSE, (depthBiasVP * matM).elem);

        // Send in depth
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glUniform1i(uDepthSampler, 1);
        
        glUniform3fv(uLightDir, 1, (float*)&sceneLight->dir);
    });

    biasMat = Mat4f(
        0.5f, 0.0f, 0.0f, 0.5f,
        0.0f, 0.5f, 0.0f, 0.5f,
        0.0f, 0.0f, 0.5f, 0.5f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

void Base::TestApp::testSceneInput()
{
    // Left click rotates
    if (camMove)
    {
        if (mouseLeftDown())
        {
            camGoalAngleXZ += mouseMove().x * 0.25f;
            camGoalAngleY  += mouseMove().y * 0.25f;
            camGoalAngleY   = clamp(camGoalAngleY, -89.f, 89.f);
        }
        else
            camMove = false;
    }
    else if (mouseLeftPressed())
        camMove = true;
    
    // Scroll zooms (sideways scroll will also rotate)
    camGoalAngleXZ += mouseScroll().x * 5.f;
    camGoalZoom    *= 1.f - 0.1f * mouseScroll().y;
    camGoalZoom     = clamp(camGoalZoom, 10.f, 2000.f);

    // Move light
    sceneLight->translate({
        (keyDown(GLFW_KEY_D) - keyDown(GLFW_KEY_A)) * 10.f,
        (keyDown(GLFW_KEY_Q) - keyDown(GLFW_KEY_E)) * 10.f,
        (keyDown(GLFW_KEY_S) - keyDown(GLFW_KEY_W)) * 10.f
    });
}

void Base::TestApp::testSceneRender()
{
    // Load the scene from JSON during start or when modified
    currentScene = (TestScene*)resHandler->get(testScene);
    sceneLight = currentScene->lights[0];

    // Animate some objects
    float d = mainWindow->getFrameDelay();
    ((Model*)resHandler->get("models/teapot/teapot.obj"))->rotateY(d)->buildMatrix();
    ((Model*)resHandler->get("models/box.obj"))->rotateX(d)->rotateY(d)->rotateZ(d)->buildMatrix();

    // Camera rotates in a circle around the scene origin
    currentScene->camera.pos = {
        dcos(camAngleXZ) * dcos(camAngleY) * camZoom,
        dsin(camAngleY) * camZoom,
        dsin(camAngleXZ) * dcos(camAngleY) * camZoom
    };

    // Smoothen the motion
    camAngleXZ += (camGoalAngleXZ - camAngleXZ) / 5.f;
    camAngleY  += (camGoalAngleY  - camAngleY)  / 5.f;
    camZoom    += (camGoalZoom    - camZoom)    / 5.f;
    
    // Render from light
    Mat4f P = Mat4f::ortho(-100.f, 100.f, -100.f, 100.f, -300.f, 300.f);
    Mat4f V = Mat4f::viewLookAt(sceneLight->pos, { 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f });
    depthVP = P * V;
    depthBiasVP = biasMat * depthVP;

    glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer);
    glViewport(0, 0, bufSize.width, bufSize.height);
    currentScene->render((Shader*)resHandler->get("shaders/depth.glsl"), depthVP);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render some goodness
    glViewport(0, 0, mainWindow->size.width, mainWindow->size.height);
    currentScene->render((Shader*)resHandler->get(testShader));
}