#pragma once
#include <WO.h>
#include <string>
#include <memory>

#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this

class Player : public Aftr::WO
{
public:
    virtual ~Player();

    static Player* New(const std::string& modelFileName, Aftr::Vector scale = Aftr::Vector(1, 1, 1), Aftr::MESH_SHADING_TYPE shadingType = Aftr::MESH_SHADING_TYPE::mstAUTO);

    float degToRad(float deg);
    Aftr::Vector* getPos();
    void setPos(Aftr::Vector pos);
    Aftr::Vector* getRelativeRotation();
    Aftr::Vector* getGlobalRotation();

    virtual void onUpdateWO() override;

protected:
    Aftr::Vector p = Aftr::Vector(0, 0, 4);
    Aftr::Vector* positionInfo = &p;
    Aftr::Vector curr_relativeRotationInfo, prev_relativeRotationInfo;
    Aftr::Vector curr_globalRotationInfo, prev_globalRotationInfo;

    Player(const std::string& modelFileName, Aftr::Vector scale, Aftr::MESH_SHADING_TYPE shadingType);

};