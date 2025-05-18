#include <vector>
#include <algorithm>
#include <functional> 
#include <memory>
#include <cstdlib>
#include <cmath>
#include <ctime>

#include <raylib.h>
#include <raymath.h>

// --- UTILS ---
namespace Utils {
	inline static float RandomFloat(float min, float max) {
		return min + static_cast<float>(rand()) / RAND_MAX * (max - min);
	}
}

// --- TRANSFORM, PHYSICS, LIFETIME, RENDERABLE ---
struct TransformA {
	Vector2 position{};
	float rotation{};
};

struct Physics {
	Vector2 velocity{};
	float rotationSpeed{};
};

struct Renderable {
	enum Size { SMALL = 1, MEDIUM = 2, LARGE = 4 } size = SMALL;
};

// --- RENDERER ---
class Renderer {
public:
	static Renderer& Instance() {
		static Renderer inst;
		return inst;
	}

	void Init(int w, int h, const char* title) {
		InitWindow(w, h, title);
		SetTargetFPS(60);
		screenW = w;
		screenH = h;
	}

	void Begin() {
		BeginDrawing();
		ClearBackground(YELLOW);		
	}

	void End() {
		EndDrawing();
	}

	void DrawPoly(const Vector2& pos, int sides, float radius, float rot) {
		DrawPolyLines(pos, sides, radius, rot, WHITE);
	}

	int Width() const {
		return screenW;
	}

	int Height() const {
		return screenH;
	}

private:
	Renderer() = default;

	int screenW{};
	int screenH{};
};

// --- ASTEROID HIERARCHY ---

class Asteroid {
public:
	Asteroid(int screenW, int screenH) {
		init(screenW, screenH);
	}
	virtual ~Asteroid() = default;

	bool Update(float dt) {
		transform.position = Vector2Add(transform.position, Vector2Scale(physics.velocity, dt));
		transform.rotation += physics.rotationSpeed * dt;
		if (transform.position.x < -GetRadius() || transform.position.x > Renderer::Instance().Width() + GetRadius() ||
			transform.position.y < -GetRadius() || transform.position.y > Renderer::Instance().Height() + GetRadius())
			return false;
		return true;
	}
	virtual void Draw() const = 0;

	Vector2 GetPosition() const {
		return transform.position;
	}

	float constexpr GetRadius() const {
		return 32.f + 15.f * (float)render.size;
	}

	int GetDamage() const {
		return baseDamage * static_cast<int>(render.size);
	}

	int GetSize() const {
		return static_cast<int>(render.size);
	}

protected:
	void init(int screenW, int screenH) {
		// Choose size
		render.size = static_cast<Renderable::Size>(1 << GetRandomValue(0, 2));

		// Spawn at random edge
		switch (GetRandomValue(0, 3)) {
		case 0:
			transform.position = { Utils::RandomFloat(0, screenW), -GetRadius() };
			break;
		case 1:
			transform.position = { screenW + GetRadius(), Utils::RandomFloat(0, screenH) };
			break;
		case 2:
			transform.position = { Utils::RandomFloat(0, screenW), screenH + GetRadius() };
			break;
		default:
			transform.position = { -GetRadius(), Utils::RandomFloat(0, screenH) };
			break;
		}

		// Aim towards center with jitter
		float maxOff = fminf(screenW, screenH) * 0.1f;
		float ang = Utils::RandomFloat(0, 2 * PI);
		float rad = Utils::RandomFloat(0, maxOff);
		Vector2 center = {
										 screenW * 0.5f + cosf(ang) * rad,
										 screenH * 0.5f + sinf(ang) * rad
		};

		Vector2 dir = Vector2Normalize(Vector2Subtract(center, transform.position));
		physics.velocity = Vector2Scale(dir, Utils::RandomFloat(SPEED_MIN, SPEED_MAX));
		physics.rotationSpeed = Utils::RandomFloat(ROT_MIN, ROT_MAX);

		transform.rotation = Utils::RandomFloat(0, 360);
	}

	TransformA transform;
	Physics    physics;
	Renderable render;

	int baseDamage = 0;
	static constexpr float LIFE = 10.f;
	static constexpr float SPEED_MIN = 125.f;
	static constexpr float SPEED_MAX = 250.f;
	static constexpr float ROT_MIN = 50.f;
	static constexpr float ROT_MAX = 240.f;
};

class TriangleAsteroid : public Asteroid {
public:
	TriangleAsteroid(int w, int h) : Asteroid(w, h) 
	{ 
		baseDamage = 5;
		texture = LoadTexture("perła.png");
		GenTextureMipmaps(&texture);                                                        // Generate GPU mipmaps for a texture
		SetTextureFilter(texture, 2);
		
	}
	~TriangleAsteroid() {
		UnloadTexture(texture);
	}
	void Draw() const override {
		//Renderer::Instance().DrawPoly(transform.position, 3, GetRadius(), transform.rotation);
		DrawTextureEx(texture, transform.position, transform.rotation, GetRadius()/Renderer::Instance().Height(), WHITE);
	}
private:
	Texture2D texture;
};
class SquareAsteroid : public Asteroid {
public:
	SquareAsteroid(int w, int h) : Asteroid(w, h) { 
		baseDamage = 10;
		texture = LoadTexture("kasztelan.png");
		GenTextureMipmaps(&texture);                                                        // Generate GPU mipmaps for a texture
		SetTextureFilter(texture, 2);
	 }
	 ~SquareAsteroid() {
		UnloadTexture(texture);
	}
	void Draw() const override {
		//Renderer::Instance().DrawPoly(transform.position, 4, GetRadius(), transform.rotation);
		DrawTextureEx(texture, transform.position, transform.rotation, GetRadius()/712.f, WHITE);
	}
	private:
	Texture2D texture;
};
class PentagonAsteroid : public Asteroid {
public:
	PentagonAsteroid(int w, int h) : Asteroid(w, h) { 
		baseDamage = 15;
		texture = LoadTexture("kozel.png");
		GenTextureMipmaps(&texture);                                                        // Generate GPU mipmaps for a texture
		SetTextureFilter(texture, 2); 
	}
	~PentagonAsteroid() {
		UnloadTexture(texture);
	}
	void Draw() const override {
		//Renderer::Instance().DrawPoly(transform.position, 5, GetRadius(), transform.rotation);
		DrawTextureEx(texture, transform.position,  transform.rotation, GetRadius()/Renderer::Instance().Height(), WHITE);
	}
	private:
	Texture2D texture;
};
class HealingAsteroid : public Asteroid {
public:
	HealingAsteroid(int w, int h) : Asteroid(w, h) { 
		baseDamage = -2;
		texture = LoadTexture("woda.png");
		GenTextureMipmaps(&texture);                                                        // Generate GPU mipmaps for a texture
		SetTextureFilter(texture, 2); 
	}
	~HealingAsteroid() {
		UnloadTexture(texture);
	}
	void Draw() const override {
		//Renderer::Instance().DrawPoly(transform.position, 5, GetRadius(), transform.rotation);
		DrawTextureEx(texture, transform.position,  transform.rotation, GetRadius()/598.f, WHITE);
	}
	private:
	Texture2D texture;
};

// Shape selector
enum class AsteroidShape { TRIANGLE = 3, SQUARE = 4, PENTAGON = 5, HEALING = 6, RANDOM = 0 };

// Factory
static inline std::unique_ptr<Asteroid> MakeAsteroid(int w, int h, AsteroidShape shape) {
	switch (shape) {
	case AsteroidShape::TRIANGLE:
		return std::make_unique<TriangleAsteroid>(w, h);
	case AsteroidShape::SQUARE:
		return std::make_unique<SquareAsteroid>(w, h);
	case AsteroidShape::PENTAGON:
		return std::make_unique<PentagonAsteroid>(w, h);
	case AsteroidShape::HEALING:
		return std::make_unique<HealingAsteroid>(w, h);
	default: {
		return MakeAsteroid(w, h, static_cast<AsteroidShape>(3 + GetRandomValue(0, 3)));
	}
	}
}

// --- PROJECTILE HIERARCHY ---
enum class WeaponType { LASER, BULLET, CRATE, COUNT };
class Projectile {
public:
	Projectile(Vector2 pos, Vector2 vel, int dmg, WeaponType wt)
	{
		transform.position = pos;
		physics.velocity = vel;
		baseDamage = dmg;
		type = wt;
		
			ptexture = LoadTexture("skrzynka.png");
			GenTextureMipmaps(&ptexture);                                                        // Generate GPU mipmaps for a texture
			SetTextureFilter(ptexture, 2); 
			biedra = LoadTexture("biedronka.png");
			GenTextureMipmaps(&biedra);                                                        // Generate GPU mipmaps for a texture
			SetTextureFilter(biedra, 2); 
		
	}
	// ~Projectile()
	// {
	// 	if(type == WeaponType::CRATE) UnloadTexture(ptexture);
	// }
	bool Update(float dt) {
		transform.position = Vector2Add(transform.position, Vector2Scale(physics.velocity, dt));
		if(type == WeaponType::LASER){
			if(IsKeyDown(KEY_LEFT)) transform.position = Vector2Add(transform.position, Vector2Scale({physics.velocity.y,physics.velocity.x}, dt));
			if(IsKeyDown(KEY_RIGHT)) transform.position = Vector2Subtract(transform.position, Vector2Scale({physics.velocity.y,physics.velocity.x}, dt));
		}
		else if(type == WeaponType::CRATE){
			transform.position = Vector2Subtract(transform.position, Vector2Scale(physics.velocity, dt));
		}

		if (transform.position.x < 0 ||
			transform.position.x > Renderer::Instance().Width() ||
			transform.position.y < 0 ||
			transform.position.y > Renderer::Instance().Height())
		{
			return true;
		}
		return false;
	}
	void Draw() const {
		if (type == WeaponType::BULLET) {
			DrawCircleV(transform.position, 5.f, BLACK);
		}
		else if (type == WeaponType::CRATE)
		{
			//DrawCircleV(transform.position, GetRadius(), WHITE);
			DrawTextureEx(ptexture, transform.position,  0.0f, GetRadius()/449.f, WHITE);
		}
		else {
			DrawTextureEx(biedra, transform.position,  0.0f, GetRadius()/307.f, WHITE);
		}
	}
	Vector2 GetPosition() const {
		return transform.position;
	}

	float GetRadius() const {
		return (type == WeaponType::BULLET) ? 5.f : (type == WeaponType::CRATE) ? 100.f : 20.f;
	}

	int GetDamage() const {
		return baseDamage;
	}

private:
	TransformA transform;
	Physics    physics;
	int        baseDamage;
	WeaponType type;
	Texture2D ptexture;
	Texture2D biedra;
};

inline static Projectile MakeProjectile(WeaponType wt,
	const Vector2 pos,
	float speed)
{
	Vector2 vel{ 0, -speed };
	if (wt == WeaponType::LASER) {
		return Projectile(pos, vel, 20, wt);
	}
	else if (wt == WeaponType::CRATE){
		return Projectile(pos, vel, 50, wt);
	}
	else {
		return Projectile(pos, vel, 10, wt);
	}
}

// --- SHIP HIERARCHY ---
class Ship {
public:
	Ship(int screenW, int screenH) {
		transform.position = {
												 screenW * 0.5f,
												 screenH * 0.5f
		};
		hp = 100;
		dmg_dealt = 0;
		speed = 250.f;
		alive = true;
		multiplier = 0.001;

		// per-weapon fire rate & spacing
		fireRateLaser = 5.f; // shots/sec
		fireRateBullet = 15.f;
		fireRateCrate = 3.f;
		spacingLaser = 40.f; // px between lasers
		spacingBullet = 20.f;
		spacingCrate = 800.f;
	}
	virtual ~Ship() = default;
	virtual void Update(float dt) = 0;
	virtual void Draw() const = 0;

	void TakeDamage(int dmg) {
		if (!alive) return;
		hp -= dmg;
		if (hp <= 0) alive = false;
	}

	void DealDamage(int dmg, WeaponType wt) {
		dmg_dealt += dmg;
		if (wt == WeaponType::LASER) fireRateLaser = GetFireRate(wt) + multiplier*dmg_dealt;
		else fireRateBullet = GetFireRate(wt) + multiplier*dmg_dealt;
	}

	bool IsAlive() const {
		return alive;
	}

	Vector2 GetPosition() const {
		return transform.position;
	}

	virtual float GetRadius() const = 0;

	int GetHP() const {
		return hp;
	}

	int GetScore() const {
		return dmg_dealt;
	}

	float GetFireRate(WeaponType wt) const {
		return (wt == WeaponType::LASER) ? fireRateLaser : (wt == WeaponType::CRATE) ? fireRateCrate : fireRateBullet;
	}

	float GetSpacing(WeaponType wt) const {
		return (wt == WeaponType::LASER) ? spacingLaser : (wt == WeaponType::CRATE) ? spacingCrate : spacingBullet;
	}

protected:
	TransformA transform;
	int        hp;
	int 	   dmg_dealt;
	float 	   multiplier;
	float      speed;
	bool       alive;
	float      fireRateLaser;
	float      fireRateBullet;
	float      fireRateCrate;
	float      spacingLaser;
	float	   spacingCrate;
	float      spacingBullet;
};

class PlayerShip :public Ship {
public:
	PlayerShip(int w, int h) : Ship(w, h) {
		texture = LoadTexture("biedronka.png");
		GenTextureMipmaps(&texture);                                                        // Generate GPU mipmaps for a texture
		SetTextureFilter(texture, 2);
		scale = 0.25f;
	}
	~PlayerShip() {
		UnloadTexture(texture);
	}

	void Update(float dt) override {
		if (alive) {
			// if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) transform.position.y -= speed * dt;
			// if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) transform.position.y += speed * dt;
			// if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) transform.position.x -= speed * dt;
			// if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) transform.position.x += speed * dt;
			if (IsKeyDown(KEY_W) ) transform.position.y -= speed * dt;
			if (IsKeyDown(KEY_S) ) transform.position.y += speed * dt;
			if (IsKeyDown(KEY_A) ) transform.position.x -= speed * dt;
			if (IsKeyDown(KEY_D) ) transform.position.x += speed * dt;
		}
		else {
			transform.position.y += speed * dt;
		}
	}

	void Draw() const override {
		if (!alive && fmodf(GetTime(), 0.4f) > 0.2f) return;
		Vector2 dstPos = {
										 transform.position.x - (texture.width * scale) * 0.5f,
										 transform.position.y - (texture.height * scale) * 0.5f
		};
		DrawTextureEx(texture, dstPos, 0.0f, scale, WHITE);
	}

	float GetRadius() const override {
		return (texture.width * scale) * 0.5f;
	}

private:
	Texture2D texture;
	float     scale;
};

// --- APPLICATION ---
class Application {
public:
	static Application& Instance() {
		static Application inst;
		return inst;
	}

	void Run() {
		srand(static_cast<unsigned>(time(nullptr)));
		Renderer::Instance().Init(C_WIDTH, C_HEIGHT, "Asteroids OOP");

		auto player = std::make_unique<PlayerShip>(C_WIDTH, C_HEIGHT);

		float spawnTimer = 0.f;
		float spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
		WeaponType currentWeapon = WeaponType::LASER;
		float shotTimer = 0.f;
		int highscore = 0;

		while (!WindowShouldClose()) {
			float dt = GetFrameTime();
			spawnTimer += dt;

			// Update player
			player->Update(dt);

			// Restart logic
			if (!player->IsAlive() && IsKeyPressed(KEY_R)) {
				if(player->GetScore() > highscore)highscore = player->GetScore();
				player = std::make_unique<PlayerShip>(C_WIDTH, C_HEIGHT);
				asteroids.clear();
				projectiles.clear();
				spawnTimer = 0.f;
				spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
			}
			// Asteroid shape switch
			if (IsKeyPressed(KEY_ONE)) {
				currentShape = AsteroidShape::TRIANGLE;
			}
			if (IsKeyPressed(KEY_TWO)) {
				currentShape = AsteroidShape::SQUARE;
			}
			if (IsKeyPressed(KEY_THREE)) {
				currentShape = AsteroidShape::PENTAGON;
			}
			if (IsKeyPressed(KEY_FOUR)) {
				currentShape = AsteroidShape::HEALING;
			}
			if (IsKeyPressed(KEY_ZERO)) {
				currentShape = AsteroidShape::RANDOM;
			}

			// Weapon switch
			if (IsKeyPressed(KEY_TAB)) {
				currentWeapon = static_cast<WeaponType>((static_cast<int>(currentWeapon) + 1) % static_cast<int>(WeaponType::COUNT));
			}

			// Shooting
			{
				if (player->IsAlive() && IsKeyDown(KEY_SPACE)) {
					shotTimer += dt;
					float interval = 1.f / player->GetFireRate(currentWeapon);
					float projSpeed = player->GetSpacing(currentWeapon) * player->GetFireRate(currentWeapon);

					while (shotTimer >= interval) {
						Vector2 p = player->GetPosition();
						p.y -= player->GetRadius();
						projectiles.push_back(MakeProjectile(currentWeapon, p, projSpeed));
						shotTimer -= interval;
					}
				}
				else {
					float maxInterval = 1.f / player->GetFireRate(currentWeapon);

					if (shotTimer > maxInterval) {
						shotTimer = fmodf(shotTimer, maxInterval);
					}
				}
			}

			// Spawn asteroids
			if (spawnTimer >= spawnInterval && asteroids.size() < MAX_AST) {
				asteroids.push_back(MakeAsteroid(C_WIDTH, C_HEIGHT, currentShape));
				spawnTimer = 0.f;
				spawnInterval = Utils::RandomFloat(C_SPAWN_MIN, C_SPAWN_MAX);
			}

			// Update projectiles - check if in boundries and move them forward
			{
				auto projectile_to_remove = std::remove_if(projectiles.begin(), projectiles.end(),
					[dt](auto& projectile) {
						return projectile.Update(dt);
					});
				projectiles.erase(projectile_to_remove, projectiles.end());
			}

			// Projectile-Asteroid collisions O(n^2)
			for (auto pit = projectiles.begin(); pit != projectiles.end();) {
				bool removed = false;

				for (auto ait = asteroids.begin(); ait != asteroids.end(); ++ait) {
					float dist = Vector2Distance((*pit).GetPosition(), (*ait)->GetPosition());
					if (dist < (*pit).GetRadius() + (*ait)->GetRadius()) {
						player->DealDamage(((*ait)->GetDamage()),currentWeapon);
						//if score is higher than 500, small chance of asteroid exploding into three new ones
						if(player->GetScore()>500 && player->GetScore()%5 != 0) for(int i=0; i<2; i++)asteroids.push_back(MakeAsteroid((*ait)->GetPosition().x, (*ait)->GetPosition().y, currentShape));
						ait = asteroids.erase(ait);
						pit = projectiles.erase(pit);
						removed = true;
						break;
					}
				}
				if (!removed) {
					++pit;
				}
			}

			// Asteroid-Ship collisions
			{
				auto remove_collision =
					[&player, dt](auto& asteroid_ptr_like) -> bool {
					if (player->IsAlive()) {
						float dist = Vector2Distance(player->GetPosition(), asteroid_ptr_like->GetPosition());

						if (dist < player->GetRadius() + asteroid_ptr_like->GetRadius()) {
							player->TakeDamage(asteroid_ptr_like->GetDamage());
							return true; // Mark asteroid for removal due to collision
						}
					}
					if (!asteroid_ptr_like->Update(dt)) {
						return true;
					}
					return false; // Keep the asteroid
					};
				auto asteroid_to_remove = std::remove_if(asteroids.begin(), asteroids.end(), remove_collision);
				asteroids.erase(asteroid_to_remove, asteroids.end());
			}

			// Render everything
			{
				Renderer::Instance().Begin();
				
				
				DrawText(TextFormat("HP: %d", player->GetHP()),
					10, 10, 20, GREEN);
				DrawText(TextFormat("Score: %d", player->GetScore()),
					10, 30, 20, RED);
				const char* weaponName = (currentWeapon == WeaponType::LASER) ? "ARMY" : ((currentWeapon == WeaponType::CRATE) ? "CRATE" : "DOTS");
				DrawText(TextFormat("Weapon: %s", weaponName),
					10, 50, 20, BLUE);
				DrawText(TextFormat("Highscore"),
					C_WIDTH-100, 10, 15, BLACK);
				DrawText(TextFormat("%d", highscore),
					C_WIDTH-85, 25, 15, BLACK);
				for (const auto& projPtr : projectiles) {
					projPtr.Draw();
				}
				for (const auto& astPtr : asteroids) {
					astPtr->Draw();
				}

				if (!(player->IsAlive())) //moving text is intentional
				{
					
					DrawText(TextFormat("GAME OVER"),
						player->GetPosition().x-player->GetRadius(), player->GetPosition().y+player->GetRadius(), 40, RED);
					DrawText(TextFormat("SCORE: %d", player->GetScore()),
						player->GetPosition().x-player->GetRadius(), player->GetPosition().y+player->GetRadius()-40, 20, BLUE);
					if(player->GetScore() > highscore)
						DrawText(TextFormat("NEW HIGHSCORE"),
							player->GetPosition().x-player->GetRadius(), player->GetPosition().y+player->GetRadius()+45, 30, GREEN);
					if(player->GetPosition().y > C_HEIGHT)
						DrawText(TextFormat("Press R to restart"),
								C_WIDTH/4, C_HEIGHT/2, 40, BLACK);

				}

				player->Draw();

				Renderer::Instance().End();
			}
		}
	}

private:
	Application()
	{
		asteroids.reserve(1000);
		projectiles.reserve(10'000);
	};

	std::vector<std::unique_ptr<Asteroid>> asteroids;
	std::vector<Projectile> projectiles;

	AsteroidShape currentShape = AsteroidShape::RANDOM;

	static constexpr int C_WIDTH = 800;
	static constexpr int C_HEIGHT = 800;
	static constexpr size_t MAX_AST = 150;
	static constexpr float C_SPAWN_MIN = 0.5f;
	static constexpr float C_SPAWN_MAX = 3.0f;
	static constexpr int C_MAX_ASTEROIDS = 1000;
	static constexpr int C_MAX_PROJECTILES = 10'000;
};

int main() {
	Application::Instance().Run();
	return 0;
}
