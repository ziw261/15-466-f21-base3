#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>


#define PLAYER_SPEED 100.f
#define CABBAGE_SIZE glm::vec3(35.f, 31.f, 41.f)
#define CARROT_SIZE glm::vec3(5.f, 50.f, 7.f)
#define CABBAGE_EATTIME 5.f
#define CARROT_EATTIME 2.f


struct GardenMode : Mode {

	struct Player {
		Scene::Transform* transform = nullptr;
		glm::vec3 size = glm::vec3(20.f, 10.f, 10.f);
		Player() {}
		Player(Scene::Transform* trans) : transform(trans) {}
	};

	struct Food {
		Scene::Transform* transform = nullptr;
		glm::vec3 size = glm::vec3(30.f, 31.f, 41.f);
		float lifetime = 0.f;
		Food() {}
		Food(Scene::Transform* trans, glm::vec3 food_size, float t) : transform(trans), size(food_size), lifetime(t) {}
	};

	enum class TextStatus {
		Default,
		Eating
	};

	GardenMode();
	virtual ~GardenMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, eat;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Player player;
	glm::quat default_rot;
	int target = -1;
	float walls[4];
	std::string show_text = "";

	std::vector<Food> foods;
	//glm::vec3 get_leg_tip_position();

	void LoadGameObjects();
	void UpdatePlayerMovement(float elapsed);
	void UpdateEating(float elapsed);
	void UpdateShowText(float elapsed, TextStatus ts);
	bool CollisionTest(glm::vec2 pos);

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
