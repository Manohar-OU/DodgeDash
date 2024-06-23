#pragma once

#include "GLView.h"
#include "chrono"
#include "Prompts.h"
#include "Player.h"
#include "irrKlang.h"
#include <WOFTGLString.h>

namespace Aftr
{
    class Camera;

    /**
       \class 
       \author Manohar D
       \brief A child of an abstract GLView. This class is the top-most manager of the module.

       Read \see GLView for important constructor and init information.

       \see GLView
    */

    class GLViewDodgeDash : public GLView
    {
    public:
        static GLViewDodgeDash* New(const std::vector<std::string>& outArgs/*, physx::PxPhysics* pxPhysics, physx::PxScene* pxScene*/);
        virtual ~GLViewDodgeDash();
        virtual void updateWorld(); ///< Called once per frame
        virtual void loadMap();     ///< Called once at startup to build this module's scene
        virtual void createNetworkingWayPoints();
        virtual void onResizeWindow(GLsizei width, GLsizei height);
        virtual void onMouseDown(const SDL_MouseButtonEvent& e);
        virtual void onMouseUp(const SDL_MouseButtonEvent& e);
        virtual void onMouseMove(const SDL_MouseMotionEvent& e);
        virtual void onMouseWheelScroll(const SDL_MouseWheelEvent& e);
        virtual void onKeyDown(const SDL_KeyboardEvent& key);
        virtual void onKeyUp(const SDL_KeyboardEvent& key);
        virtual void updateActiveKeys(SDL_KeyCode keycode, bool state);

        virtual void onStartGame();
        virtual void movePlayerForward();
        virtual void updatePlayerProjection();
        virtual void movePlayerLeft();
        virtual void movePlayerRight();
        virtual void controlPlayerToStayOnGrassPlane();
        std::vector<float> generateRandomValue(float posZ, std::optional<Vector> existingPosition = std::nullopt);
        virtual void generateObstacle(int index);
        virtual void generateEnergy(int index);
        virtual void moveObstaclesTowardsPlayer(float updatePosition);
        virtual void pickUpEnergy(int timeHidden);
        virtual void respawnEnergy(int currentTime);
        virtual void displayGameOverText();
        virtual void onGameOver();

    protected:
        GLViewDodgeDash(const std::vector<std::string>& args);
        virtual void onCreate();

        int energyIndex = NULL, obstacleIndex = NULL;
        bool startGame = NULL;
        bool gameOver = NULL;
        float updatePosition = NULL;
        float updateTime = NULL;
        long long timeInSeconds = NULL;
        long long score = NULL;
        float energyLevel = 100.0f;
        std::vector<long long> list_of_scores;
        long long previousBestScore = NULL;
        long long currentScore = NULL;
        bool isHighScore = NULL;

        std::map<SDL_KeyCode, bool> active_keys;
        
        Aftr::Vector prev_pos;
        std::vector<std::pair<int, int>> usedPositions;

        std::string player_location;
        std::vector<std::string> obstacle_location;
        std::vector<Vector> obstacle_original_position;
        std::string energy_location;

        Player* player;
        std::vector<std::pair<WO*, irrklang::ISound*>> obstacles;

        struct EnergyInfo {
            WO* wo_energy = nullptr;
            int hiddenTime = 0;
        };
        std::vector<EnergyInfo> Energy_control;

        std::chrono::high_resolution_clock::time_point tcpRetry;
        std::chrono::high_resolution_clock::time_point timeToUpdateObstacle = std::chrono::high_resolution_clock::now();
        std::chrono::high_resolution_clock::time_point currentTime;

        irrklang::ISoundEngine* soundEngine;

        WOFTGLString* gameOverText = nullptr;
    };

}
