#include "Game.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>

Game::Game(const std::string config) {
    if (!init(config)) exit(-1);
}

bool Game::init(const std::string path) {
    srand(time(NULL));
    std::ifstream fin(path);
    std::string type;

    while (fin >> type) {
        if (type == "Window")
            fin >> m_windowConfig.W >> m_windowConfig.H >> m_windowConfig.FPS >>
                m_windowConfig.fullscreen;

        else if (type == "Font")
            fin >> m_fontConfig.path >> m_fontConfig.SZ >> m_fontConfig.R >>
                m_fontConfig.G >> m_fontConfig.B;

        else if (type == "Player")
            fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >>
                m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >>
                m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >>
                m_playerConfig.OT >> m_playerConfig.V;

        else if (type == "Enemy")
            fin >> m_enemyConfig.SR >> m_enemyConfig.CR >>
                m_enemyConfig.S_MIN >> m_enemyConfig.S_MAX >>
                m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >>
                m_enemyConfig.OT >> m_enemyConfig.V_MIN >>
                m_enemyConfig.V_MAX >> m_enemyConfig.L >> m_enemyConfig.R;

        else if (type == "Bullet")
            fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >>
                m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >>
                m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >>
                m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
    }

    if (m_windowConfig.fullscreen)
        m_window.create(sf::VideoMode(m_windowConfig.W, m_windowConfig.H),
                        "ECS Geometry War", sf::Style::Fullscreen);
    else
        m_window.create(sf::VideoMode(m_windowConfig.W, m_windowConfig.H),
                        "ECS Geometry War", sf::Style::Default);

    m_window.setFramerateLimit(m_windowConfig.FPS);
    ImGui::SFML::Init(m_window);
    ImGui::GetStyle().ScaleAllSizes(1.0f);

    if (!m_font.loadFromFile(m_fontConfig.path)) {
        std::cerr << "Failed to load font :(\n";
        return false;
    }

    auto e = m_manager.addEntity("player");

    e->cShape = std::make_shared<CShape>(
        m_playerConfig.SR, m_playerConfig.V,
        sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
        sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB),
        m_playerConfig.OT);

    e->cTransform = std::make_shared<CTransform>(Vec2(0, 0), Vec2(0, 0), 0,
                                                 (float)m_playerConfig.S / 10.0,
                                                 m_playerConfig.S);
    e->cInput = std::make_shared<CInput>();
    e->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);
    e->cScore = std::make_shared<CScore>(0);

    return true;
}

void Game::run() {
    while (m_window.isOpen()) {
        m_manager.update();
        ImGui::SFML::Update(m_window, m_deltaClock.restart());
        if (m_currentFrame > 0) sGUI();
        if (m_currentFrame > 0) sUserInput();
        if (m_currentFrame > 0 && m_collisionSystem) sCollision();
        if (m_currentFrame > 0 && m_enemySpawnerSystem && !m_paused)
            sEnemySpawner();
        if (m_currentFrame > 0 && m_movementSystem) sMovement();
        if (m_currentFrame > 0 && m_lifespanSystem) sLifespan();
        if (m_currentFrame > 0) sScore();
        sRender();
        if (m_currentFrame == 0) sPlayerSpawner();
        m_currentFrame++;
    }
}

int randomNumber(int a, int b) { return a + std::rand() % (b - a + 1); }

void Game::sEnemySpawner() {
    if (m_currentFrame % m_enemyConfig.R != 0) return;
    int screenWidth = m_window.getView().getSize().x;
    int screenHeight = m_window.getView().getSize().y;

    float speed = randomNumber(m_enemyConfig.S_MIN, m_enemyConfig.S_MAX);

    Vec2 pos(
        randomNumber(m_enemyConfig.CR + 1, screenWidth - m_enemyConfig.CR - 1),
        randomNumber(m_enemyConfig.CR + 1,
                     screenHeight - m_enemyConfig.CR - 1));

    int vertices = randomNumber(m_enemyConfig.V_MIN, m_enemyConfig.V_MAX);

    Vec2 dir(randomNumber(-screenWidth, screenWidth),
             randomNumber(-screenHeight, screenHeight));
    dir = dir.normalize();

    sf::Color fill(randomNumber(0, 255), randomNumber(0, 255),
                   randomNumber(0, 255));

    auto e = m_manager.addEntity("enemy");

    e->cShape = std::make_shared<CShape>(
        m_enemyConfig.SR, vertices, fill,
        sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
        m_enemyConfig.OT);
    e->cTransform = std::make_shared<CTransform>(pos, dir, 0, 0, speed);
    e->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
}

void Game::sUserInput() {
    if (!m_window.hasFocus()) return;
    sf::Event event;
    while (m_window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(m_window, event);
        if (event.type == sf::Event::Closed)
            m_window.close();
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) m_window.close();
            if (event.key.code == sf::Keyboard::W) {
                for (auto& e : m_manager.getEntities())
                    if (e->cInput) e->cInput->up = true;
            }
            if (event.key.code == sf::Keyboard::S) {
                for (auto& e : m_manager.getEntities())
                    if (e->cInput) e->cInput->down = true;
            }
            if (event.key.code == sf::Keyboard::A) {
                for (auto& e : m_manager.getEntities())
                    if (e->cInput) e->cInput->left = true;
            }
            if (event.key.code == sf::Keyboard::D) {
                for (auto& e : m_manager.getEntities())
                    if (e->cInput) e->cInput->right = true;
            }
        } else if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::W) {
                for (auto& e : m_manager.getEntities())
                    if (e->cInput) e->cInput->up = false;
            }
            if (event.key.code == sf::Keyboard::S) {
                for (auto& e : m_manager.getEntities())
                    if (e->cInput) e->cInput->down = false;
            }
            if (event.key.code == sf::Keyboard::A) {
                for (auto& e : m_manager.getEntities())
                    if (e->cInput) e->cInput->left = false;
            }
            if (event.key.code == sf::Keyboard::D) {
                for (auto& e : m_manager.getEntities())
                    if (e->cInput) e->cInput->right = false;
            }
            if (event.key.code == sf::Keyboard::P) m_paused = !m_paused;
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !m_paused)
            spawnWeapon();
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !m_paused)
            spawnSpecialWeapon();
    }
    processInput();
}

void Game::processInput() {
    for (auto& e : m_manager.getEntities()) {
        if (e->cInput && e->cTransform) {
            if (e->cInput->up)
                m_input.y = -1;
            else if (e->cInput->down)
                m_input.y = 1;
            else
                m_input.y = 0;

            if (e->cInput->left)
                m_input.x = -1;
            else if (e->cInput->right)
                m_input.x = 1;
            else
                m_input.x = 0;
        }
    }
}

void Game::spawnWeapon() {
    if (m_manager.getEntities("player").empty()) return;
    if (m_currentFrame - m_lastNormalShoot < m_delayNormalWeapon) return;
    m_lastNormalShoot = m_currentFrame;

    sf::Mouse::getPosition(m_window).x, sf::Mouse::getPosition(m_window).y;

    Vec2 dir = {sf::Mouse::getPosition(m_window).x,
                sf::Mouse::getPosition(m_window).y};
    Vec2 pos = m_manager.getEntities("player")[0]->cTransform->pos;
    float angle = m_manager.getEntities("player")[0]->cTransform->angle;

    dir -= pos;
    dir = dir.normalize();

    auto e = m_manager.addEntity("bullet");
    e->cTransform =
        std::make_shared<CTransform>(pos, dir, angle, 0, m_bulletConfig.S);
    e->cShape = std::make_shared<CShape>(
        m_bulletConfig.SR, m_bulletConfig.V,
        sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
        sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
        m_bulletConfig.OT);
    e->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
    e->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon() {
    if (m_currentFrame - m_lastSpecialShoot < m_delaySpecialWeapon) return;
    m_lastSpecialShoot = m_delaySpecialWeapon;
    Vec2 pos = {sf::Mouse::getPosition(m_window).x,
                sf::Mouse::getPosition(m_window).y};

    auto e = m_manager.addEntity("specialbullet");
    e->cTransform = std::make_shared<CTransform>(pos, Vec2(0, 0), 0, 0,
                                                 m_bulletConfig.S / 2);
    e->cShape =
        std::make_shared<CShape>(20, 18, sf::Color(0, 0, 0, 0),
                                 sf::Color(148, 0, 211), m_bulletConfig.OT * 2);
    e->cCollision = std::make_shared<CCollision>(20);
    e->cLifespan = std::make_shared<CLifespan>(m_delaySpecialWeapon);
}

void Game::sCollision() {
    for (auto& e : m_manager.getEntities()) {
        if (e->tag() == "specialbullet") continue;
        if (e->cCollision && e->cTransform) {
            Vec2 pos = e->cTransform->pos;
            float speed = e->cTransform->speed;
            float radius = e->cCollision->radius;
            float screenWidth = m_window.getView().getSize().x;
            float screenHeight = m_window.getView().getSize().y;

            if (pos.x - radius <= 0) e->cTransform->velocity.x = speed;
            if (pos.x + radius >= screenWidth)
                e->cTransform->velocity.x = -speed;
            if (pos.y - radius <= 0) e->cTransform->velocity.y = speed;
            if (pos.y + radius >= screenHeight)
                e->cTransform->velocity.y = -speed;
        }
    }

    for (auto& bullet : m_manager.getEntities("bullet")) {
        for (auto& enemy : m_manager.getEntities("enemy")) {
            Vec2 bulletPos = bullet->cTransform->pos;
            Vec2 enemyPos = enemy->cTransform->pos;
            float bulletRadius = bullet->cCollision->radius;
            float enemyRadius = enemy->cCollision->radius;

            if (bulletPos.dist(enemyPos) <= bulletRadius + enemyRadius) {
                enemyDeadEffect(enemy);
                bullet->destroy();
                enemy->destroy();
                if (!m_manager.getEntities("player").empty())
                    m_manager.getEntities("player")[0]->cScore->score += 100;
            }
        }
        for (auto& enemy : m_manager.getEntities("minienemie")) {
            Vec2 bulletPos = bullet->cTransform->pos;
            Vec2 enemyPos = enemy->cTransform->pos;
            float bulletRadius = bullet->cCollision->radius;
            float enemyRadius = enemy->cCollision->radius;

            if (bulletPos.dist(enemyPos) <= bulletRadius + enemyRadius) {
                bullet->destroy();
                enemy->destroy();
                if (!m_manager.getEntities("player").empty())
                    m_manager.getEntities("player")[0]->cScore->score += 200;
            }
        }
    }

    for (auto& enemy : m_manager.getEntities("enemy")) {
        for (auto& player : m_manager.getEntities("player")) {
            Vec2 enemyPos = enemy->cTransform->pos;
            Vec2 playerPos = player->cTransform->pos;
            float playerRadius = player->cCollision->radius;
            float enemyRadius = enemy->cCollision->radius;

            if (enemyPos.dist(playerPos) <= playerRadius + enemyRadius) {
                enemyDeadEffect(enemy);
                enemy->destroy();
                sPlayerSpawner();
            }
        }
    }

    for (auto& b : m_manager.getEntities("specialbullet")) {
        for (auto& e : m_manager.getEntities()) {
            if (e->tag() == "enemy" || e->tag() == "minienemie") {
                Vec2 enemyPos = e->cTransform->pos;
                Vec2 bulletPos = b->cTransform->pos;
                float bulletRadius = b->cCollision->radius;
                float enemyRadius = e->cCollision->radius;
                if (enemyPos.dist(bulletPos) <= bulletRadius + enemyRadius)
                    e->cTransform->speed /= 1.05;
            }
        }
    }
}

void Game::enemyDeadEffect(const std::shared_ptr<Entity>& enemy) {
    int vertices = enemy->cShape->shape.getPointCount();
    float collisionRadius = enemy->cCollision->radius;
    float shapeRadius = enemy->cShape->shape.getRadius();
    float thickness = enemy->cShape->shape.getOutlineThickness();
    sf::Color fill = enemy->cShape->shape.getFillColor();
    sf::Color outline = enemy->cShape->shape.getOutlineColor();
    Vec2 pos = enemy->cTransform->pos;
    float angle = enemy->cTransform->angle;
    float speed = enemy->cTransform->speed;

    float cnt = 360.0 / (float)vertices;

    for (float i = 0; i <= 360.0 + EPS; i += cnt) {
        float theta = i * M_PI / 180.0;
        Vec2 dir(cos(theta), sin(theta));

        auto e = m_manager.addEntity("minienemie");
        e->cCollision = std::make_shared<CCollision>(collisionRadius / 3.0);
        e->cShape = std::make_shared<CShape>(shapeRadius / 3.0, vertices, fill,
                                             outline, thickness);
        e->cTransform = std::make_shared<CTransform>(pos, dir, angle, 0, speed);
        e->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
    }
}

void Game::sMovement() {
    for (auto& e : m_manager.getEntities()) {
        if (e->cTransform) {
            if (!m_paused) {
                float speed = e->cTransform->speed;

                Vec2 move = e->cTransform->velocity;
                if (e->cInput) move += m_input;
                e->cTransform->pos += move.normalize() * speed;

                if (e->cTransform->velocity.x < 0)
                    e->cTransform->velocity.x =
                        std::min((float)0, e->cTransform->velocity.x +
                                               e->cTransform->friction);
                else
                    e->cTransform->velocity.x =
                        std::max((float)0, e->cTransform->velocity.x -
                                               e->cTransform->friction);

                if (e->cTransform->velocity.y < 0)
                    e->cTransform->velocity.y =
                        std::min((float)0, e->cTransform->velocity.y +
                                               e->cTransform->friction);
                else
                    e->cTransform->velocity.y =
                        std::max((float)0, e->cTransform->velocity.y -
                                               e->cTransform->friction);
            }
            e->cTransform->angle++;
        }
    }
    for (auto& e : m_manager.getEntities("specialbullet")) {
        e->cCollision->radius++;
        float radius = e->cShape->shape.getRadius();
        e->cShape->shape.setRadius(radius + 1);
        e->cShape->shape.setOrigin(radius, radius);
    }
}

void Game::sGUI() {
    ImGui::Begin("Geometry Wars");
    ImGui::BeginTabBar("bar");

    if (ImGui::BeginTabItem("Systems")) {
        ImGui::Checkbox("Movement", &m_movementSystem);
        ImGui::Checkbox("Lifespan", &m_lifespanSystem);
        ImGui::Checkbox("Collision", &m_collisionSystem);
        ImGui::Checkbox("Spawning", &m_enemySpawnerSystem);
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Entities")) {
        if (ImGui::CollapsingHeader("Entities by tag")) {
            for (auto [tag, v] : m_manager.getEntityMap()) {
                if (ImGui::CollapsingHeader(tag.c_str())) {
                    for (auto& e : v) {
                        int r = e->cShape->shape.getFillColor().r;
                        int g = e->cShape->shape.getFillColor().g;
                        int b = e->cShape->shape.getFillColor().b;
                        int a = e->cShape->shape.getFillColor().a;

                        ImGui::PushStyleColor(ImGuiCol_Button,
                                              IM_COL32(r, g, b, a));
                        std::string buttonName =
                            "D##" + std::to_string(e->id()) + ".";
                        if (ImGui::Button(buttonName.c_str())) {
                            std::cout << "hello\n";
                            e->destroy();
                        }
                        ImGui::PopStyleColor();

                        ImGui::SameLine();
                        ImGui::Text(e->tag().c_str());

                        ImGui::SameLine();
                        std::string s =
                            "(" + std::to_string((int)e->cTransform->pos.x) +
                            "," + std::to_string((int)e->cTransform->pos.y) +
                            ")";
                        ImGui::Text(s.c_str());
                    }
                }
            }
        }
        if (ImGui::CollapsingHeader("All Entities")) {
            for (auto& e : m_manager.getEntities()) {
                int r = e->cShape->shape.getFillColor().r;
                int g = e->cShape->shape.getFillColor().g;
                int b = e->cShape->shape.getFillColor().b;
                int a = e->cShape->shape.getFillColor().a;

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(r, g, b, a));
                std::string buttonName = "D##" + std::to_string(e->id());
                if (ImGui::Button(buttonName.c_str())) {
                    std::cout << "hello\n";
                    e->destroy();
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                ImGui::Text(e->tag().c_str());

                ImGui::SameLine();
                std::string s =
                    "(" + std::to_string((int)e->cTransform->pos.x) + "," +
                    std::to_string((int)e->cTransform->pos.y) + ")";
                ImGui::Text(s.c_str());
            }
        }
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();
}

void Game::sRender() {
    m_window.clear();
    ImGui::SFML::Render(m_window);
    for (auto& e : m_manager.getEntities()) {
        if (e->cTransform && e->cShape) {
            e->cShape->shape.setPosition(e->cTransform->pos.x,
                                         e->cTransform->pos.y);
            e->cShape->shape.setRotation(e->cTransform->angle);
            m_window.draw(e->cShape->shape);
        }
    }
    m_text.setPosition(1, 1);
    m_window.draw(m_text);
    m_window.display();
}

void Game::sPlayerSpawner() {
    if (m_manager.getEntities("player").empty()) return;
    m_manager.getEntities("player")[0]->cTransform->pos = {
        m_window.getSize().x / 2.0, m_window.getSize().y / 2.0};

    m_manager.getEntities("player")[0]->cTransform->velocity = {0, 0};
    m_manager.getEntities("player")[0]->cScore->score = 0;
}

void Game::sLifespan() {
    for (auto& e : m_manager.getEntities()) {
        if (e->cLifespan) {
            if (e->cLifespan->remaining == 0)
                e->destroy();

            else if (e->cShape) {
                sf::Color c = e->cShape->shape.getFillColor();
                e->cShape->shape.setFillColor(sf::Color(
                    c.r, c.g, c.b,
                    255.0 * e->cLifespan->remaining / e->cLifespan->total));

                c = e->cShape->shape.getOutlineColor();
                e->cShape->shape.setOutlineColor(sf::Color(
                    c.r, c.g, c.b,
                    255.0 * e->cLifespan->remaining / e->cLifespan->total));

                e->cLifespan->remaining--;
            }
        }
    }
}

void Game::sScore() {
    if (m_manager.getEntities("player").empty()) return;
    std::string s =
        "Score " +
        std::to_string(m_manager.getEntities("player")[0]->cScore->score);
    m_text = sf::Text(s, m_font, m_fontConfig.SZ);
    m_text.setFillColor(
        sf::Color(m_fontConfig.R, m_fontConfig.G, m_fontConfig.B));
}
