#include "Player.h"

Player::~Player()
{
}

Player::Player(const std::string& modelFileName, Aftr::Vector scale, Aftr::MESH_SHADING_TYPE shadingType)
    : IFace(this), WO() {
    this->Aftr::WO::onCreate(modelFileName, scale, shadingType);
}

Player* Player::New(const std::string& modelFileName, Aftr::Vector scale, Aftr::MESH_SHADING_TYPE shadingType)
{
    Player* newPlayer = new Player(modelFileName, scale, shadingType);//stuff you want to happen in descendent classes
    //newPlayer->onCreate();//you can call a parent’s function within
    return newPlayer;
}


float Player::degToRad(float deg) {
    return deg * Aftr::PI / 180;
}

Aftr::Vector* Player::getRelativeRotation()
{
    return &curr_relativeRotationInfo;
}

Aftr::Vector* Player::getGlobalRotation()
{
    return &curr_globalRotationInfo;
}

Aftr::Vector* Player::getPos()
{
    return positionInfo;
}

void Player::setPos(Aftr::Vector pos)
{
    *positionInfo = pos;
}

void Player::onUpdateWO()
{
    WO::onUpdateWO();

    // translation
    this->setPosition(*positionInfo);

    // relative rotations 
    this->rotateAboutRelX(degToRad(curr_relativeRotationInfo.x - prev_relativeRotationInfo.x));
    this->rotateAboutRelY(degToRad(curr_relativeRotationInfo.y - prev_relativeRotationInfo.y));
    this->rotateAboutRelZ(degToRad(curr_relativeRotationInfo.z - prev_relativeRotationInfo.z));

    prev_relativeRotationInfo = curr_relativeRotationInfo;

    // global rotations
    this->rotateAboutGlobalX(degToRad(curr_globalRotationInfo.x - prev_globalRotationInfo.x));
    this->rotateAboutGlobalY(degToRad(curr_globalRotationInfo.y - prev_globalRotationInfo.y));
    this->rotateAboutGlobalZ(degToRad(curr_globalRotationInfo.z - prev_globalRotationInfo.z));

    prev_globalRotationInfo = curr_globalRotationInfo;

}
