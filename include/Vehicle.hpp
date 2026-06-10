#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include "Character.hpp"

class Vehicle : public Character {
public:
    Vehicle(float x, float y, const std::string& texturePath);

    glm::vec2 Update(std::shared_ptr<Map> map) override;

    void SetSpawnPoint(int x, int y) { m_SpawnGridX = x; m_SpawnGridY = y; }
    void SetResetY(int y) { m_ResetGridY = y; }

protected:
    void LoadSprites() override;
    void UpdateSprite() override;

private:
    std::string m_TexturePath;
    int m_SpawnGridX = 0;
    int m_SpawnGridY = 0;
    int m_ResetGridY = -1; 

    float m_SpawnTimer = 0.0f;
    bool m_IsActive = true;
};

#endif
