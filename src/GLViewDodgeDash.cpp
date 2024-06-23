#include "GLViewDodgeDash.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "WOImGui.h" //GUI Demos also need to #include "AftrImGuiIncludes.h"
#include "AftrImGuiIncludes.h"
#include "AftrGLRendererBase.h"
#include <random>
#include <MGLFTGLString.h>
#include <FTGLString.h>

using namespace Aftr;
using namespace std::chrono;

// Define a global random number generator and seed it once
std::mt19937 gen(std::random_device{}());

GLViewDodgeDash* GLViewDodgeDash::New(const std::vector< std::string >& args)
{
    GLViewDodgeDash* glv = new GLViewDodgeDash(args);
    glv->init(Aftr::GRAVITY, Vector(0, 0, -1.0f), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE);
    glv->onCreate();
    return glv;
}

GLViewDodgeDash::GLViewDodgeDash(const std::vector< std::string >& args) : GLView(args)
{
    //Initialize any member variables that need to be used inside of LoadMap() here.
    //Note: At this point, the Managers are not yet initialized. The Engine initialization
    //occurs immediately after this method returns (see GLViewDodgeDash::New() for
    //reference). Then the engine invoke's GLView::loadMap() for this module.
    //After loadMap() returns, GLView::onCreate is finally invoked.

    //The order of execution of a module startup:
    //GLView::New() is invoked:
    //    calls GLView::init()
    //       calls GLView::loadMap() (as well as initializing the engine's Managers)
    //    calls GLView::onCreate()

    //GLViewDodgeDash::onCreate() is invoked after this module's LoadMap() is completed.
}

void GLViewDodgeDash::onCreate()
{
    //GLViewDodgeDash::onCreate() is invoked after this module's LoadMap() is completed.
    //At this point, all the managers are initialized. That is, the engine is fully initialized.

    if (this->pe != NULL)
    {
        //optionally, change gravity direction and magnitude here
        //The user could load these values from the module's aftr.conf
        this->pe->setGravityNormalizedVector(Vector(0, 0, -1.0f));
        this->pe->setGravityScalar(Aftr::GRAVITY);
    }
    this->setActorChaseType(STANDARDEZNAV); //Default is STANDARDEZNAV mode
}

GLViewDodgeDash::~GLViewDodgeDash()
{
    //Implicitly calls GLView::~GLView()
    //GLView::~GLView();
}

void GLViewDodgeDash::updateWorld()
{
    GLView::updateWorld(); //Just call the parent's update world first.
    //If you want to add additional functionality, do it after this call.

    // Set listener position
    Vector playerPos = this->getCamera()->getPosition() + this->getCamera()->getLookDirection() * 10;
    irrklang::vec3df listenerPos(playerPos.x, playerPos.y, playerPos.z);
    soundEngine->setListenerPosition(listenerPos, irrklang::vec3df(0, 0, 2));

    // Update sound positions and volumes
    for (auto& obstacle : obstacles) {
        Vector obstaclePos = obstacle.first->getPosition();
        irrklang::vec3df soundPos(obstaclePos.x, obstaclePos.y, obstaclePos.z);
        obstacle.second->setPosition(soundPos);

        // Calculate distance to player
        float distance = (obstaclePos - playerPos).magnitude();

        // Adjust volume based on distance
        if (distance > 10.0f) {
            obstacle.second->setVolume(0.0f);
        }
        else {
            float volume = 1.0f - (distance / 10.0f);
            obstacle.second->setVolume(volume);
        }
    }

    movePlayerForward();
    updatePlayerProjection();

    if (startGame) {

        controlPlayerToStayOnGrassPlane();

        auto timeNow = std::chrono::high_resolution_clock::now();

        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - currentTime);
        score = elapsedTime.count();
        timeInSeconds = score / 1000;

        pickUpEnergy(timeInSeconds);
        respawnEnergy(timeInSeconds);

        if (timeInSeconds > 60) {
            updatePosition = 1.5f;
            updateTime = 0.10f;
        }
        else if (timeInSeconds > 30) {
            updatePosition = 1.0f;
            updateTime = 0.30f;
        }

        duration<double> timeElapsedforMovingObstacles = duration_cast<duration<double>>(timeNow - timeToUpdateObstacle);
        if (timeElapsedforMovingObstacles.count() >= updateTime)
        {
            moveObstaclesTowardsPlayer(updatePosition);
            timeToUpdateObstacle = timeNow;
        }
    }
}

void GLViewDodgeDash::onResizeWindow(GLsizei width, GLsizei height)
{
    GLView::onResizeWindow(width, height); //call parent's resize method.
}

void GLViewDodgeDash::onMouseDown(const SDL_MouseButtonEvent& e)
{
        //GLView::onMouseDown(e);
}

void GLViewDodgeDash::onMouseUp(const SDL_MouseButtonEvent& e)
{
        //GLView::onMouseUp(e);
}

void GLViewDodgeDash::onMouseMove(const SDL_MouseMotionEvent& e)
{
        //GLView::onMouseMove(e);
}

void GLViewDodgeDash::onMouseWheelScroll(const SDL_MouseWheelEvent& e)
{
        /*GLView::onMouseWheelScroll(e);

        auto cvel = this->getCamera()->getCameraVelocity();
        if (e.y > 0) {
            this->getCamera()->setCameraVelocity(cvel + 0.25f);
        }
        else if (e.y < 0 && cvel > 0.5) {
            this->getCamera()->setCameraVelocity(cvel - 0.25f);
        }*/
}

void GLViewDodgeDash::onKeyDown(const SDL_KeyboardEvent& key)
{
    GLView::onKeyDown(key);
    if (key.keysym.sym == SDLK_0)
        this->setNumPhysicsStepsPerRender(1);

    if (key.keysym.sym == SDLK_UP)
    {
        if (!gameOver) {
            active_keys[SDLK_UP] = true;
        }
    }

    if (key.keysym.sym == SDLK_SPACE)
    {
        if (!startGame) {
            onStartGame();
        }
    }

    if (key.keysym.sym == SDLK_RIGHT)
    {
        movePlayerRight();
    }

    if (key.keysym.sym == SDLK_LEFT)
    {
        movePlayerLeft();
    }
}

void GLViewDodgeDash::onKeyUp(const SDL_KeyboardEvent& key)
{
    GLView::onKeyUp(key);

    if (key.keysym.sym == SDLK_UP)
    {
        if (!startGame && !gameOver) {
            active_keys[SDLK_UP] = false;
        }
    }
}

void GLViewDodgeDash::updateActiveKeys(SDL_KeyCode keycode, bool state)
{
    active_keys[keycode] = state;
}

void Aftr::GLViewDodgeDash::loadMap()
{
    this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
    this->actorLst = new WorldList();
    this->netLst = new WorldList();

    ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
    ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
    ManagerOpenGLState::enableFrustumCulling = false;
    Axes::isVisible = false;
    this->glRenderer->isUsingShadowMapping(false); //set to TRUE to enable shadow mapping, must be using GL 3.2+

    this->cam->setPosition(0, 0, 4);

    soundEngine = irrklang::createIrrKlangDevice();

    irrklang::ISoundSource* danger = soundEngine->addSoundSourceFromFile((ManagerEnvironmentConfiguration::getLMM() + "/sounds/dangerApproaching.mp3").c_str());
    auto dangerAlias = soundEngine->addSoundSourceAlias(danger, "Danger");
    dangerAlias->setDefaultMinDistance(10.0f);
    
    // Set the volume for 2D sounds to 50%
    soundEngine->setSoundVolume(0.40f);

    irrklang::ISoundSource* energy = soundEngine->addSoundSourceFromFile((ManagerEnvironmentConfiguration::getLMM() + "/sounds/energy.mp3").c_str());
    auto energyAlias = soundEngine->addSoundSourceAlias(energy, "Energy");
    energyAlias->setDefaultMinDistance(10.0f);

    irrklang::ISoundSource* collision = soundEngine->addSoundSourceFromFile((ManagerEnvironmentConfiguration::getLMM() + "/sounds/collision.mp3").c_str());
    auto collisionAlias = soundEngine->addSoundSourceAlias(collision, "Collision");
    collisionAlias->setDefaultMinDistance(10.0f);

    irrklang::ISoundSource* gameover = soundEngine->addSoundSourceFromFile((ManagerEnvironmentConfiguration::getLMM() + "/sounds/gameover.mp3").c_str());
    auto gameoverAlias = soundEngine->addSoundSourceAlias(gameover, "Gameover");
    gameoverAlias->setDefaultMinDistance(10.0f);

    std::string grass(ManagerEnvironmentConfiguration::getSMM() + "/models/grassFloor400x400_pp.wrl");

    // SkyBox Textures readily available
    std::vector<std::string> skyBoxImageNames; // vector to store texture paths
    skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg");

    obstacle_location.push_back(ManagerEnvironmentConfiguration::getLMM() + "/models/Trex/Trex.obj");
    obstacle_location.push_back(ManagerEnvironmentConfiguration::getLMM() + "/models/Ant/Ant.obj");
    obstacle_location.push_back(ManagerEnvironmentConfiguration::getLMM() + "/models/Spider/Spider.obj");
    obstacle_location.push_back(ManagerEnvironmentConfiguration::getLMM() + "/models/Grasshoper/Grasshoper.obj");
    obstacle_location.push_back(ManagerEnvironmentConfiguration::getLMM() + "/models/Star/Star.obj");
    obstacle_location.push_back(ManagerEnvironmentConfiguration::getLMM() + "/models/Worm/Worm.obj");

    energy_location = ManagerEnvironmentConfiguration::getLMM() + "/models/Cheese/Cheese.obj";

    player_location = ManagerEnvironmentConfiguration::getLMM() + "/models/RedRobot/RedRobot.obj";

    {
        //Insert player
        player = Player::New(player_location, Vector(0.8, 0.8, 0.8), MESH_SHADING_TYPE::mstFLAT);
        player->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        player->setLabel("Player Robot");
        worldLst->push_back(player);
    }

    {
        // Create a light
        float ga = 0.1f; // Global Ambient Light level for this module
        ManagerLight::setGlobalAmbientLight(aftrColor4f(ga, ga, ga, 1.0f));
        WOLight* light = WOLight::New();
        light->isDirectionalLight(true);
        light->setPosition(Vector(0, 0, 100));
        // Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
        // for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
        light->getModel()->setDisplayMatrix(Mat4::rotateIdentityMat({ 0, 1, 0 }, 120.0f * Aftr::DEGtoRAD));
        light->setLabel("Light");
        worldLst->push_back(light);
    }

    {
        // Create the SkyBox
        WO* wo = WOSkyBox::New(skyBoxImageNames.at(0), this->getCameraPtrPtr());
        wo->setPosition(Vector(0, 0, 0));
        wo->setLabel("Sky Box");
        wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        worldLst->push_back(wo);
    }

    {
        //Create the infinite grass plane (the floor)
        WO* wo = WO::New(grass, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
        wo->setPosition(Vector(0, 0, 0));
        wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        wo->upon_async_model_loaded([wo]()
            {
                ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
                grassSkin.getMultiTextureSet().at(0).setTexRepeats(5.0f);
                grassSkin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f));  // Color of object when it is not in any light
                grassSkin.setDiffuse(aftrColor4f(1.0f, 1.0f, 1.0f, 1.0f));  // Diffuse color components (ie, matte shading color of this object)
                grassSkin.setSpecular(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); // Specular color component (ie, how "shiney" it is)
                grassSkin.setSpecularCoefficient(10);                       // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
            });
        wo->setLabel("Grass");
        worldLst->push_back(wo);
    }

    for (int i = 0; i < obstacle_location.size(); i++)
    {
        for (int j = 0; j < 5; j++) { // Adjust to increase the number of obstacles, make sure to check the min distance between obstacles if more obstacles are to be inserted
            generateObstacle(i);
            obstacleIndex += 1;
            if (j < 2) { // Adjust to increase the number of energy points, make sure to check the min distance between energy if more energy are to be inserted
                generateEnergy(energyIndex);
                energyIndex += 1;
            }
        }
    }

    // Make a Dear Im Gui instance via the WOImGui in the engine... This calls
    // the default Dear ImGui demo that shows all the features... To create your own,
    // inherit from WOImGui and override WOImGui::drawImGui_for_this_frame(...) (among any others you need).
    WOImGui* gui = WOImGui::New(nullptr);

    gui->setLabel("My Gui");
    gui->subscribe_drawImGuiWidget(
        [this, gui]() // this is a lambda, the capture clause is in [], the input argument list is in (), and the body is in {}
        {
            Prompts::getPrompts();

            ImVec4 color_cyan = ImVec4(0.000f, 1.000f, 1.000f, 1.000f);
            ImVec4 color_magenta = ImVec4(1.000f, 0.000f, 1.000f, 1.000f);
            ImVec4 color_dark_gray = ImVec4(0.250f, 0.250f, 0.250f, 1.000f);
            ImVec4 color_green = ImVec4(0.000f, 1.000f, 0.000f, 1.000f);
            ImVec4 color_yellow = ImVec4(1.000f, 1.000f, 0.000f, 1.000f);
            ImVec4 color_red = ImVec4(1.000f, 0.000f, 0.000f, 1.000f);

            ImGui::Begin("Controls");

            ImGui::Separator();
            ImGui::TextColored(color_magenta, "Score:");
            ImGui::SameLine();
            ImGui::TextColored(color_cyan, " %d", score);
            ImGui::Separator();

            if (previousBestScore != NULL) {
                ImGui::Separator();
                ImGui::TextColored(color_magenta, "Previous Best Score:");
                ImGui::SameLine();
                ImGui::TextColored(color_cyan, " %d", previousBestScore);
                ImGui::Separator();
            }

            if (isHighScore) {
                ImGui::Spacing();
                // Center text
                float windowWidth = ImGui::GetWindowSize().x;
                float textWidth = ImGui::CalcTextSize("Hurray, New High Score !!").x;
                ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
                ImGui::TextColored(color_magenta, " Hurray, New High Score !!");
                ImGui::Spacing();
            }

            // Energy bar
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Text("Energy:");
            ImVec4 energyColor;

            if (energyLevel >= 70.0f) {
                energyColor = color_green;
            }
            else if (energyLevel >= 30.0f && energyLevel < 70.0f) {
                energyColor = color_yellow;
            }
            else {
                energyColor = color_red;
                if (energyLevel == 0) {
                    onGameOver();
                }
            }

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, energyColor);
            ImGui::Spacing();
            ImGui::ProgressBar(energyLevel / 100.0f, ImVec2(0.0f, 0.0f));
            ImGui::Spacing();
            ImGui::PopStyleColor();
            ImGui::Separator();

            ImGui::Separator();
            ImGui::Spacing();
            ImVec2 button_size = ImVec2{ 150, 0 };
            float width = ImGui::GetWindowSize().x;
            float pos = (width - button_size.x) / 2;
            ImGui::SetCursorPosX(pos);
            ImGui::PushStyleColor(ImGuiCol_Text, color_green);
            ImGui::PushStyleColor(ImGuiCol_Button, color_dark_gray);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_magenta);

            if (!startGame) {
                if (ImGui::Button("Start Game!", button_size)) {
                    onStartGame();
                }
            }

            ImGui::PopStyleColor(3);

            ImGui::End();
        });
    this->worldLst->push_back(gui);

    createNetworkingWayPoints();
}

void GLViewDodgeDash::onStartGame() {
    if (gameOverText) {
        worldLst->eraseViaWOptr(gameOverText);
    }

    if (isHighScore) {
        previousBestScore = currentScore;
        isHighScore = false;
    }

    updatePosition = 0.5f;
    updateTime = 0.5f;
    startGame = true;
    gameOver = false;
    energyLevel = 100;

    this->cam->setPosition(0, 0, 6);
    auto lookDirection = this->getCamera()->getLookDirection().normalizeMe();
    lookDirection.x = 1;
    lookDirection.y = 0;
    lookDirection.z = 0;
    this->cam->setCameraLookDirection(lookDirection);

    for (int i = 0; i < obstacles.size(); i++) {
        Vector obstaclePosition = obstacles[i].first->getPosition();
        if (obstaclePosition != obstacle_original_position[i]) {
            obstacles[i].first->setPosition(obstacle_original_position[i]);
        }
    }


    active_keys[SDLK_UP] = true;
    currentTime = std::chrono::high_resolution_clock::now();
}

void GLViewDodgeDash::updatePlayerProjection()
{
        auto lookDirection = this->getCamera()->getLookDirection().normalizeMe();
        auto pos = player->getPosition();
        pos = (lookDirection * 10) + this->getCamera()->getPosition();
        pos.z -= 5;
        if (pos.z < 4) pos.z = 4;
        player->setPosition(pos);

        auto rotateByInZ = lookDirection.x * 90 - 90;

        if (lookDirection.y > 0)
            rotateByInZ *= -1;

        player->getRelativeRotation()->z = rotateByInZ;
}

void GLViewDodgeDash::movePlayerForward()
{
    if (active_keys[SDLK_UP])
    {
        auto lookDirection = this->getCamera()->getLookDirection();
        lookDirection.z = 0;
        this->getCamera()->moveRelative(lookDirection * this->getCamera()->getCameraVelocity());
    }
}

void GLViewDodgeDash::movePlayerRight() {

    auto lookDirection = this->getCamera()->getLookDirection().normalizeMe();

    if (lookDirection.x == 1) {
        lookDirection.x = 0;
        lookDirection.y = -1;
        lookDirection.z = 0;
    }
    else if (lookDirection.y == -1) {
        lookDirection.x = -1;
        lookDirection.y = 0;
        lookDirection.z = 0;
    }
    else if (lookDirection.y == 1) {
        lookDirection.x = 1;
        lookDirection.y = 0;
        lookDirection.z = 0;
    }
    else if (lookDirection.x == -1) {
        lookDirection.x = 0;
        lookDirection.y = 1;
        lookDirection.z = 0;
    }

    this->cam->setCameraLookDirection(lookDirection);
    auto playerPosition = player->getPosition();

    if (playerPosition.x < -180.0) {
        playerPosition.x = -180.0f;
    }
    else if (playerPosition.x > 180.0) {
        playerPosition.x = 180.0f;
    }

    if (playerPosition.y < -180.0) {
        playerPosition.y = -180.0f;
    }
    else if (playerPosition.y > 180.0) {
        playerPosition.y = 180.0f;
    }

    this->cam->setPosition(playerPosition - lookDirection * 10);
    player->setPosition(playerPosition);
}

void GLViewDodgeDash::movePlayerLeft() {

    auto lookDirection = this->getCamera()->getLookDirection().normalizeMe();

    if (lookDirection.x == 1) {
        lookDirection.x = 0;
        lookDirection.y = 1;
        lookDirection.z = 0;
    }
    else if (lookDirection.y == -1) {
        lookDirection.x = 1;
        lookDirection.y = 0;
        lookDirection.z = 0;
    }
    else if (lookDirection.y == 1) {
        lookDirection.x = -1;
        lookDirection.y = 0;
        lookDirection.z = 0;
    }
    else if (lookDirection.x == -1) {
        lookDirection.x = 0;
        lookDirection.y = -1;
        lookDirection.z = 0;
    }

    this->cam->setCameraLookDirection(lookDirection);
    auto playerPosition = player->getPosition();

    if (playerPosition.x < -180.0) {
        playerPosition.x = -180.0f;
    }
    else if (playerPosition.x > 180.0) {
        playerPosition.x = 180.0f;
    }

    if (playerPosition.y < -180.0) {
        playerPosition.y = -180.0f;
    }
    else if (playerPosition.y > 180.0) {
        playerPosition.y = 180.0f;
    }

    this->cam->setPosition(playerPosition - lookDirection * 10);
    player->setPosition(playerPosition);
}

void GLViewDodgeDash::controlPlayerToStayOnGrassPlane() {
    Vector playerPosition = this->player->getPosition();
    auto lookDirection = this->cam->getLookDirection().normalizeMe();

    if (playerPosition.x >= 180 && playerPosition.y >= 180) {
        this->cam->setPosition(playerPosition - lookDirection * 10);
        if (lookDirection.x == 1) {
            movePlayerRight();
        }
        else if (lookDirection.y == 1) {
            movePlayerLeft();
        }
    }
    else if (playerPosition.x >= 180 && playerPosition.y <= -180) {
        this->cam->setPosition(playerPosition - lookDirection * 10);
        if (lookDirection.x == 1) {
            movePlayerLeft();
        }
        else if (lookDirection.y == -1) {
            movePlayerRight();
        }
    }
    else if (playerPosition.x <=- 180 && playerPosition.y >= 180) {
        this->cam->setPosition(playerPosition - lookDirection * 10);
        if (lookDirection.x == -1) {
            movePlayerLeft();
        }
        else if (lookDirection.y == 1) {
            movePlayerRight();
        }
    }
    else if (playerPosition.x <=- 180 && playerPosition.y <= -180) {
        this->cam->setPosition(playerPosition - lookDirection * 10);
        if (lookDirection.x == -1) {
            movePlayerRight();
        }
        else if (lookDirection.y == -1) {
            movePlayerLeft();
        }
    }
    else if (playerPosition.x > 180 || playerPosition.x < -180 || playerPosition.y > 180 || playerPosition.y < -180) {
        std::uniform_int_distribution<int> dist(0, 1);
        int choice = dist(gen);
        if (choice == 0) {
            movePlayerLeft();
        }
        else {
            movePlayerRight();
        }
    }
}

std::vector<float> Aftr::GLViewDodgeDash::generateRandomValue(float z, std::optional<Vector> existingPosition) {
    std::uniform_int_distribution<int> dist(-3, 3);

    int x, y;
    auto isUsed = [this](int x, int y) {
        for (const auto& pos : usedPositions) {
            if (pos.first == x && pos.second == y) {
                return true;
            }
        }
        return false;
        };

    do {
        x = dist(gen) * 50;
        y = dist(gen) * 50;
    } while (isUsed(x, y) || (x == 0 && y == 0));

    // Check if the existingPosition was provided
    if (existingPosition.has_value()) {
        auto existingX = existingPosition->x;
        auto existingY = existingPosition->y;

        for (auto it = usedPositions.begin(); it != usedPositions.end(); ++it) {
            if (it->first == static_cast<int>(existingX) && it->second == static_cast<int>(existingY)) {
                *it = { x, y };
                return { static_cast<float>(x), static_cast<float>(y), z };
            }
        }
    }
    else {
        // If no existing position was found or provided, add the new position to usedPositions
        usedPositions.emplace_back(x, y);
        return { static_cast<float>(x), static_cast<float>(y), z };
    }
}

void GLViewDodgeDash::generateObstacle(int index) {

    float randomZ = 3.0f;
    Vector obstaclePosition = generateRandomValue(randomZ);

    WO* obstacle = WO::New(obstacle_location[index], Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);

    obstacle->setPosition(obstaclePosition);
    obstacle->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    obstacle->setLabel("Obstacle " + std::to_string(obstacleIndex + 1));
    obstacle_original_position.push_back(obstacle->getPosition());
    worldLst->push_back(obstacle);

    irrklang::ISound* sound = soundEngine->play3D("Danger",
        irrklang::vec3df(obstaclePosition.at(0), obstaclePosition.at(1), obstaclePosition.at(2)),
        true, false, true);

    obstacles.push_back(std::make_pair(obstacle, sound));
}

void GLViewDodgeDash::generateEnergy(int index) {

    float randomZ = 2.0f;
    Vector energyPosition = generateRandomValue(randomZ);

    WO* energy = WO::New(energy_location, Vector(2, 2, 2), MESH_SHADING_TYPE::mstFLAT);

    energy->setPosition(energyPosition);
    energy->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    energy->setLabel("Energy " + std::to_string(index + 1));
    worldLst->push_back(energy);

    EnergyInfo newEnergyInfo;
    newEnergyInfo.wo_energy = energy;
    Energy_control.push_back(newEnergyInfo);
}

void GLViewDodgeDash::moveObstaclesTowardsPlayer(float updatePosition)
{
    Vector playerPosition = player->getPosition();
    for (int i = 0; i < obstacles.size(); i++)
    {
        Vector obstaclePosition = obstacles[i].first->getPosition();

        float dx = std::abs(playerPosition.x - obstaclePosition.x);
        float dy = std::abs(playerPosition.y - obstaclePosition.y);

        float dx_Original = std::abs(playerPosition.x - obstacle_original_position[i].x);
        float dy_Original = std::abs(playerPosition.y - obstacle_original_position[i].y);

        if (dx <= 4.0f && dy <= 4.0f) {
            if (energyLevel > 10) {
                energyLevel -= 10;
            }
            else
            {
                energyLevel = 0;
            }
            obstaclePosition = obstacle_original_position[i];
            soundEngine->play2D("Collision");
        }
        else if ((dx <= 50.0f && dy <= 50.0f) && (dx_Original <= 50.0f && dy_Original <= 50.0f)){
            if (playerPosition.x - obstaclePosition.x >= 0) { // Obstacle is behind the player
                obstaclePosition.x = obstaclePosition.x + updatePosition;
            }
            else if (playerPosition.x - obstaclePosition.x < 0) { // Obstacle is infront of the player
                obstaclePosition.x = obstaclePosition.x - updatePosition;
            }

            if (playerPosition.y - obstaclePosition.y >= 0) { // Obstacle is right hand side of the player
                obstaclePosition.y = obstaclePosition.y + updatePosition;
            }
            else if (playerPosition.y - obstaclePosition.y < 0) { // Obstacle is left hand side of the player
                obstaclePosition.y = obstaclePosition.y - updatePosition;
            }
        }
        else {
            obstaclePosition = obstacle_original_position[i];
        }
        obstacles[i].first->setPosition(obstaclePosition);
    }
}

void GLViewDodgeDash::pickUpEnergy(int timeHidden) {

    Vector playerPosition = player->getPosition();

    for (int i = 0; i < Energy_control.size(); i++) {

        Vector energyPosition = Energy_control[i].wo_energy->getPosition();

        float dx = std::abs(playerPosition.x - energyPosition.x);
        float dy = std::abs(playerPosition.y - energyPosition.y);

        if (dx <= 2.0 && dy <= 2.0f) {
            auto newEnergyPosition = generateRandomValue(energyPosition.z, energyPosition);
            if (energyLevel < 100)
                energyLevel += 5;
            Energy_control[i].wo_energy->isVisible = false;
            Energy_control[i].hiddenTime = timeHidden;
            Energy_control[i].wo_energy->setPosition(newEnergyPosition);
            soundEngine->play2D("Energy");
        }
    }
}

void GLViewDodgeDash::respawnEnergy(int currentTime) {

    for (int i = 0; i < Energy_control.size(); i++) {
        if (!Energy_control[i].wo_energy->isVisible) {
            auto hiddenDuration = currentTime - Energy_control[i].hiddenTime;
            if (hiddenDuration >= 5) {
                Energy_control[i].wo_energy->isVisible = true;
            }
        }
    }
}

void GLViewDodgeDash::displayGameOverText()
{
    gameOverText = WOFTGLString::New(ManagerEnvironmentConfiguration::getSMM() + "/fonts/DejaVuSansMono.ttf", 30);
    gameOverText->getModelT<MGLFTGLString>()->setFontColor(aftrColor4f(1.0f, 0.0f, 0.0f, 1.0f));
    gameOverText->getModelT<MGLFTGLString>()->setSize(30, 10);
    gameOverText->getModelT<MGLFTGLString>()->setText("Game Over");
    gameOverText->setText("Game   Over");
    gameOverText->rotateAboutGlobalX(-(Aftr::PI / 2));
    gameOverText->rotateAboutGlobalZ(Aftr::PI / 2);
    gameOverText->rotateAboutRelZ(Aftr::PI);

    const Vector textPosition = player->getPosition() + Vector(15, 0, 6);
    gameOverText->setPosition(textPosition);
    worldLst->push_back(gameOverText);
}

void GLViewDodgeDash::onGameOver() {
    energyLevel = 0.1;
    currentScore = score;
    gameOver = true;
    startGame = false;
    soundEngine->play2D("Gameover");
    displayGameOverText();

    auto lookDirection = this->getCamera()->getLookDirection().normalizeMe();
    lookDirection.x = 1;
    lookDirection.y = 0;
    lookDirection.z = 0;
    this->cam->setPosition(player->getPosition() - lookDirection * 10);
    this->cam->setCameraLookDirection(lookDirection);

    active_keys[SDLK_UP] = false;

    if (currentScore > previousBestScore) {
        isHighScore = true;
    }
}

void GLViewDodgeDash::createNetworkingWayPoints()
{
    // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
    WayPointParametersBase params(this);
    params.frequency = 5000;
    params.useCamera = true;
    params.visible = false;
    WOWayPointSpherical* wayPt = WOWayPointSpherical::New(params, 3);
    wayPt->setPosition(Vector(50, 0, 3));
    worldLst->push_back(wayPt);
}
