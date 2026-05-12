#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "sl.h"

// --- KONSTANTA & PENGATURAN ---
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

enum GameState { MENU, PLAYING, GAME_OVER };

// --- STRUKTUR DATA ---
struct Player {
    float x, y;
    int hp;
    int maxHp;
    int score;
    float speed;
    int weaponLevel;
    float shootCooldown;
};

struct Bullet {
    float x, y;
    float speed;
    bool active;
};

struct Enemy {
    float x, y;
    float speedX, speedY;
    int hp;
    int type; // 0 = Meteor, 1 = Alien
    bool active;
};

struct Explosion {
    float x, y;
    int timer;
    bool active;
};

struct PowerUp {
    float x, y;
    float speed;
    int type; // 0 = Heal, 1 = Upgrade Senjata
    bool active;
};

// --- FUNGSI DETEKSI TABRAKAN (AABB) ---
bool checkCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return (std::abs(x1 - x2) * 2 < (w1 + w2)) && (std::abs(y1 - y2) * 2 < (h1 + h2));
}

int main() {
    slWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Space War Shooter", false);
    srand(static_cast<unsigned int>(time(0)));

    // ==========================================================
    // 1. LOAD ASSETS (SUDAH DIUBAH KE .TTF DAN DOUBLE BACKSLASH)
    // ==========================================================
    int texPlayer = slLoadTexture("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\Pesawat.png");
    int texMeteor = slLoadTexture("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\Asteroid.png");
    int texAlien = slLoadTexture("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\UFO.png");
    int texBullet = slLoadTexture("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\laser.png");
    int texExplode = slLoadTexture("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\duar.png");
    int texPowerUp = slLoadTexture("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\PowerUp.png");
    int texBg = slLoadTexture("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\blackholm.png");

    // PERBAIKAN: Menggunakan .ttf dan path lengkap sesuai lokasi folder Anda
    int fntMain = slLoadFont("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\SPACE.ttf");

    int sfxShoot = slLoadWAV("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\Shoot2.wav");
    int sfxExplode = slLoadWAV("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\Hit4.wav");
    int sfxPowerUp = slLoadWAV("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\PowerUp1.wav");
    int bgmMusic = slLoadWAV("D:\\coba coba\\Game-perang2-main\\Game perang\\Game perang\\assets\\Happy8Bit.wav");

    if (bgmMusic != -1) slSoundLoop(bgmMusic);

    // ==========================================
    // 2. VARIABEL GAME
    // ==========================================
    GameState state = MENU;
    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Explosion> explosions;
    std::vector<PowerUp> powerups;

    int level = 1;
    float bgY = 300;
    int enemySpawnTimer = 0;
    int enemySpawnRate = 100;

    auto resetGame = [&]() {
        player = { SCREEN_WIDTH / 2.0f, 100.0f, 5, 5, 0, 5.0f, 1, 0 };
        bullets.clear();
        enemies.clear();
        explosions.clear();
        powerups.clear();
        level = 1;
        enemySpawnRate = 100;
        state = PLAYING;
        };

    // ==========================================
    // 3. MAIN GAME LOOP
    // ==========================================
    while (!slShouldClose()) {

        bgY -= 1.0f;
        if (bgY <= -300) bgY = 300;
        slSetForeColor(1, 1, 1, 1);
        slSprite(texBg, 400, bgY, 800, 1200);
        slSprite(texBg, 400, bgY + 1200, 800, 1200);

        if (state == MENU) {
            slSetFont(fntMain, 48); // Menggunakan font .ttf
            slSetTextAlign(SL_ALIGN_CENTER);
            slText(SCREEN_WIDTH / 2, 400, "SPACE WAR SHOOTER");
            slSetFont(fntMain, 24);
            slText(SCREEN_WIDTH / 2, 300, "Tekan 'ENTER' untuk Mulai");
            slText(SCREEN_WIDTH / 2, 250, "Gunakan W, A, S, D / Arrow & SPASI");

            if (slGetKey(257)) {
                resetGame();
            }
        }
        else if (state == PLAYING) {
            // KONTROL
            if (slGetKey('W') || slGetKey(265)) player.y += player.speed;
            if (slGetKey('S') || slGetKey(264)) player.y -= player.speed;
            if (slGetKey('A') || slGetKey(263)) player.x -= player.speed;
            if (slGetKey('D') || slGetKey(262)) player.x += player.speed;

            if (player.x < 30) player.x = 30;
            if (player.x > SCREEN_WIDTH - 30) player.x = SCREEN_WIDTH - 30;
            if (player.y < 40) player.y = 40;
            if (player.y > SCREEN_HEIGHT - 40) player.y = SCREEN_HEIGHT - 40;

            // MENEMBAK
            if (player.shootCooldown > 0) player.shootCooldown--;
            if (slGetKey(' ') && player.shootCooldown <= 0) {
                if (player.weaponLevel == 1) {
                    bullets.push_back({ player.x, player.y + 30, 10.0f, true });
                }
                else {
                    bullets.push_back({ player.x - 15, player.y + 30, 10.0f, true });
                    bullets.push_back({ player.x + 15, player.y + 30, 10.0f, true });
                }
                slSoundPlay(sfxShoot);
                player.shootCooldown = 15;
            }

            // UPDATE PELURU
            for (auto& b : bullets) {
                if (b.active) {
                    b.y += b.speed;
                    slSprite(texBullet, b.x, b.y, 10, 30);
                    if (b.y > SCREEN_HEIGHT + 50) b.active = false;
                }
            }

            // SPAWN MUSUH
            enemySpawnTimer++;
            if (enemySpawnTimer >= enemySpawnRate) {
                Enemy e;
                e.x = rand() % (SCREEN_WIDTH - 60) + 30;
                e.y = SCREEN_HEIGHT + 50;
                e.type = rand() % 2;
                e.active = true;

                if (e.type == 0) {
                    e.hp = 2 + (level / 2);
                    e.speedY = -(2.0f + (level * 0.5f));
                    e.speedX = 0;
                }
                else {
                    e.hp = 1 + (level / 2);
                    e.speedY = -(1.5f + (level * 0.5f));
                    e.speedX = (rand() % 3 - 1) * 2.0f;
                }
                enemies.push_back(e);
                enemySpawnTimer = 0;
            }

            // UPDATE MUSUH & COLLISION
            for (auto& e : enemies) {
                if (!e.active) continue;
                e.x += e.speedX;
                e.y += e.speedY;

                if (e.type == 1 && (e.x < 30 || e.x > SCREEN_WIDTH - 30)) e.speedX *= -1;

                if (e.type == 0) slSprite(texMeteor, e.x, e.y, 60, 60);
                else slSprite(texAlien, e.x, e.y, 70, 50);

                if (e.y < -50) e.active = false;

                if (checkCollision(player.x, player.y, 50, 60, e.x, e.y, 50, 50)) {
                    e.active = false;
                    player.hp--;
                    explosions.push_back({ player.x, player.y, 30, true });
                    slSoundPlay(sfxExplode);
                    if (player.hp <= 0) state = GAME_OVER;
                }

                for (auto& b : bullets) {
                    if (b.active && checkCollision(b.x, b.y, 10, 30, e.x, e.y, 50, 50)) {
                        b.active = false;
                        e.hp--;
                        if (e.hp <= 0) {
                            e.active = false;
                            player.score += (e.type == 0) ? 10 : 20;
                            explosions.push_back({ e.x, e.y, 20, true });
                            slSoundPlay(sfxExplode);
                            if (rand() % 100 < 10) powerups.push_back({ e.x, e.y, -2.0f, rand() % 2, true });
                        }
                    }
                }
            }

            // UPDATE POWER UPS
            for (auto& p : powerups) {
                if (p.active) {
                    p.y += p.speed;
                    slSetForeColor(p.type == 0 ? 0 : 1, 1, p.type == 0 ? 0 : 1, 1);
                    slSprite(texPowerUp, p.x, p.y, 30, 30);
                    slSetForeColor(1, 1, 1, 1);

                    if (checkCollision(player.x, player.y, 50, 60, p.x, p.y, 30, 30)) {
                        p.active = false;
                        slSoundPlay(sfxPowerUp);
                        if (p.type == 0 && player.hp < player.maxHp) player.hp++;
                        else player.weaponLevel = 2;
                    }
                }
            }

            // LEDAKAN
            for (auto& ex : explosions) {
                if (ex.active) {
                    slSprite(texExplode, ex.x, ex.y, 120, 90);
                    ex.timer--;
                    if (ex.timer <= 0) ex.active = false;
                }
            }

            if (player.score > level * 150) {
                level++;
                if (enemySpawnRate > 20) enemySpawnRate -= 15;
            }

            slSprite(texPlayer, player.x, player.y, 80, 60);

            // UI
            slSetFont(fntMain, 20); // Menggunakan font .ttf
            slSetTextAlign(SL_ALIGN_LEFT);
            slText(20, 560, ("HP: " + std::to_string(player.hp)).c_str());
            slText(20, 530, ("SCORE: " + std::to_string(player.score)).c_str());
            slText(20, 500, ("LEVEL: " + std::to_string(level)).c_str());
        }
        else if (state == GAME_OVER) {
            slSetFont(fntMain, 60);
            slSetTextAlign(SL_ALIGN_CENTER);
            slSetForeColor(1, 0, 0, 1);
            slText(SCREEN_WIDTH / 2, 350, "GAME OVER");
            slSetFont(fntMain, 30);
            slSetForeColor(1, 1, 1, 1);
            slText(SCREEN_WIDTH / 2, 250, ("Final Score: " + std::to_string(player.score)).c_str());
            slSetFont(fntMain, 20);
            slText(SCREEN_WIDTH / 2, 200, "Tekan 'ENTER' untuk Kembali ke Menu");

            if (slGetKey(257)) state = MENU;
        }

        slRender();
    }

    slClose();
    return 0;
}