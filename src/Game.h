#include <SFML/Graphics.hpp>

#include "EntityManager.h"
#include "imgui-SFML.h"
#include "imgui.h"

struct WindowConfig {
    int W, H, FPS, fullscreen;
};
struct FontConfig {
    std::string path;
    int SZ, R, G, B;
};
struct PlayerConfig {
    int SR, CR, S, FR, FG, FB, OR, OG, OB, OT, V;
};
struct EnemyConfig {
    int SR, CR, S_MIN, S_MAX, OR, OG, OB, OT, V_MIN, V_MAX, L, R;
};
struct BulletConfig {
    int SR, CR, S, FR, FG, FB, OR, OG, OB, OT, V, L;
};

class Game {
    sf::RenderWindow m_window;
    EntityManager m_manager;
    sf::Font m_font;
    sf::Text m_text;
    sf::Clock m_deltaClock;
    int m_score = 0;
    int m_currentFrame = 0;
    bool m_paused = false;
    bool m_running = true;
    bool m_movementSystem = true;
    bool m_collisionSystem = true;
    bool m_enemySpawnerSystem = true;
    bool m_lifespanSystem = true;

    WindowConfig m_windowConfig;
    FontConfig m_fontConfig;
    PlayerConfig m_playerConfig;
    EnemyConfig m_enemyConfig;
    BulletConfig m_bulletConfig;

    Vec2 m_input = {0, 0};

    int m_delayNormalWeapon = 40;
    int m_lastNormalShoot = -m_delayNormalWeapon;

   public:
    Game(const std::string config);
    bool init(const std::string path);
    void run();
    void sMovement();
    void sRender();
    void sCollision();
    void sEnemySpawner();
    void sLifespan();
    void sUserInput();
    void sScore();
    void sGUI();
    void sPlayerSpawner();
    void spawnWeapon();
    void spawnSpecialWeapon();

    void processInput();
    void enemyDeadEffect(const std::shared_ptr<Entity>& enemy);
};
